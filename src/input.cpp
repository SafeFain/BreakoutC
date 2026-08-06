#include "input.h"

#include <algorithm>

namespace neon {

InputManager::InputManager(SDL_FRect gameRect, std::function<void(InputMethod)> onMethod)
    : gameRect_(gameRect), onMethod_(std::move(onMethod)) {}

void InputManager::setPointerActive(bool active) {
  pointerActive_ = active;
  if (!active) { pointerWorldX_ = -1; pointerWorldY_ = -1; }
}

void InputManager::setMethod(InputMethod m) {
  if (method_ != m) { method_ = m; if (onMethod_) onMethod_(m); }
}

bool InputManager::inGameArea(float x, float y) const {
  return x >= gameRect_.x && x <= gameRect_.x + gameRect_.w &&
         y >= gameRect_.y && y <= gameRect_.y + gameRect_.h;
}

void InputManager::setPointer(float worldX, float worldY) {
  pointerWorldX_ = worldX;
  pointerWorldY_ = worldY;
}

void InputManager::handleEvent(const SDL_Event& e) {
  switch (e.type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Scancode sc = e.key.scancode;
      setMethod(InputMethod::Keyboard);
      pointerWorldX_ = -1;
      if (!e.key.repeat) {
        if (sc == SDL_SCANCODE_SPACE) primaryQueued_ = true;
        if (sc == SDL_SCANCODE_ESCAPE || sc == SDL_SCANCODE_AC_BACK) pauseQueued_ = true; // Android 返回键等效 Esc
        if (sc == SDL_SCANCODE_R) restartQueued_ = true;
      }
      keys_.insert(sc);
      break;
    }
    case SDL_EVENT_KEY_UP:
      keys_.erase(e.key.scancode);
      break;

    case SDL_EVENT_MOUSE_MOTION: {
      if (!pointerActive_) break;
      float lx = (e.motion.x - offsetX_) / scale_;
      float ly = (e.motion.y - offsetY_) / scale_;
      if (inGameArea(lx, ly)) {
        setMethod(InputMethod::Mouse);
        setPointer(mapX(lx), mapY(ly));
      } else {
        pointerWorldX_ = -1;
        pointerWorldY_ = -1;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (!pointerActive_ || e.button.button != SDL_BUTTON_LEFT) break;
      float lx = (e.button.x - offsetX_) / scale_;
      float ly = (e.button.y - offsetY_) / scale_;
      if (inGameArea(lx, ly)) {
        setMethod(InputMethod::Mouse);
        activePointer_ = 1;
        setPointer(mapX(lx), mapY(ly));
        primaryQueued_ = true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (e.button.button != SDL_BUTTON_LEFT) break;
      activePointer_ = UINT64_MAX;
      break;
    }

    case SDL_EVENT_FINGER_MOTION: {
      if (!pointerActive_ || e.tfinger.fingerID != activePointer_) break;
      float lx = (e.tfinger.x * winW_ - offsetX_) / scale_;
      float ly = (e.tfinger.y * winH_ - offsetY_) / scale_;
      setMethod(InputMethod::Touch);
      setPointer(mapX(lx), mapY(ly));
      break;
    }
    case SDL_EVENT_FINGER_DOWN: {
      if (!pointerActive_) break;
      float lx = (e.tfinger.x * winW_ - offsetX_) / scale_;
      float ly = (e.tfinger.y * winH_ - offsetY_) / scale_;
      setMethod(InputMethod::Touch);
      activePointer_ = e.tfinger.fingerID;
      setPointer(mapX(lx), mapY(ly));
      primaryQueued_ = true;
      break;
    }
    case SDL_EVENT_FINGER_UP: {
      if (e.tfinger.fingerID == activePointer_) activePointer_ = UINT64_MAX;
      break;
    }
    default:
      break;
  }
}

InputActions InputManager::poll() {
  InputActions a;
  a.moveAxis = (keys_.contains(SDL_SCANCODE_RIGHT) || keys_.contains(SDL_SCANCODE_D) ? 1.0f : 0.0f) -
               (keys_.contains(SDL_SCANCODE_LEFT) || keys_.contains(SDL_SCANCODE_A) ? 1.0f : 0.0f);
  a.pointerX = (method_ == InputMethod::Keyboard) ? -1.0f : pointerWorldX_;
  a.pointerY = (method_ == InputMethod::Keyboard) ? -1.0f : pointerWorldY_;
  a.primary = primaryQueued_;
  a.pause = pauseQueued_;
  a.restart = restartQueued_;
  primaryQueued_ = pauseQueued_ = restartQueued_ = false;
  return a;
}

void InputManager::clear() {
  keys_.clear();
  activePointer_ = UINT64_MAX;
  pointerWorldX_ = -1;
  pointerWorldY_ = -1;
}

} // namespace neon
