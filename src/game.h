#pragma once
#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include "audio.h"
#include "constants.h"
#include "input.h"
#include "types.h"

namespace neon {

class AudioManager;
class FxSystem;
class InputManager;
class Renderer;

// 与 web 版 game.ts 对应:状态机 + 物理 + 计分 + 道具,不接触任何渲染/输入细节
class Game {
public:
  Game(Renderer* renderer, InputManager* input, AudioManager* audio, FxSystem* fx,
       std::function<void(const GameSnapshot&)> onSnapshot);

  void applyStorage(const StorageData& data);
  void step(float frameSeconds); // 每帧推进(内部含固定步长累加器)
  void toggleSound();
  void updatePreferences(Language language, bool ballTrail, const std::string& ballColor, ThemeMode themeMode, bool impactFx);
  void refreshSystemTheme();
  void primaryAction() { input_->queuePrimary(); }
  void pauseAction() { input_->queuePause(); }
  void restartAction() { input_->queueRestart(); }
  void autoPause();
  void selectLevel(int index);

  // 渲染数据访问
  GameState state() const { return state_; }
  const std::vector<BallState>& balls() const { return balls_; }
  const PaddleState& paddle() const { return paddle_; }
  const std::vector<BrickState>& bricks() const { return bricks_; }
  const std::vector<BrickState>& respawnQueue() const { return respawnQueue_; }
  const std::vector<PowerUpState>& powerUps() const { return powerUps_; }
  const std::vector<LaserState>& lasers() const { return lasers_; }
  bool shield() const { return shield_; }
  const std::vector<ActiveEffect>& effects() const { return effects_; }
  int combo() const { return combo_; }
  bool stuckBall() const { return stuckBall_ != nullptr; }
  float aimX() const { return aimX_; }
  float aimY() const { return aimY_; }

  // 粘球发射方向:球→鼠标连线(归一化);鼠标无效/过近时回退默认方向;接近水平或向下时施加最小仰角
  static void computeLaunchDirection(float& dx, float& dy, float ballX, float ballY, float aimX, float aimY);

private:
  Renderer* renderer_;
  InputManager* input_;
  AudioManager* audio_;
  FxSystem* fx_;
  std::function<void(const GameSnapshot&)> onSnapshot_;

  GameState state_ = GameState::Menu;
  int score_ = 0, highScore_ = 0, lives_ = INITIAL_LIVES;
  GameState prePauseState_ = GameState::Ready;
  int levelIndex_ = 0, combo_ = 0;
  float comboTimer_ = 0, accumulator_ = 0;
  std::vector<BallState> balls_;
  std::vector<BrickState> bricks_;
  std::vector<PowerUpState> powerUps_;
  std::vector<ActiveEffect> effects_;
  std::vector<BrickState> respawnQueue_;
  std::vector<LaserState> lasers_;
  bool shield_ = false;
  int nextId_ = 1;
  float laserCooldown_ = 0;
  BallState* stuckBall_ = nullptr;
  float aimX_ = -1, aimY_ = -1; // 最后有效指针位置(粘球瞄准方向)
  PaddleState paddle_{ 0, PADDLE_Y, 120, PADDLE_HEIGHT };
  Language language_ = Language::ZH;
  bool ballTrail_ = true;
  std::string ballColor_ = "#f5fbff";
  ThemeMode themeMode_ = ThemeMode::System;

  void handleActions(const InputActions& actions);
  void update(float dt, const InputActions& actions);
  void moveBall(BallState& ball, float dt);
  bool intersects(const BallState& ball, const BrickState& brick) const;
  void hitBrick(BrickState& brick);
  void explode(const BrickState& origin);
  void updatePowerUps(float dt);
  void applyPowerUp(PowerUpKind kind, float x, float y);
  void fireLasers();
  void updateLasers(float dt);
  void launchStuckBall();
  void updateEffects(float dt);
  void recalculateEffects();
  bool hasEffect(PowerUpKind kind) const { for (const auto& e : effects_) if (e.kind == kind) return true; return false; }
  void removeEffect(PowerUpKind kind) {
    for (size_t i = 0; i < effects_.size(); i++) if (effects_[i].kind == kind) { effects_.erase(effects_.begin() + static_cast<ptrdiff_t>(i)); break; }
  }
  void setupLevel();
  BrickKind kindFor(char code) const;
  void updateBricks(float dt);
  void attachBall();
  void centerPaddle();
  void newGame();
  void nextLevel();
  void completeLevel();
  void loseLife();
  void dropPowerUp(const BrickState& brick);
  float multiplier() const { return std::min(5.0f, 1 + std::floor(combo_ / 5.0f) * 0.5f); }
  void sound(SoundName name);
  void updateHighScore();
  void configureRenderer();
  void persist();
  void emit();
};

} // namespace neon
