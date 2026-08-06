#pragma once
#include <SDL3/SDL.h>

#include <cstdint>
#include <functional>
#include <unordered_set>

#include "types.h"

namespace neon {

// 与 web 版 input.ts 对应:事件 → 队列 → 每帧 poll 消费
class InputManager {
public:
  InputManager(SDL_FRect gameRect, std::function<void(InputMethod)> onMethod);

  void handleEvent(const SDL_Event& e);
  void setPointerActive(bool active); // overlay 可见时禁用指针输入(等价于 overlay 遮挡 canvas)
  void setScreenTransform(float scale, float ox, float oy, int winW, int winH) {
    scale_ = scale; offsetX_ = ox; offsetY_ = oy; winW_ = winW; winH_ = winH;
  }
  InputActions poll();
  void queuePrimary() { primaryQueued_ = true; }
  void queuePause() { pauseQueued_ = true; }
  void queueRestart() { restartQueued_ = true; }
  void clear();
  InputMethod inputMethod() const { return method_; }

private:
  SDL_FRect gameRect_;
  std::function<void(InputMethod)> onMethod_;
  std::unordered_set<SDL_Scancode> keys_;
  bool pointerActive_ = true;
  Uint64 activePointer_ = UINT64_MAX;
  float pointerWorldX_ = -1;
  float pointerWorldY_ = -1;
  float scale_ = 1, offsetX_ = 0, offsetY_ = 0;
  int winW_ = 960, winH_ = 700;
  bool primaryQueued_ = false, pauseQueued_ = false, restartQueued_ = false;
  InputMethod method_ = InputMethod::Keyboard;

  void setMethod(InputMethod m);
  float mapX(float screenX) const { return (screenX - gameRect_.x) / std::max(1.0f, gameRect_.w) * 960.0f; }
  float mapY(float screenY) const { return (screenY - gameRect_.y) / std::max(1.0f, gameRect_.h) * 600.0f; }
  bool inGameArea(float x, float y) const;
  void setPointer(float worldX, float worldY);
};

} // namespace neon
