#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

namespace neon {

class FxSystem;

struct Palette { SDL_Color surface, surfaceVariant, grid, onSurface, primary, shield; };

// 世界渲染参数(指针引用,避免拷贝)
struct WorldRenderArgs {
  GameState state = GameState::Menu;
  const std::vector<BallState>* balls = nullptr;
  const PaddleState* paddle = nullptr;
  const std::vector<BrickState>* bricks = nullptr;
  const std::vector<BrickState>* respawnQueue = nullptr;
  const std::vector<PowerUpState>* powerUps = nullptr;
  const std::vector<LaserState>* lasers = nullptr;
  FxSystem* fx = nullptr;
  bool shield = false;
  const std::vector<ActiveEffect>* effects = nullptr;
  int combo = 0;
  bool stuckBall = false;
  float aimX = -1, aimY = -1; // 粘球瞄准目标(世界坐标,<0 表示无)
  Language language = Language::ZH;
};

// 与 web 版 renderer.ts 对应:SDL 渲染 + SDL_ttf 文本(带纹理缓存)
class Renderer {
public:
  static constexpr int WINDOW_W = 960;
  static constexpr int WINDOW_H = 700;
  static constexpr float WORLD_OFFSET_Y = 84.0f; // 顶部 HUD 高度

  Renderer() = default;
  ~Renderer() { shutdown(); }
  bool init(SDL_Window* window, SDL_Renderer* renderer);
  void shutdown(); // 在 SDL_DestroyRenderer / TTF_Quit 之前显式清理
  SDL_Renderer* sdlRenderer() { return renderer_; }

  void configure(const std::string& ballColor, bool trailEnabled, Language lang, ThemeMode theme);
  void renderWorld(const WorldRenderArgs& args);
  void clearTextCache();
  // 窗口自适应:等比缩放 + 居中(letterbox),逻辑坐标恒为 WINDOW_W×WINDOW_H
  void onWindowResized(int newW, int newH);
  void screenToLogical(float& x, float& y) const;
  float scale() const { return scale_; }
  float offsetX() const { return offsetX_; }
  float offsetY() const { return offsetY_; }
  int windowWidth() const { return winW_; }
  int windowHeight() const { return winH_; }

  // UI 原语(屏幕坐标)
  void drawText(const std::string& text, float x, float y, float size, SDL_Color color, int align = 0, float alpha = 1.0f); // align 0=left 1=center 2=right
  float textWidth(const std::string& text, float size);
  void fillRect(float x, float y, float w, float h, SDL_Color c);
  void fillRoundedRect(float x, float y, float w, float h, float radius, SDL_Color c);
  void fillCircle(float cx, float cy, float r, SDL_Color c);
  void strokeCircle(float cx, float cy, float r, float width, SDL_Color c);
  void fillRotatedRect(float cx, float cy, float size, float angle, SDL_Color c);
  void thickLine(float x1, float y1, float x2, float y2, float width, SDL_Color c);
  Palette palette() const;
  bool lightTheme() const { return light_; }

private:
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
  TTF_Font* font_ = nullptr;
  std::string ballColor_ = "#f5fbff";
  bool trailEnabled_ = true;
  Language language_ = Language::ZH;
  bool light_ = false;
  int winW_ = WINDOW_W, winH_ = WINDOW_H;
  float scale_ = 1, offsetX_ = 0, offsetY_ = 0;
  struct TextTex { SDL_Texture* tex = nullptr; int w = 0, h = 0; };
  std::unordered_map<std::string, TextTex> cache_;
  std::vector<std::vector<SDL_FPoint>> trails_;
  struct BgDot { float x, y, r; SDL_Color color; float speed; };
  std::vector<BgDot> bgDots_;
  bool bgDotsInit_ = false;

  SDL_Color hexColor(const std::string& s) const;
  TextTex* textTexture(const std::string& text, float size, SDL_Color c);
  void setColor(SDL_Color c) const;

  // 世界绘制(ox, oy 为世界坐标→屏幕坐标偏移,含屏震)
  void drawBackground(float ox, float oy);
  void drawAmbient(float ox, float oy);
  void drawGrid(float ox, float oy);
  void drawComboGlow(int combo, float ox, float oy);
  void drawBrick(const BrickState& b, float ox, float oy);
  void drawRespawnGhost(const BrickState& b, float ox, float oy);
  void drawPowerUp(const PowerUpState& p, float ox, float oy);
  void drawLasers(const std::vector<LaserState>& lasers, float ox, float oy);
  void drawPaddle(const PaddleState& p, float ox, float oy);
  void drawBalls(const std::vector<BallState>& balls, const PaddleState& paddle, bool stuck, float aimX, float aimY, float ox, float oy);
  void drawFx(const FxSystem& fx, float ox, float oy);
  void drawEffectLabels(const std::vector<ActiveEffect>& effects, float ox, float oy);
};

} // namespace neon
