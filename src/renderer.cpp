#include "renderer.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "constants.h"
#include "fx.h"
#include "i18n.h"

namespace neon {

static constexpr float TWO_PI = 6.283185307179586f;

static SDL_Color mix(SDL_Color a, SDL_Color b, float t) {
  return { static_cast<Uint8>(a.r + (b.r - a.r) * t), static_cast<Uint8>(a.g + (b.g - a.g) * t),
           static_cast<Uint8>(a.b + (b.b - a.b) * t), static_cast<Uint8>(a.a + (b.a - a.a) * t) };
}

static SDL_Color withAlpha(SDL_Color c, Uint8 a) { c.a = a; return c; }

void Renderer::shutdown() {
  clearTextCache();
  if (font_) { TTF_CloseFont(font_); font_ = nullptr; }
}

// 打开字体:优先指定 face(用于 ttc 集合),失败回退默认 face
static bool openFont(TTF_Font*& out, const char* path, int faceIndex) {
  if (faceIndex >= 0) {
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, TTF_PROP_FONT_CREATE_FILENAME_STRING, path);
    SDL_SetNumberProperty(props, TTF_PROP_FONT_CREATE_FACE_NUMBER, faceIndex);
    SDL_SetFloatProperty(props, TTF_PROP_FONT_CREATE_SIZE_FLOAT, 16.0f);
    out = TTF_OpenFontWithProperties(props);
    SDL_DestroyProperties(props);
    if (out) return true;
  }
  out = TTF_OpenFont(path, 16.0f);
  return out != nullptr;
}

bool Renderer::init(SDL_Window* window, SDL_Renderer* renderer) {
  window_ = window;
  renderer_ = renderer;
  // 1) 内置子集字体(跨平台兜底;移动端唯一可靠来源)
  //    - 桌面/非 Android:SDL_GetBasePath() 相对路径(开发:build/assets/fonts/;iOS bundle:fonts/)
  //    - Android:SDL_IOFromFile 直接映射 APK assets(AAssetManager),路径不带前缀
  {
#if defined(__ANDROID__)
    const char* bundled = "fonts/NotoSansSC-Bold-subset.otf";
    if (openFont(font_, bundled, -1)) {
      onWindowResized(WINDOW_W, WINDOW_H);
      return true;
    }
#else
    const char* base = SDL_GetBasePath();
    if (base) {
      std::string bundled = std::string(base) + "assets/fonts/NotoSansSC-Bold-subset.otf";
      if (openFont(font_, bundled.c_str(), 2) || openFont(font_, bundled.c_str(), -1)) {
        onWindowResized(WINDOW_W, WINDOW_H);
        return true;
      }
      // iOS App Bundle 布局:字体置于 bundle 根 fonts/ 子目录
      bundled = std::string(base) + "fonts/NotoSansSC-Bold-subset.otf";
      if (openFont(font_, bundled.c_str(), -1)) {
        onWindowResized(WINDOW_W, WINDOW_H);
        return true;
      }
    }
#endif
  }
  // 2) 桌面系统字体候选(平台条件编译)
#if defined(_WIN32)
  const char* systemFonts[] = {
    "C:\\Windows\\Fonts\\msyhbd.ttc",
    "C:\\Windows\\Fonts\\msyh.ttc",
    "C:\\Windows\\Fonts\\simhei.ttf",
    "C:\\Windows\\Fonts\\simsun.ttc",
  };
#elif defined(__APPLE__)
  const char* systemFonts[] = {
    "/System/Library/Fonts/PingFang.ttc",
    "/System/Library/Fonts/STHeiti Light.ttc",
    "/System/Library/Fonts/Hiragino Sans GB.ttc",
    "/System/Library/Fonts/Supplemental/Songti.ttc",
  };
#else
  const char* systemFonts[] = {
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc",
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
    "/usr/share/fonts/wenquanyi/wqy-zenhei/wqy-zenhei.ttc",
    "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
  };
#endif
  for (const char* path : systemFonts) {
    if (openFont(font_, path, 2) || openFont(font_, path, -1)) break;
  }
  onWindowResized(WINDOW_W, WINDOW_H);
  return font_ != nullptr;
}

void Renderer::onWindowResized(int newW, int newH) {
  winW_ = std::max(1, newW);
  winH_ = std::max(1, newH);
  scale_ = std::min(static_cast<float>(winW_) / static_cast<float>(WINDOW_W),
                    static_cast<float>(winH_) / static_cast<float>(WINDOW_H));
  offsetX_ = (static_cast<float>(winW_) - WINDOW_W * scale_) / 2;
  offsetY_ = (static_cast<float>(winH_) - WINDOW_H * scale_) / 2;
  SDL_SetRenderScale(renderer_, scale_, scale_);
  SDL_Rect vp = { static_cast<int>(std::lround(offsetX_ / scale_)), static_cast<int>(std::lround(offsetY_ / scale_)),
                  WINDOW_W, WINDOW_H };
  SDL_SetRenderViewport(renderer_, &vp);
}

void Renderer::screenToLogical(float& x, float& y) const {
  x = (x - offsetX_) / scale_;
  y = (y - offsetY_) / scale_;
}

void Renderer::configure(const std::string& ballColor, bool trailEnabled, Language lang, ThemeMode theme) {
  ballColor_ = ballColor;
  trailEnabled_ = trailEnabled;
  language_ = lang;
  bool light = theme == ThemeMode::Light;
  if (theme == ThemeMode::System) light = (SDL_GetSystemTheme() == SDL_SYSTEM_THEME_LIGHT);
  if (light != light_) { light_ = light; clearTextCache(); }
  if (!trailEnabled_) trails_.clear();
}

void Renderer::clearTextCache() {
  for (auto& [k, v] : cache_) SDL_DestroyTexture(v.tex);
  cache_.clear();
}

SDL_Color Renderer::hexColor(const std::string& s) const {
  SDL_Color c = { 255, 255, 255, 255 };
  if (s.size() >= 7 && s[0] == '#') {
    auto hex = [](char a, char b) -> int {
      auto v = [](char ch) -> int { return (ch >= '0' && ch <= '9') ? ch - '0' : (ch | 0x20) - 'a' + 10; };
      return v(a) * 16 + v(b);
    };
    c.r = static_cast<Uint8>(hex(s[1], s[2]));
    c.g = static_cast<Uint8>(hex(s[3], s[4]));
    c.b = static_cast<Uint8>(hex(s[5], s[6]));
  } else if (s.rfind("rgba(", 0) == 0) {
    // 手工解析 rgba(r,g,b,a):格式固定,避免 sscanf(MSVC C4996)
    float v[4] = { 0, 0, 0, 0 };
    const char* p = s.c_str() + 5;
    bool ok = true;
    for (int i = 0; i < 4 && ok; i++) {
      char* end = nullptr;
      v[i] = std::strtof(p, &end);
      if (end == p) { ok = false; break; }
      p = end;
      if (i < 3) {
        if (*p != ',') { ok = false; break; }
        p++;
      }
    }
    if (ok && *p == ')') {
      c = { static_cast<Uint8>(v[0]), static_cast<Uint8>(v[1]), static_cast<Uint8>(v[2]), static_cast<Uint8>(v[3] * 255) };
    }
  }
  return c;
}

void Renderer::setColor(SDL_Color c) const {
  SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, c.a);
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
}

Palette Renderer::palette() const {
  if (light_) {
    return { hexColor("#f9f7ff"), hexColor("#e5e1ec"), { 70, 70, 85, 20 }, hexColor("#1b1b21"),
             hexColor("#4f5794"), hexColor("#006b60") };
  }
  return { hexColor("#111318"), hexColor("#25262d"), { 225, 224, 235, 14 }, hexColor("#e5e1e9"),
           hexColor("#bec2ff"), hexColor("#65dbc8") };
}

// ---------------- 文本 ----------------

Renderer::TextTex* Renderer::textTexture(const std::string& text, float size, SDL_Color c) {
  if (!font_) return nullptr;
  char key[256];
  std::snprintf(key, sizeof(key), "%s|%.0f|%d,%d,%d,%d", text.c_str(), size, c.r, c.g, c.b, c.a);
  auto it = cache_.find(key);
  if (it != cache_.end()) return &it->second;
  if (cache_.size() > 400) clearTextCache();
  TTF_SetFontSize(font_, size);
  SDL_Surface* surf = TTF_RenderText_Blended(font_, text.c_str(), 0, c);
  if (!surf) return nullptr;
  SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surf);
  int w = surf->w, h = surf->h;
  SDL_DestroySurface(surf);
  if (!tex) return nullptr;
  TextTex tt{ tex, w, h };
  auto [it2, ok] = cache_.emplace(key, tt);
  return ok ? &it2->second : nullptr;
}

void Renderer::drawText(const std::string& text, float x, float y, float size, SDL_Color color, int align, float alpha) {
  TextTex* tt = textTexture(text, size, color);
  if (!tt) return;
  float dx = x;
  if (align == 1) dx = x - tt->w / 2.0f;
  else if (align == 2) dx = x - static_cast<float>(tt->w);
  SDL_FRect dst = { dx, y, static_cast<float>(tt->w), static_cast<float>(tt->h) };
  SDL_SetTextureAlphaMod(tt->tex, static_cast<Uint8>(alpha * 255));
  SDL_RenderTexture(renderer_, tt->tex, nullptr, &dst);
}

float Renderer::textWidth(const std::string& text, float size) {
  if (!font_) return 0;
  TTF_SetFontSize(font_, size);
  int w = 0, h = 0;
  TTF_GetStringSize(font_, text.c_str(), 0, &w, &h);
  return static_cast<float>(w);
}

// ---------------- 原语 ----------------

void Renderer::fillRect(float x, float y, float w, float h, SDL_Color c) {
  setColor(c);
  SDL_FRect r = { x, y, w, h };
  SDL_RenderFillRect(renderer_, &r);
}

void Renderer::fillCircle(float cx, float cy, float r, SDL_Color c) {
  setColor(c);
  int ri = static_cast<int>(r);
  for (int dy = -ri; dy <= ri; dy++) {
    float w = std::sqrt(std::max(0.0f, r * r - static_cast<float>(dy * dy)));
    SDL_FRect line = { cx - w, cy + static_cast<float>(dy), w * 2, 1.0f };
    SDL_RenderFillRect(renderer_, &line);
  }
}

void Renderer::strokeCircle(float cx, float cy, float r, float width, SDL_Color c) {
  setColor(c);
  int steps = static_cast<int>(std::max(24.0f, TWO_PI * r / 2.0f));
  for (int i = 0; i < steps; i++) {
    float a = TWO_PI * static_cast<float>(i) / static_cast<float>(steps);
    SDL_FPoint p = { cx + std::cos(a) * r, cy + std::sin(a) * r };
    SDL_FRect q = { p.x - width / 2, p.y - width / 2, width, width };
    SDL_RenderFillRect(renderer_, &q);
  }
}

void Renderer::fillRoundedRect(float x, float y, float w, float h, float radius, SDL_Color c) {
  float r = std::min(radius, std::min(w, h) / 2);
  setColor(c);
  SDL_FRect center = { x + r, y, w - 2 * r, h };
  SDL_RenderFillRect(renderer_, &center);
  SDL_FRect mid = { x, y + r, w, h - 2 * r };
  SDL_RenderFillRect(renderer_, &mid);
  fillCircle(x + r, y + r, r, c);
  fillCircle(x + w - r, y + r, r, c);
  fillCircle(x + r, y + h - r, r, c);
  fillCircle(x + w - r, y + h - r, r, c);
}

void Renderer::fillRotatedRect(float cx, float cy, float size, float angle, SDL_Color c) {
  float h = size / 2;
  SDL_FPoint corners[4] = {
    { -h, -h }, { h, -h }, { h, h }, { -h, h },
  };
  float ca = std::cos(angle), sa = std::sin(angle);
  SDL_Vertex verts[6];
  SDL_FPoint pts[4];
  for (int i = 0; i < 4; i++) {
    pts[i] = { cx + corners[i].x * ca - corners[i].y * sa, cy + corners[i].x * sa + corners[i].y * ca };
  }
  int idx[6] = { 0, 1, 2, 0, 2, 3 };
  for (int i = 0; i < 6; i++) {
    verts[i].position = pts[idx[i]];
    verts[i].color = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
    verts[i].tex_coord = { 0, 0 };
  }
  SDL_RenderGeometry(renderer_, nullptr, verts, 6, nullptr, 0);
}

void Renderer::thickLine(float x1, float y1, float x2, float y2, float width, SDL_Color c) {
  float dx = x2 - x1, dy = y2 - y1;
  float len = std::hypot(dx, dy);
  if (len < 0.01f) return;
  float nx = -dy / len * width / 2, ny = dx / len * width / 2;
  SDL_FPoint corners[4] = {
    { x1 + nx, y1 + ny }, { x2 + nx, y2 + ny }, { x2 - nx, y2 - ny }, { x1 - nx, y1 - ny },
  };
  SDL_Vertex verts[6];
  int idx[6] = { 0, 1, 2, 0, 2, 3 };
  for (int i = 0; i < 6; i++) {
    verts[i].position = corners[idx[i]];
    verts[i].color = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
    verts[i].tex_coord = { 0, 0 };
  }
  SDL_RenderGeometry(renderer_, nullptr, verts, 6, nullptr, 0);
}

// ---------------- 世界渲染 ----------------

void Renderer::renderWorld(const WorldRenderArgs& a) {
  float ox = a.fx ? a.fx->shakeX() : 0;
  float oy = WORLD_OFFSET_Y + (a.fx ? a.fx->shakeY() : 0);
  drawBackground(ox, oy);
  drawComboGlow(a.combo, ox, oy);
  if (a.bricks) for (const auto& b : *a.bricks) drawBrick(b, ox, oy);
  if (a.respawnQueue) for (const auto& b : *a.respawnQueue) drawRespawnGhost(b, ox, oy);
  if (a.powerUps) for (const auto& p : *a.powerUps) drawPowerUp(p, ox, oy);
  if (a.lasers) drawLasers(*a.lasers, ox, oy);
  if (a.shield) {
    SDL_Color sc = palette().shield;
    fillRect(28 + ox, WORLD_HEIGHT - 17 + oy, WORLD_WIDTH - 56, 5, withAlpha(sc, 255));
    fillRect(28 + ox - 3, WORLD_HEIGHT - 20 + oy, WORLD_WIDTH - 50, 11, withAlpha(sc, 40));
  }
  if (a.paddle) drawPaddle(*a.paddle, ox, oy);
  if (a.balls && a.paddle) drawBalls(*a.balls, *a.paddle, a.stuckBall, a.aimX, a.aimY, ox, oy);
  if (a.fx) drawFx(*a.fx, ox, oy);
  if (a.effects) drawEffectLabels(*a.effects, ox, oy);
  if (a.state == GameState::Paused) fillRect(0, WORLD_OFFSET_Y, WORLD_WIDTH, WORLD_HEIGHT, { 3, 5, 18, 90 });
}

void Renderer::drawBackground(float ox, float oy) {
  Palette p = palette();
  // 三段渐变 surface → variant → surface
  for (int i = 0; i < static_cast<int>(WORLD_HEIGHT); i++) {
    float t = static_cast<float>(i) / WORLD_HEIGHT;
    SDL_Color c;
    if (t < 0.62f) c = mix(p.surface, p.surfaceVariant, t / 0.62f);
    else c = mix(p.surfaceVariant, p.surface, (t - 0.62f) / 0.38f);
    SDL_FRect line = { ox - 24, oy + i, WORLD_WIDTH + 48, 1 };
    setColor(c);
    SDL_RenderFillRect(renderer_, &line);
  }
  drawAmbient(ox, oy);
  drawGrid(ox, oy);
}

void Renderer::drawAmbient(float ox, float oy) {
  if (!bgDotsInit_) {
    bgDotsInit_ = true;
    SDL_Color colors[4] = { hexColor("#42d9ff"), hexColor("#aa7dff"), hexColor("#ff5d8f"), hexColor("#45f5d2") };
    for (int i = 0; i < 34; i++) {
      bgDots_.push_back({ static_cast<float>(std::rand() % 960), static_cast<float>(std::rand() % 600),
                          1.0f + static_cast<float>(std::rand() % 26) / 10.0f,
                          colors[i % 4], 6.0f + static_cast<float>(std::rand() % 180) / 10.0f });
    }
  }
  float time = static_cast<float>(SDL_GetTicksNS()) / 1e9f;
  for (const auto& d : bgDots_) {
    float y = std::fmod(d.y + time * d.speed, WORLD_HEIGHT);
    float alpha = 0.05f + 0.09f * std::abs(std::sin(time * 0.7f + d.x));
    SDL_Color c = withAlpha(d.color, static_cast<Uint8>(alpha * 255));
    setColor(c);
    SDL_FRect r = { d.x + ox, y + oy, d.r * 2, d.r * 2 };
    SDL_RenderFillRect(renderer_, &r);
  }
}

void Renderer::drawGrid(float ox, float oy) {
  setColor(palette().grid);
  for (int x = -24; x <= static_cast<int>(WORLD_WIDTH) + 24; x += 48) {
    SDL_FRect line = { static_cast<float>(x) + ox, oy - 24, 1, WORLD_HEIGHT + 48 };
    SDL_RenderFillRect(renderer_, &line);
  }
  for (int y = -24; y <= static_cast<int>(WORLD_HEIGHT) + 24; y += 48) {
    SDL_FRect line = { ox - 24, static_cast<float>(y) + oy, WORLD_WIDTH + 48, 1 };
    SDL_RenderFillRect(renderer_, &line);
  }
}

void Renderer::drawComboGlow(int combo, float ox, float oy) {
  if (combo < 5) return;
  int r = combo >= 20 ? 255 : combo >= 10 ? 255 : 66;
  int g = combo >= 20 ? 209 : combo >= 10 ? 93 : 217;
  int b = combo >= 20 ? 102 : combo >= 10 ? 143 : 255;
  float alpha = std::min(0.30f, static_cast<float>(combo - 4) * 0.045f);
  const float THICK = 56, STEP = 3;
  int layers = static_cast<int>(THICK / STEP);
  for (int i = 0; i < layers; i++) {
    float a = alpha * (1.0f - static_cast<float>(i) / layers);
    SDL_Color c = { static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a * 255) };
    float t = static_cast<float>(i) * STEP;
    fillRect(ox, oy + t, WORLD_WIDTH, STEP, c);                      // 上
    fillRect(ox, oy + WORLD_HEIGHT - t - STEP, WORLD_WIDTH, STEP, c); // 下
    fillRect(ox + t, oy, STEP, WORLD_HEIGHT, c);                      // 左
    fillRect(ox + WORLD_WIDTH - t - STEP, oy, STEP, WORLD_HEIGHT, c); // 右
  }
}

void Renderer::drawBrick(const BrickState& b, float ox, float oy) {
  const char* col;
  if (b.kind == BrickKind::Rainbow) {
    int idx = std::max(0, std::min(2, b.maxHp - b.hp));
    col = RAINBOW_HP_COLORS[idx];
  } else {
    switch (b.kind) {
      case BrickKind::Tough: col = BRICK_COLOR_TOUGH; break;
      case BrickKind::Indestructible: col = BRICK_COLOR_INDESTRUCTIBLE; break;
      case BrickKind::Bonus: col = BRICK_COLOR_BONUS; break;
      case BrickKind::Explosive: col = BRICK_COLOR_EXPLOSIVE; break;
      case BrickKind::Respawn: col = BRICK_COLOR_RESPAWN; break;
      default: col = BRICK_COLOR_NORMAL; break;
    }
  }
  SDL_Color base = hexColor(col);
  float cx = b.x + b.width / 2, cy = b.y + b.height / 2;
  float scale = 1.0f;
  float inset = 0;
  if (b.hitFlash > 0) {
    scale = 1 + (b.hitFlash / 0.09f) * 0.09f;
    inset = 2;
  }
  float nw = b.width * scale - inset * 2, nh = b.height * scale - inset * 2;
  float nx = cx - nw / 2 + ox, ny = cy - nh / 2 + oy;
  // 发光底
  if (b.kind != BrickKind::Indestructible) {
    fillRoundedRect(nx - 3, ny - 3, nw + 6, nh + 6, 9, withAlpha(base, 26));
  }
  SDL_Color face = b.hitFlash > 0 ? SDL_Color{ 255, 255, 255, 255 } : base;
  fillRoundedRect(nx, ny, nw, nh, 6, face);
  fillRoundedRect(nx + 1, ny + 1, nw - 2, nh - 2, 5, withAlpha(SDL_Color{ 255, 255, 255, 97 }, 97));
  // 符号
  std::string mark;
  if (b.kind == BrickKind::Indestructible) mark = "\xe2\x97\x86";     // ◆
  else if (b.kind == BrickKind::Bonus) mark = "\xe2\x98\x85";         // ★
  else if (b.kind == BrickKind::Explosive) mark = "\xef\xbc\x8a";     // ＊(Noto Sans CJK 无 ✦)
  else if (b.kind == BrickKind::Respawn) mark = "\xe2\x99\xbb";       // ♻(Noto Sans CJK 无 ↻)
  else if (b.maxHp > 1) mark = std::to_string(b.hp);
  if (!mark.empty()) {
    drawText(mark, cx + ox, cy + oy - 7, 13, { 6, 9, 28, 148 }, 1);
  }
}

void Renderer::drawRespawnGhost(const BrickState& b, float ox, float oy) {
  float progress = b.respawnTimer / RESPAWN_TIME;
  float flash = b.respawnTimer < 1 ? ((static_cast<int>(b.respawnTimer * 8) % 2 == 0) ? 1.0f : 0.35f) : 1.0f;
  float alpha = (0.22f + 0.5f * (1 - progress)) * flash;
  SDL_Color c = withAlpha(hexColor(BRICK_COLOR_RESPAWN), static_cast<Uint8>(alpha * 255));
  fillRoundedRect(b.x + ox, b.y + oy, b.width, b.height, 6, c);
  // 虚线边框(4 段)
  SDL_Color edge = withAlpha(hexColor(BRICK_COLOR_RESPAWN), static_cast<Uint8>(alpha * 255));
  const float dash = 6;
  for (float dx = 2; dx < b.width - 2; dx += dash * 2) {
    fillRect(b.x + ox + dx, b.y + oy, std::min(dash, b.width - 2 - dx), 1.5f, edge);
    fillRect(b.x + ox + dx, b.y + oy + b.height - 1.5f, std::min(dash, b.width - 2 - dx), 1.5f, edge);
  }
  for (float dy = 2; dy < b.height - 2; dy += dash * 2) {
    fillRect(b.x + ox, b.y + oy + dy, 1.5f, std::min(dash, b.height - 2 - dy), edge);
    fillRect(b.x + ox + b.width - 1.5f, b.y + oy + dy, 1.5f, std::min(dash, b.height - 2 - dy), edge);
  }
}

void Renderer::drawPowerUp(const PowerUpState& p, float ox, float oy) {
  SDL_Color bg = light_ ? hexColor("#f9f7ff") : hexColor("#25262d");
  SDL_Color edge = light_ ? hexColor("#006b60") : hexColor("#65dbc8");
  fillRoundedRect(p.x - 25 + ox, p.y - 11 + oy, 50, 22, 7, withAlpha(edge, 60));
  fillRoundedRect(p.x - 24 + ox, p.y - 10 + oy, 48, 20, 6, bg);
  SDL_Color fg = light_ ? hexColor("#1b1b21") : hexColor("#e5e1e9");
  drawText(powerLabel(language_, p.kind), p.x + ox, p.y - 6 + oy, 12, fg, 1);
}

void Renderer::drawLasers(const std::vector<LaserState>& lasers, float ox, float oy) {
  SDL_Color pink = hexColor("#ff5d8f");
  for (const auto& l : lasers) {
    fillRect(l.x - 1.5f + ox, l.y + oy, 3, 46, withAlpha(pink, 40));
    fillRect(l.x - 1.5f + ox, l.y + oy, 3, 46, pink);
    fillCircle(l.x + ox, l.y + oy, 3.5f, SDL_Color{ 255, 255, 255, 255 });
  }
}

void Renderer::drawPaddle(const PaddleState& p, float ox, float oy) {
  SDL_Color edge = light_ ? hexColor("#4f5794") : hexColor("#bec2ff");
  SDL_Color center = light_ ? hexColor("#006b60") : hexColor("#65dbc8");
  // 发光底
  fillRoundedRect(p.x - 4 + ox, p.y - 4 + oy, p.width + 8, p.height + 8, (p.height + 8) / 2, withAlpha(center, 26));
  // 横向渐变逐列 + 两端圆,形成胶囊
  int cols = static_cast<int>(p.width);
  for (int i = 0; i < cols; i++) {
    float t = static_cast<float>(i) / std::max(1, cols - 1);
    SDL_Color c = t < 0.5f ? mix(edge, center, t * 2) : mix(center, edge, (t - 0.5f) * 2);
    SDL_FRect seg = { p.x + static_cast<float>(i) + ox, p.y + oy, 1, p.height };
    setColor(c);
    SDL_RenderFillRect(renderer_, &seg);
  }
  float h2 = p.height / 2;
  fillCircle(p.x + h2 + ox, p.y + h2 + oy, h2, edge);
  fillCircle(p.x + p.width - h2 + ox, p.y + h2 + oy, h2, edge);
}

void Renderer::drawBalls(const std::vector<BallState>& balls, const PaddleState& paddle, bool stuck, float aimX, float aimY, float ox, float oy) {
  trails_.resize(balls.size());
  SDL_Color ballCol = hexColor(ballColor_);
  for (size_t i = 0; i < balls.size(); i++) {
    const BallState& ball = balls[i];
    SDL_Color c = ball.piercing ? hexColor("#ffd166") : ballCol;
    if (stuck && i == 0) {
      // 粘球吸附虚线(球→挡板)
      SDL_Color dash = { 255, 255, 255, 115 };
      float y0 = ball.y + ball.radius, y1 = paddle.y;
      for (float yy = y0; yy < y1; yy += 6) {
        fillRect(ball.x - 0.75f + ox, yy + oy, 1.5f, std::min(3.0f, y1 - yy), dash);
      }
      // 瞄准线(球→鼠标)+ 光标环
      if (aimX >= 0 && aimY >= 0) {
        float dx = aimX - ball.x, dy = aimY - ball.y;
        float len = std::hypot(dx, dy);
        if (len > 8) {
          float ux = dx / len, uy = dy / len;
          SDL_Color aim = { 255, 255, 255, 170 };
          const float seg = 8, gap = 5;
          for (float d = ball.radius + 2; d < len; d += seg + gap) {
            float d2 = std::min(d + seg, len);
            thickLine(ball.x + ux * d + ox, ball.y + uy * d + oy,
                      ball.x + ux * d2 + ox, ball.y + uy * d2 + oy, 2.5f, aim);
          }
          strokeCircle(aimX + ox, aimY + oy, 6, 1.5f, { 255, 255, 255, 140 });
        }
      }
    }
    auto& trail = trails_[i];
    trail.push_back({ ball.x, ball.y });
    if (trail.size() > 10) trail.erase(trail.begin());
    if (trailEnabled_) {
      for (size_t j = 0; j < trail.size(); j++) {
        float alpha = static_cast<float>(j) / trail.size() * 0.28f;
        float r = ball.radius * (0.25f + static_cast<float>(j) / trail.size() * 0.55f);
        fillCircle(trail[j].x + ox, trail[j].y + oy, r, withAlpha(c, static_cast<Uint8>(alpha * 255)));
      }
    }
    fillCircle(ball.x + ox, ball.y + oy, ball.radius + 4, withAlpha(c, 30));
    fillCircle(ball.x + ox, ball.y + oy, ball.radius, c);
  }
}

void Renderer::drawFx(const FxSystem& fx, float ox, float oy) {
  // 碎片(旋转)
  for (const auto& d : fx.debris()) {
    float alpha = std::max(0.0f, d.life / d.maxLife);
    fillRotatedRect(d.x + ox, d.y + oy, d.size, d.rotation, withAlpha(hexColor(d.color), static_cast<Uint8>(alpha * 255)));
  }
  // 粒子
  for (const auto& p : fx.particles) {
    float alpha = std::max(0.0f, p.life / p.maxLife);
    fillRect(p.x + ox, p.y + oy, p.size, p.size, withAlpha(hexColor(p.color), static_cast<Uint8>(alpha * 255)));
  }
  // 圆环
  for (const auto& r : fx.rings()) {
    float alpha = std::max(0.0f, r.life / r.maxLife) * 0.8f;
    strokeCircle(r.x + ox, r.y + oy, r.radius, 2.5f, withAlpha(hexColor(r.color), static_cast<Uint8>(alpha * 255)));
  }
  // 飘字(黑描边 + 主色)
  for (const auto& t : fx.texts()) {
    float alpha = std::max(0.0f, t.life / t.maxLife);
    SDL_Color dark = { 6, 9, 28, static_cast<Uint8>(217 * alpha) };
    drawText(t.text, t.x + ox - 1, t.y + oy - 1, t.size, dark, 1, alpha);
    drawText(t.text, t.x + ox + 1, t.y + oy - 1, t.size, dark, 1, alpha);
    drawText(t.text, t.x + ox - 1, t.y + oy + 1, t.size, dark, 1, alpha);
    drawText(t.text, t.x + ox + 1, t.y + oy + 1, t.size, dark, 1, alpha);
    drawText(t.text, t.x + ox, t.y + oy, t.size, hexColor(t.color), 1, alpha);
  }
  // 全屏闪光
  for (const auto& f : fx.flashes()) {
    float alpha = std::max(0.0f, f.life / f.maxLife);
    fillRect(ox - 24, oy - 24, WORLD_WIDTH + 48, WORLD_HEIGHT + 48, withAlpha(hexColor(f.color), static_cast<Uint8>(alpha * 255)));
  }
}

void Renderer::drawEffectLabels(const std::vector<ActiveEffect>& effects, float ox, float oy) {
  if (effects.empty()) return;
  SDL_Color fg = palette().onSurface;
  for (size_t i = 0; i < effects.size(); i++) {
    std::string label = std::string(powerLabel(language_, effects[i].kind)) + " " +
                        std::to_string(static_cast<int>(std::ceil(effects[i].remaining))) + "s";
    drawText(label, WORLD_WIDTH - 18 + ox, WORLD_HEIGHT - 44 - static_cast<float>(i) * 19 + oy, 13, fg, 2);
  }
}

} // namespace neon
