#pragma once
#include <SDL3/SDL.h>

#include <functional>
#include <string>
#include <vector>

#include "types.h"

namespace neon {

class Game;
class Renderer;

// 与 web 版 index.html + main.ts + style.css 对应的 UI 层:按钮系统、HUD、overlay、菜单、选关、设置对话框
class UI {
public:
  UI(Renderer* renderer, Game* game);

  void setSnapshot(const GameSnapshot& s); // game.emit 回调:重建按钮/文本/布局
  void handleEvent(const SDL_Event& e);    // 鼠标点击 + Esc 关闭设置
  void render();                           // 绘制 HUD / overlay / 对话框 / toast
  bool settingsOpen() const { return settingsOpen_; }

private:
  struct Btn {
    SDL_FRect rect{};
    std::string key;    // i18n 键(为空则用 label)
    std::string label;  // 当前语言文本
    int size = 15;
    enum class Style { Primary, Secondary, Tonal, Ghost, Segment, Switch, Swatch, LevelCard } style = Style::Tonal;
    bool visible = true;
    bool active = false; // 分段选中 / switch 开 / swatch 选中
    int levelIndex = -1;
    std::string swatchColor;
    std::function<void()> onClick;
  };

  Renderer* r_;
  Game* g_;
  GameSnapshot snap_;
  bool settingsOpen_ = false;
  bool menuViewLevels_ = false;
  std::string toast_;
  uint64_t toastStart_ = 0;
  GameState lastState_ = GameState::Menu;
  std::vector<Btn> buttons_;

  void rebuildButtons();
  void rebuildSettingsButtons();
  void click(float x, float y);
  bool hit(const Btn& b, float x, float y) const;
  Btn& add(float x, float y, float w, float h, Btn::Style style, int size);

  void drawTopbar();
  void drawStats();
  void drawFooter();
  void drawOverlay();
  void drawSettingsDialog();
  void drawToast();
  SDL_Color uiColor(const char* hex, Uint8 alpha = 255) const;
};

} // namespace neon
