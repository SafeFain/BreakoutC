#include "ui.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "constants.h"
#include "game.h"
#include "i18n.h"
#include "levels.h"
#include "renderer.h"

namespace neon {

static constexpr float TOPBAR_H = 40;
static constexpr float STATS_H = 44;
static constexpr float FOOTER_H = 16;
static constexpr float GAME_Y = TOPBAR_H + STATS_H;                    // 84
static constexpr float CARD_W = 430;

static SDL_Color hexC(const char* hex, Uint8 alpha = 255) {
  auto v = [](char ch) -> int { return (ch >= '0' && ch <= '9') ? ch - '0' : (ch | 0x20) - 'a' + 10; };
  SDL_Color c = { 255, 255, 255, alpha };
  if (hex && hex[0] == '#' && std::strlen(hex) >= 7) {
    c.r = static_cast<Uint8>(v(hex[1]) * 16 + v(hex[2]));
    c.g = static_cast<Uint8>(v(hex[3]) * 16 + v(hex[4]));
    c.b = static_cast<Uint8>(v(hex[5]) * 16 + v(hex[6]));
  }
  return c;
}

static const char* SWATCH_COLORS[8] = { "#f5fbff", "#54e7ff", "#b69cff", "#ff7eb6", "#ffd166", "#45f5d2", "#ff5d8f", "#aa7dff" };

UI::UI(Renderer* renderer, Game* game) : r_(renderer), g_(game) {}

SDL_Color UI::uiColor(const char* hex, Uint8 alpha) const { return hexC(hex, alpha); }

void UI::setSnapshot(const GameSnapshot& s) {
  bool langChanged = (s.language != snap_.language);
  snap_ = s;
  if (s.state != lastState_) {
    if (s.state == GameState::Ready && lastState_ == GameState::LevelComplete) {
      const char* name = s.language == Language::EN ? englishLevelNames[s.level - 1] : s.levelName.c_str();
      char buf[128];
      std::snprintf(buf, sizeof(buf), s.language == Language::EN ? "Level %d \xc2\xb7 %s" : "\xe7\xac\xac %d \xe5\x85\xb3 \xc2\xb7 %s", s.level, name);
      toast_ = buf;
      toastStart_ = SDL_GetTicksNS();
    }
    lastState_ = s.state;
  }
  if (langChanged) { menuViewLevels_ = false; if (!settingsOpen_) settingsOpen_ = false; }
  rebuildButtons();
}

void UI::rebuildButtons() {
  buttons_.clear();
  if (settingsOpen_) { rebuildSettingsButtons(); return; }
  auto& b = buttons_;
  Language lang = snap_.language;
  auto k = [&](const char* key) { return t(lang, key); };

  // 游戏区卡片位置
  float cx = (Renderer::WINDOW_W - CARD_W) / 2;
  float cy = GAME_Y + 58;

  auto addButton = [&](float x, float y, float w, float h, Btn::Style style, int size, const char* keyOrLabel, bool useKey) {
    Btn btn;
    btn.rect = { x, y, w, h };
    btn.style = style;
    btn.size = size;
    if (useKey) { btn.key = keyOrLabel; btn.label = k(keyOrLabel); }
    else btn.label = keyOrLabel;
    b.push_back(btn);
    return b.back();
  };

  if (snap_.state == GameState::Menu) {
    if (menuViewLevels_) {
      // 选关面板:5 个关卡按钮(2 列)+ 返回
      float bw = (CARD_W - 48 - 10) / 2;
      for (int i = 0; i < 5; i++) {
        float bx = cx + 24 + (i % 2) * (bw + 10);
        float by = cy + 10 + (i / 2) * 60;
        std::string label = std::to_string(i + 1) + ". " + (lang == Language::EN ? englishLevelNames[i] : LEVELS[static_cast<size_t>(i)].name.c_str());
        Btn btn;
        btn.rect = { bx, by, bw, 52 };
        btn.style = Btn::Style::LevelCard;
        btn.size = 15;
        btn.label = label;
        btn.levelIndex = i;
        b.push_back(btn);
      }
      addButton(cx + 24, cy + 10 + 3 * 60 + 8, CARD_W - 48, 44, Btn::Style::Tonal, 15, "back", true);
    } else {
      addButton(cx + 24, cy + 118, CARD_W - 48, 48, Btn::Style::Primary, 17, "start", true);
      addButton(cx + 24, cy + 174, CARD_W - 48, 44, Btn::Style::Tonal, 15, "levelSelect", true);
      addButton(cx + 24, cy + 226, CARD_W - 48, 44, Btn::Style::Tonal, 15, "settings", true);
    }
  } else if (snap_.state != GameState::Playing) {
    addButton(cx + 24, cy + 118, CARD_W - 48, 48, Btn::Style::Primary, 17, snap_.state == GameState::Ready ? "launch" : snap_.state == GameState::LevelComplete ? "nextLevel" : snap_.state == GameState::GameOver ? "restart" : snap_.state == GameState::Victory ? "again" : "resume", true);
    if (snap_.state == GameState::Paused || snap_.state == GameState::LevelComplete) {
      addButton(cx + 24, cy + 174, CARD_W - 48, 44, Btn::Style::Secondary, 15, "restart", true);
    }
  }

  // topbar 按钮
  float tx = Renderer::WINDOW_W - 16;
  auto topBtn = [&](float& x, const char* key, Btn::Style style) {
    float w = 64;
    x -= w;
    Btn btn;
    btn.rect = { x, 8, w - 6, 26 };
    btn.style = style;
    btn.size = 13;
    btn.key = key;
    btn.label = k(key);
    b.push_back(btn);
  };
  float right = tx;
  topBtn(right, "settings", Btn::Style::Ghost);
  topBtn(right, "sound", Btn::Style::Ghost);
  topBtn(right, snap_.state == GameState::Paused ? "resume" : "pause", Btn::Style::Ghost);
}

void UI::rebuildSettingsButtons() {
  Language lang = snap_.language;
  auto k = [&](const char* key) { return t(lang, key); };
  auto& b = buttons_;
  float cw = 520;
  float cx = (Renderer::WINDOW_W - cw) / 2;
  float cy = 96;

  auto seg = [&](float x, float y, float w, const char* key, bool active) {
    Btn btn;
    btn.rect = { x, y, w, 34 };
    btn.style = Btn::Style::Segment;
    btn.size = 14;
    btn.key = key;
    btn.label = k(key);
    btn.active = active;
    b.push_back(btn);
  };
  // 语言
  seg(cx + 190, cy + 78, 100, "chinese", snap_.language == Language::ZH);
  seg(cx + 296, cy + 78, 100, "english", snap_.language == Language::EN);
  // 主题
  seg(cx + 120, cy + 148, 120, "system", snap_.themeMode == ThemeMode::System);
  seg(cx + 246, cy + 148, 90, "light", snap_.themeMode == ThemeMode::Light);
  seg(cx + 342, cy + 148, 90, "dark", snap_.themeMode == ThemeMode::Dark);
  // 拖尾 switch
  {
    Btn btn;
    btn.rect = { cx + 420, cy + 200, 46, 26 };
    btn.style = Btn::Style::Switch;
    btn.active = snap_.ballTrail;
    b.push_back(btn);
  }
  // 粒子特效 switch
  {
    Btn btn;
    btn.rect = { cx + 420, cy + 240, 46, 26 };
    btn.style = Btn::Style::Switch;
    btn.active = snap_.impactFx;
    b.push_back(btn);
  }
  // 球色 swatch
  for (int i = 0; i < 8; i++) {
    Btn btn;
    btn.rect = { cx + 150 + static_cast<float>(i) * 44, cy + 318, 34, 34 };
    btn.style = Btn::Style::Swatch;
    btn.swatchColor = SWATCH_COLORS[i];
    std::string lower = snap_.ballColor;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::string sc = SWATCH_COLORS[i];
    std::transform(sc.begin(), sc.end(), sc.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    btn.active = (lower == sc);
    b.push_back(btn);
  }
  // 完成
  Btn done;
  done.rect = { cx + 24, cy + 386, cw - 48, 46 };
  done.style = Btn::Style::Primary;
  done.size = 16;
  done.key = "close";
  done.label = k("close");
  b.push_back(done);
}

void UI::handleEvent(const SDL_Event& e) {
  if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
    float lx = e.button.x, ly = e.button.y;
    r_->screenToLogical(lx, ly); // 窗口像素 → 逻辑坐标(适配窗口缩放)
    click(lx, ly);
  } else if (e.type == SDL_EVENT_KEY_DOWN &&
             (e.key.scancode == SDL_SCANCODE_ESCAPE || e.key.scancode == SDL_SCANCODE_AC_BACK) && settingsOpen_) {
    settingsOpen_ = false;
    rebuildButtons();
  }
}

bool UI::hit(const Btn& b, float x, float y) const {
  return b.visible && x >= b.rect.x && x <= b.rect.x + b.rect.w && y >= b.rect.y && y <= b.rect.y + b.rect.h;
}

void UI::click(float x, float y) {
  for (const Btn& btn : buttons_) {
    if (!hit(btn, x, y)) continue;
    switch (btn.style) {
      case Btn::Style::LevelCard:
        if (btn.levelIndex >= 0) { g_->selectLevel(btn.levelIndex); }
        break;
      case Btn::Style::Primary:
      case Btn::Style::Secondary: {
        const std::string& key = btn.key;
        if (key == "start" || key == "launch" || key == "nextLevel" || key == "again" || key == "resume") g_->primaryAction();
        else if (key == "restart") g_->restartAction();
        else if (key == "close") { settingsOpen_ = false; rebuildButtons(); }
        break;
      }
      case Btn::Style::Tonal:
        if (btn.key == "levelSelect") { menuViewLevels_ = true; rebuildButtons(); }
        else if (btn.key == "back") { menuViewLevels_ = false; rebuildButtons(); }
        else if (btn.key == "settings") { g_->autoPause(); settingsOpen_ = true; rebuildButtons(); }
        break;
      case Btn::Style::Ghost:
        if (btn.key == "sound") g_->toggleSound();
        else if (btn.key == "pause" || btn.key == "resume") g_->pauseAction();
        else if (btn.key == "settings") { g_->autoPause(); settingsOpen_ = true; rebuildButtons(); }
        break;
      case Btn::Style::Segment: {
        if (btn.key == "chinese") g_->updatePreferences(Language::ZH, snap_.ballTrail, snap_.ballColor, snap_.themeMode, snap_.impactFx);
        else if (btn.key == "english") g_->updatePreferences(Language::EN, snap_.ballTrail, snap_.ballColor, snap_.themeMode, snap_.impactFx);
        else if (btn.key == "system") g_->updatePreferences(snap_.language, snap_.ballTrail, snap_.ballColor, ThemeMode::System, snap_.impactFx);
        else if (btn.key == "light") g_->updatePreferences(snap_.language, snap_.ballTrail, snap_.ballColor, ThemeMode::Light, snap_.impactFx);
        else if (btn.key == "dark") g_->updatePreferences(snap_.language, snap_.ballTrail, snap_.ballColor, ThemeMode::Dark, snap_.impactFx);
        break;
      }
      case Btn::Style::Switch: {
        if (btn.rect.y < 300) g_->updatePreferences(snap_.language, !snap_.ballTrail, snap_.ballColor, snap_.themeMode, snap_.impactFx);
        else g_->updatePreferences(snap_.language, snap_.ballTrail, snap_.ballColor, snap_.themeMode, !snap_.impactFx);
        break;
      }
      case Btn::Style::Swatch:
        g_->updatePreferences(snap_.language, snap_.ballTrail, btn.swatchColor, snap_.themeMode, snap_.impactFx);
        break;
    }
    return;
  }
}

// ---------------- 绘制 ----------------

void UI::render() {
  drawTopbar();
  drawStats();
  drawFooter();
  if (snap_.state != GameState::Playing) drawOverlay();
  if (settingsOpen_) drawSettingsDialog();
  drawToast();
}

void UI::drawTopbar() {
  Palette p = r_->palette();
  r_->fillRect(0, 0, Renderer::WINDOW_W, TOPBAR_H, hexC(r_->lightTheme() ? "#f2f0f8" : "#0e1014", 255));
  Language lang = snap_.language;
  r_->drawText("NEON BREAKER", 16, 9, 14, p.primary, 0);
  std::string sub = std::string(t(lang, "subtitle"));
  r_->drawText(sub, 150, 12, 11, p.onSurface, 0);
  // 按钮(从按钮列表取 ghost)
  for (const Btn& btn : buttons_) {
    if (btn.style != Btn::Style::Ghost) continue;
    bool disabled = btn.key == "pause" || btn.key == "resume";
    if (disabled && (snap_.state == GameState::Menu || snap_.state == GameState::GameOver ||
                     snap_.state == GameState::Victory || snap_.state == GameState::LevelComplete))
      continue;
    if (btn.key == "sound") {
      std::string label = std::string(snap_.soundEnabled ? "\xe2\x99\xaa " : "") + std::string(t(snap_.language, "sound")) +
                          (snap_.soundEnabled ? "" : " \xc3\x97"); // ×(Noto Sans CJK 无 ⚫)
      r_->drawText(label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 6, 13, p.onSurface, 1, 0.85f);
    } else {
      r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 6, 13, p.onSurface, 1, 0.85f);
    }
  }
}

void UI::drawStats() {
  Palette p = r_->palette();
  r_->fillRect(0, TOPBAR_H, Renderer::WINDOW_W, STATS_H, hexC(r_->lightTheme() ? "#f9f7ff" : "#111318", 255));
  Language lang = snap_.language;
  const float colW = Renderer::WINDOW_W / 5.0f;
  auto stat = [&](int col, const char* labelKey, const std::string& value, const std::string& small = "") {
    float cx = colW * col + colW / 2;
    r_->drawText(t(lang, labelKey), cx, TOPBAR_H + 4, 11, p.onSurface, 1, 0.6f);
    float vw = r_->textWidth(value, 16);
    r_->drawText(value, cx - vw / 2, TOPBAR_H + 20, 16, p.onSurface, 0);
    if (!small.empty()) r_->drawText(small, cx + vw / 2 + 6, TOPBAR_H + 24, 11, p.primary, 0);
  };
  stat(0, "score", std::to_string(snap_.score));
  stat(1, "highScore", std::to_string(snap_.highScore));
  std::string levelTxt = std::to_string(snap_.level) + " / " + std::to_string(snap_.levelCount);
  stat(2, "level", levelTxt, snap_.language == Language::EN ? englishLevelNames[snap_.level - 1] : snap_.levelName);
  std::string livesTxt;
  for (int i = 0; i < snap_.lives; i++) livesTxt += "\xe2\x99\xa5 ";
  if (livesTxt.empty()) livesTxt = "\xe2\x80\x94";
  stat(3, "lives", livesTxt);
  char comboBuf[32];
  std::snprintf(comboBuf, sizeof(comboBuf), "\xc3\x97%.1f", snap_.multiplier);
  std::string comboSmall = std::to_string(snap_.combo) + " " + t(lang, "combo");
  std::transform(comboSmall.begin(), comboSmall.end(), comboSmall.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  stat(4, "comboMultiplier", comboBuf, comboSmall);
}

void UI::drawFooter() {
  Palette p = r_->palette();
  r_->fillRect(0, Renderer::WINDOW_H - FOOTER_H, Renderer::WINDOW_W, FOOTER_H, hexC(r_->lightTheme() ? "#f2f0f8" : "#0e1014", 255));
  const char* hint = snap_.inputMethod == InputMethod::Touch ? t(snap_.language, "touchHint") : t(snap_.language, "keyboardHint");
  r_->drawText(hint, Renderer::WINDOW_W / 2, Renderer::WINDOW_H - 13, 11, p.onSurface, 1, 0.55f);
}

void UI::drawOverlay() {
  float gy = GAME_Y;
  // 半透明遮罩(游戏区)
  r_->fillRect(0, gy, Renderer::WINDOW_W, WORLD_HEIGHT, { 4, 7, 24, 148 });
  float cx = (Renderer::WINDOW_W - CARD_W) / 2;
  float cy = gy + 42;
  Palette p = r_->palette();
  r_->fillRoundedRect(cx, cy, CARD_W, 296, 28, hexC(r_->lightTheme() ? "#f0eef6" : "#1a1b22", 255));

  Language lang = snap_.language;
  const char* eyebrow, *title, *text, *primaryKey;
  switch (snap_.state) {
    case GameState::Menu: eyebrow = t(lang, "menuEyebrow"); title = t(lang, "menuTitle"); text = t(lang, "menuText"); primaryKey = "start"; break;
    case GameState::Ready: eyebrow = t(lang, "readyEyebrow"); title = t(lang, "readyTitle"); text = t(lang, "readyText"); primaryKey = "launch"; break;
    case GameState::Paused: eyebrow = t(lang, "pausedEyebrow"); title = t(lang, "pausedTitle"); text = t(lang, "pausedText"); primaryKey = "resume"; break;
    case GameState::LevelComplete: eyebrow = t(lang, "levelEyebrow"); title = t(lang, "levelTitle"); text = t(lang, "levelText"); primaryKey = "nextLevel"; break;
    case GameState::GameOver: eyebrow = t(lang, "gameOverEyebrow"); title = t(lang, "gameOverTitle"); text = t(lang, "gameOverText"); primaryKey = "restart"; break;
    case GameState::Victory: eyebrow = t(lang, "victoryEyebrow"); title = t(lang, "victoryTitle"); text = t(lang, "victoryText"); primaryKey = "again"; break;
    default: return;
  }
  (void)primaryKey;
  r_->drawText(eyebrow, cx + CARD_W / 2, cy + 26, 14, p.primary, 1);
  r_->drawText(title, cx + CARD_W / 2, cy + 50, 40, p.onSurface, 1);
  r_->drawText(text, cx + CARD_W / 2, cy + 96, 14, p.onSurface, 1, 0.75f);

  // 选关面板:额外标题行
  if (snap_.state == GameState::Menu && menuViewLevels_) {
    r_->drawText(t(lang, "levelSelect"), cx + CARD_W / 2, cy + 126, 17, p.onSurface, 1);
  }

  // 按钮
  for (const Btn& btn : buttons_) {
    if (btn.style == Btn::Style::Ghost) continue;
    switch (btn.style) {
      case Btn::Style::Primary:
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 24, p.primary);
        r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 14, static_cast<float>(btn.size), hexC("#ffffff"), 1);
        break;
      case Btn::Style::Secondary: {
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 22, { 0, 0, 0, 0 });
        SDL_Color edge = p.primary; edge.a = 140;
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 22, edge);
        r_->fillRoundedRect(btn.rect.x + 1, btn.rect.y + 1, btn.rect.w - 2, btn.rect.h - 2, 21, hexC(r_->lightTheme() ? "#f0eef6" : "#1a1b22"));
        r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 13, static_cast<float>(btn.size), p.primary, 1);
        break;
      }
      case Btn::Style::Tonal:
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 22, p.primary);
        r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 13, static_cast<float>(btn.size), hexC(r_->lightTheme() ? "#1b1b21" : "#ffffff"), 1);
        break;
      case Btn::Style::LevelCard:
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 14, hexC(r_->lightTheme() ? "#e9e7f0" : "#23242b"));
        r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 16, static_cast<float>(btn.size), p.onSurface, 1);
        break;
      case Btn::Style::Segment:
        if (btn.active) {
          r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 12, p.primary);
          r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 9, static_cast<float>(btn.size), hexC("#ffffff"), 1);
        } else {
          r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 12, { 0, 0, 0, 0 });
          SDL_Color edge = p.primary; edge.a = 90;
          r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 12, edge);
          r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 9, static_cast<float>(btn.size), p.onSurface, 1);
        }
        break;
      case Btn::Style::Switch: {
        SDL_Color bg = btn.active ? p.primary : hexC("#6b6e78");
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 13, bg);
        float knobX = btn.active ? btn.rect.x + btn.rect.w - 22 : btn.rect.x + 4;
        r_->fillCircle(knobX + 9, btn.rect.y + 13, 9, hexC("#ffffff"));
        break;
      }
      case Btn::Style::Swatch: {
        r_->fillCircle(btn.rect.x + 17, btn.rect.y + 17, 16, hexC(btn.swatchColor.c_str()));
        if (btn.active) {
          SDL_Color ring = p.primary;
          r_->strokeCircle(btn.rect.x + 17, btn.rect.y + 17, 17, 2, ring);
        }
        break;
      }
      default: break;
    }
  }
}

void UI::drawSettingsDialog() {
  Palette p = r_->palette();
  r_->fillRect(0, 0, Renderer::WINDOW_W, Renderer::WINDOW_H, { 0, 0, 0, 130 });
  float cw = 520, ch = 470;
  float cx = (Renderer::WINDOW_W - cw) / 2, cy = 96;
  r_->fillRoundedRect(cx, cy, cw, ch, 24, hexC(r_->lightTheme() ? "#f6f4fb" : "#1a1b22", 255));
  Language lang = snap_.language;

  r_->drawText(t(lang, "settingsTitle"), cx + 24, cy + 20, 22, p.onSurface, 0);
  r_->drawText(t(lang, "settingsDesc"), cx + 24, cy + 52, 12, p.onSurface, 0, 0.6f);

  auto rowLabel = [&](float y, const char* key, const char* desc = nullptr) {
    r_->drawText(t(lang, key), cx + 24, y, 14, p.onSurface, 0);
    if (desc) r_->drawText(t(lang, desc), cx + 24, y + 18, 11, p.onSurface, 0, 0.55f);
  };
  rowLabel(cy + 92, "language");
  rowLabel(cy + 162, "theme");
  rowLabel(cy + 212, "trail", "trailDesc");
  rowLabel(cy + 252, "fx", "fxDesc");
  r_->drawText(t(lang, "ballColor"), cx + 24, cy + 332, 14, p.onSurface, 0);

  // 绘制对话框内按钮
  for (const Btn& btn : buttons_) {
    switch (btn.style) {
      case Btn::Style::Segment:
        if (btn.active) {
          r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 12, p.primary);
          r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 9, static_cast<float>(btn.size), hexC("#ffffff"), 1);
        } else {
          SDL_Color edge = p.primary; edge.a = 90;
          r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 12, edge);
          r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 9, static_cast<float>(btn.size), p.onSurface, 1);
        }
        break;
      case Btn::Style::Switch: {
        SDL_Color bg = btn.active ? p.primary : hexC("#6b6e78");
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 13, bg);
        float knobX = btn.active ? btn.rect.x + btn.rect.w - 22 : btn.rect.x + 4;
        r_->fillCircle(knobX + 9, btn.rect.y + 13, 9, hexC("#ffffff"));
        break;
      }
      case Btn::Style::Swatch:
        r_->fillCircle(btn.rect.x + 17, btn.rect.y + 17, 16, hexC(btn.swatchColor.c_str()));
        if (btn.active) r_->strokeCircle(btn.rect.x + 17, btn.rect.y + 17, 17, 2, p.primary);
        break;
      case Btn::Style::Primary:
        r_->fillRoundedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, 23, p.primary);
        r_->drawText(btn.label, btn.rect.x + btn.rect.w / 2, btn.rect.y + 13, static_cast<float>(btn.size), hexC("#ffffff"), 1);
        break;
      default: break;
    }
  }
}

void UI::drawToast() {
  if (toast_.empty()) return;
  uint64_t now = SDL_GetTicksNS();
  float elapsed = static_cast<float>(now - toastStart_) / 1e9f;
  if (elapsed > 1.5f) { toast_.clear(); return; }
  float alpha = elapsed > 1.2f ? (1.5f - elapsed) / 0.3f : 1.0f;
  float w = r_->textWidth(toast_, 15) + 32;
  float x = (Renderer::WINDOW_W - w) / 2, y = Renderer::WINDOW_H - 46;
  r_->fillRoundedRect(x, y, w, 32, 16, hexC("#1a1b22", static_cast<Uint8>(230 * alpha)));
  r_->drawText(toast_, Renderer::WINDOW_W / 2, y + 7, 15, hexC("#e5e1e9", static_cast<Uint8>(255 * alpha)), 1);
}

} // namespace neon
