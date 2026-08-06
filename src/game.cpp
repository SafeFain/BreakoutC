#include "game.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "audio.h"
#include "constants.h"
#include "fx.h"
#include "i18n.h"
#include "input.h"
#include "levels.h"
#include "renderer.h"
#include "storage.h"

namespace neon {

static constexpr PowerUpKind DROP_TABLE[] = {
  PowerUpKind::Wide, PowerUpKind::Wide, PowerUpKind::Multiball, PowerUpKind::Slow,
  PowerUpKind::Life, PowerUpKind::Pierce, PowerUpKind::Shield, PowerUpKind::Fast,
  PowerUpKind::Narrow, PowerUpKind::Laser, PowerUpKind::Sticky, PowerUpKind::Mega,
};
static constexpr int DROP_TABLE_SIZE = static_cast<int>(sizeof(DROP_TABLE) / sizeof(DROP_TABLE[0]));

static bool isBreakable(const BrickState& b) { return b.kind != BrickKind::Indestructible; }

Game::Game(Renderer* renderer, InputManager* input, AudioManager* audio, FxSystem* fx,
           std::function<void(const GameSnapshot&)> onSnapshot)
    : renderer_(renderer), input_(input), audio_(audio), fx_(fx), onSnapshot_(std::move(onSnapshot)) {
  setupLevel();
  centerPaddle();
  attachBall();
}

void Game::applyStorage(const StorageData& data) {
  highScore_ = data.highScore;
  audio_->enabled = data.soundEnabled;
  language_ = data.language;
  ballTrail_ = data.ballTrail;
  ballColor_ = data.ballColor;
  themeMode_ = data.themeMode;
  fx_->enabled = data.impactFx;
  configureRenderer();
  emit();
}

void Game::step(float frame) {
  InputActions actions = input_->poll();
  handleActions(actions);
  accumulator_ += frame;
  while (accumulator_ >= FIXED_STEP) {
    update(FIXED_STEP, actions);
    accumulator_ -= FIXED_STEP;
  }
  emit();
}

void Game::toggleSound() {
  audio_->enabled = !audio_->enabled;
  if (audio_->enabled) audio_->unlock();
  persist();
  emit();
}

void Game::updatePreferences(Language language, bool ballTrail, const std::string& ballColor, ThemeMode themeMode, bool impactFx) {
  language_ = language;
  ballTrail_ = ballTrail;
  ballColor_ = ballColor;
  themeMode_ = themeMode;
  fx_->enabled = impactFx;
  configureRenderer();
  persist();
  emit();
}

void Game::refreshSystemTheme() { configureRenderer(); emit(); }

void Game::autoPause() {
  if (state_ == GameState::Playing || state_ == GameState::Ready) {
    prePauseState_ = state_;
    state_ = GameState::Paused;
    input_->clear();
    emit();
  }
}

void Game::handleActions(const InputActions& actions) {
  if (actions.restart) { newGame(); return; }
  if (actions.pause && (state_ == GameState::Ready || state_ == GameState::Playing || state_ == GameState::Paused)) {
    if (state_ == GameState::Paused) state_ = prePauseState_;
    else { prePauseState_ = state_; state_ = GameState::Paused; }
    audio_->unlock();
    return;
  }
  if (!actions.primary) return;
  audio_->unlock();
  if (state_ == GameState::Menu || state_ == GameState::GameOver || state_ == GameState::Victory) newGame();
  else if (state_ == GameState::Ready) state_ = GameState::Playing;
  else if (state_ == GameState::Paused) state_ = prePauseState_;
  else if (state_ == GameState::LevelComplete) nextLevel();
  else if (state_ == GameState::Playing && stuckBall_) launchStuckBall();
}

void Game::update(float dt, const InputActions& actions) {
  fx_->update(dt);
  if (actions.pointerX >= 0 && actions.pointerY >= 0) { aimX_ = actions.pointerX; aimY_ = actions.pointerY; }
  if (state_ == GameState::Paused || state_ == GameState::Menu || state_ == GameState::GameOver ||
      state_ == GameState::Victory || state_ == GameState::LevelComplete)
    return;
  if (actions.pointerX >= 0) paddle_.x = actions.pointerX - paddle_.width / 2;
  else paddle_.x += actions.moveAxis * PADDLE_SPEED * dt;
  paddle_.x = std::max(0.0f, std::min(WORLD_WIDTH - paddle_.width, paddle_.x));
  if (state_ == GameState::Ready) {
    if (!balls_.empty()) {
      balls_[0].x = paddle_.x + paddle_.width / 2;
      balls_[0].y = paddle_.y - BALL_RADIUS - 1;
    }
    return;
  }
  updateEffects(dt);
  updateBricks(dt);
  updatePowerUps(dt);
  if (hasEffect(PowerUpKind::Laser)) {
    laserCooldown_ -= dt;
    if (laserCooldown_ <= 0) fireLasers();
    updateLasers(dt);
  }
  if (combo_ > 0) { comboTimer_ -= dt; if (comboTimer_ <= 0) combo_ = 0; }
  for (int i = static_cast<int>(balls_.size()) - 1; i >= 0; i--) {
    BallState& ball = balls_[static_cast<size_t>(i)];
    moveBall(ball, dt);
    if (ball.y - ball.radius > WORLD_HEIGHT + 20) balls_.erase(balls_.begin() + i);
  }
  if (balls_.empty()) loseLife();
  if (state_ == GameState::Playing && !std::any_of(bricks_.begin(), bricks_.end(), isBreakable)) completeLevel();
}

void Game::moveBall(BallState& ball, float dt) {
  if (&ball == stuckBall_) {
    ball.x = paddle_.x + paddle_.width / 2;
    ball.y = paddle_.y - ball.radius - 1;
    return;
  }
  ball.x += ball.vx * dt;
  ball.y += ball.vy * dt;
  if (ball.x - ball.radius < 0) { ball.x = ball.radius; ball.vx = std::abs(ball.vx); }
  if (ball.x + ball.radius > WORLD_WIDTH) { ball.x = WORLD_WIDTH - ball.radius; ball.vx = -std::abs(ball.vx); }
  if (ball.y - ball.radius < 0) { ball.y = ball.radius; ball.vy = std::abs(ball.vy); }
  if (ball.y + ball.radius >= paddle_.y && ball.y - ball.radius <= paddle_.y + paddle_.height && ball.vy > 0 &&
      ball.x >= paddle_.x - ball.radius && ball.x <= paddle_.x + paddle_.width + ball.radius) {
    ball.y = paddle_.y - ball.radius;
    if (hasEffect(PowerUpKind::Sticky) && &ball == &balls_[0]) {
      stuckBall_ = &ball;
      ball.vx = 0; ball.vy = 0;
      sound(SoundName::Sticky);
      return;
    }
    float offset = std::max(-1.0f, std::min(1.0f, (ball.x - (paddle_.x + paddle_.width / 2)) / (paddle_.width / 2)));
    float speed = std::hypot(ball.vx, ball.vy);
    float angle = offset * 1.05f;
    ball.vx = speed * std::sin(angle);
    ball.vy = -std::abs(speed * std::cos(angle));
    sound(SoundName::Paddle);
  }
  if (shield_ && ball.y + ball.radius >= WORLD_HEIGHT - 17 && ball.vy > 0) {
    ball.y = WORLD_HEIGHT - 17 - ball.radius;
    ball.vy = -std::abs(ball.vy);
    shield_ = false;
    removeEffect(PowerUpKind::Shield);
    fx_->burst(ball.x, ball.y, "#45f5d2", 14);
    fx_->ring(ball.x, ball.y, "#45f5d2");
  }
  for (BrickState& brick : bricks_) {
    if (!intersects(ball, brick)) continue;
    float hitX = std::min(std::abs(ball.x - brick.x), std::abs(ball.x - (brick.x + brick.width)));
    float hitY = std::min(std::abs(ball.y - brick.y), std::abs(ball.y - (brick.y + brick.height)));
    if (!ball.piercing || brick.kind == BrickKind::Indestructible) {
      if (hitX < hitY) ball.vx *= -1; else ball.vy *= -1;
    }
    hitBrick(brick);
    break;
  }
}

bool Game::intersects(const BallState& ball, const BrickState& brick) const {
  float x = std::max(brick.x, std::min(ball.x, brick.x + brick.width));
  float y = std::max(brick.y, std::min(ball.y, brick.y + brick.height));
  float dx = ball.x - x, dy = ball.y - y;
  return dx * dx + dy * dy <= ball.radius * ball.radius;
}

void Game::hitBrick(BrickState& brick) {
  brick.hitFlash = 0.09f;
  if (brick.kind == BrickKind::Indestructible) { sound(SoundName::Brick); return; }
  brick.hp--;
  combo_++;
  comboTimer_ = COMBO_TIMEOUT;
  float multiplier = this->multiplier();
  score_ += static_cast<int>(std::lround(brick.points * multiplier * (brick.hp <= 0 ? 1.0f : 0.25f)));
  updateHighScore();
  if (brick.hp > 0) {
    sound(brick.kind == BrickKind::Rainbow ? SoundName::Rainbow : SoundName::Brick);
    fx_->burst(brick.x + brick.width / 2, brick.y + brick.height / 2, "#ffffff", 4);
    return;
  }
  // 击碎:先拷贝完整数据(删除后引用失效)
  BrickState saved = brick;
  for (size_t i = 0; i < bricks_.size(); i++) {
    if (bricks_[i].id == brick.id) { bricks_.erase(bricks_.begin() + static_cast<ptrdiff_t>(i)); break; }
  }
  sound(SoundName::Break);
  float cx = saved.x + saved.width / 2, cy = saved.y + saved.height / 2;
  const char* mainColor = saved.kind == BrickKind::Bonus ? "#ffd166" : saved.kind == BrickKind::Respawn ? "#45f5d2" : "#48e5ff";
  if (saved.kind == BrickKind::Rainbow) {
    for (const char* color : RAINBOW_HP_COLORS) fx_->burst(cx, cy, color, 6);
  } else {
    fx_->burst(cx, cy, mainColor, saved.kind == BrickKind::Explosive ? 22 : 10);
  }
  fx_->spawnDebris(cx, cy, mainColor, saved.kind == BrickKind::Explosive ? 12 : 8);
  int pts = static_cast<int>(std::lround(saved.points * multiplier));
  fx_->addText(cx, cy - 8, "+" + std::to_string(pts), saved.kind == BrickKind::Bonus ? "#ffd166" : "#ffffff", 15);
  if (saved.kind == BrickKind::Explosive) {
    fx_->addShake(7, 0.28f);
    fx_->flash("rgba(255,93,143,0.16)");
  }
  if (combo_ % 5 == 0) {
    sound(SoundName::Combo);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "x%.1f", this->multiplier());
    fx_->burst(cx, cy, "#ffd166", 16);
    fx_->addText(cx, cy - 26, buf, "#ffd166", 18);
  }
  if (saved.kind == BrickKind::Explosive) explode(saved);
  if (saved.kind == BrickKind::Respawn) {
    saved.respawnTimer = RESPAWN_TIME;
    saved.hitFlash = 0;
    respawnQueue_.push_back(saved);
  }
  if (saved.kind == BrickKind::Bonus || (std::rand() % 10000) / 10000.0f < LEVELS[static_cast<size_t>(levelIndex_)].dropChance) {
    dropPowerUp(saved);
  }
}

void Game::explode(const BrickState& origin) {
  std::vector<int> ids;
  for (const BrickState& brick : bricks_) {
    if (!isBreakable(brick)) continue;
    float dx = brick.x - origin.x, dy = brick.y - origin.y;
    if (std::hypot(dx, dy) < 125) ids.push_back(brick.id);
  }
  // 逐个按 id 重查,规避 vector erase 导致的引用失效;连锁爆炸会递归进入本函数
  for (int id : ids) {
    for (BrickState& brick : bricks_) {
      if (brick.id == id) { brick.hp = 1; hitBrick(brick); break; }
    }
  }
}

void Game::updatePowerUps(float dt) {
  for (int i = static_cast<int>(powerUps_.size()) - 1; i >= 0; i--) {
    PowerUpState& item = powerUps_[static_cast<size_t>(i)];
    item.y += item.vy * dt;
    if (item.y + item.radius >= paddle_.y && item.y - item.radius <= paddle_.y + paddle_.height &&
        item.x >= paddle_.x && item.x <= paddle_.x + paddle_.width) {
      applyPowerUp(item.kind, item.x, item.y);
      powerUps_.erase(powerUps_.begin() + i);
    } else if (item.y > WORLD_HEIGHT + 20) {
      powerUps_.erase(powerUps_.begin() + i);
    }
  }
}

void Game::applyPowerUp(PowerUpKind kind, float x, float y) {
  sound(SoundName::Power);
  score_ += static_cast<int>(100 * multiplier());
  updateHighScore();
  fx_->ring(x, y, "#65dbc8");
  fx_->addText(x, y - 14, powerLabel(language_, kind), "#65dbc8", 14);
  if (kind == PowerUpKind::Life) { lives_ = std::min(MAX_LIVES, lives_ + 1); return; }
  if (kind == PowerUpKind::Multiball) {
    if (!balls_.empty()) {
      for (float sign : { -1.0f, 1.0f }) {
        if (static_cast<int>(balls_.size()) < MAX_BALLS) {
          const BallState& src = balls_[0];
          BallState nb = src;
          nb.vx = sign * std::max(160.0f, std::abs(src.vx));
          nb.vy = -std::abs(src.vy);
          balls_.push_back(nb);
        }
      }
    }
    return;
  }
  if (kind == PowerUpKind::Shield) shield_ = true;
  for (size_t i = 0; i < effects_.size();) {
    const ActiveEffect& e = effects_[i];
    bool conflict = e.kind == kind ||
                    ((kind == PowerUpKind::Wide || kind == PowerUpKind::Narrow) && (e.kind == PowerUpKind::Wide || e.kind == PowerUpKind::Narrow)) ||
                    ((kind == PowerUpKind::Slow || kind == PowerUpKind::Fast) && (e.kind == PowerUpKind::Slow || e.kind == PowerUpKind::Fast));
    if (conflict) effects_.erase(effects_.begin() + static_cast<ptrdiff_t>(i));
    else i++;
  }
  effects_.push_back({ kind, EFFECT_DURATION });
  recalculateEffects();
  if (kind == PowerUpKind::Laser) laserCooldown_ = 0;
}

void Game::fireLasers() {
  laserCooldown_ = LASER_COOLDOWN;
  float left = paddle_.x + 8, right = paddle_.x + paddle_.width - 8;
  lasers_.push_back({ left, paddle_.y });
  lasers_.push_back({ right, paddle_.y });
  sound(SoundName::Laser);
}

void Game::updateLasers(float dt) {
  for (int i = static_cast<int>(lasers_.size()) - 1; i >= 0; i--) {
    LaserState& laser = lasers_[static_cast<size_t>(i)];
    laser.y -= LASER_SPEED * dt;
    if (laser.y < 10) { lasers_.erase(lasers_.begin() + i); continue; }
    bool hit = false;
    for (BrickState& brick : bricks_) {
      if (laser.x >= brick.x && laser.x <= brick.x + brick.width && laser.y >= brick.y && laser.y <= brick.y + brick.height) {
        if (brick.kind != BrickKind::Indestructible) {
          hitBrick(brick);
          fx_->burst(laser.x, laser.y, "#ff5d8f", 5);
        }
        hit = true;
        break;
      }
    }
    if (hit) lasers_.erase(lasers_.begin() + i);
  }
}

void Game::launchStuckBall() {
  BallState* ball = stuckBall_;
  stuckBall_ = nullptr;
  if (!ball) return;
  float speed = LEVELS[static_cast<size_t>(levelIndex_)].ballSpeed *
                (hasEffect(PowerUpKind::Slow) ? 0.78f : hasEffect(PowerUpKind::Fast) ? 1.25f : 1.0f);
  float dx = 0.55f, dy = -0.835f; // 默认方向(无指针/与球过近时)
  if (aimX_ >= 0 && aimY_ >= 0) {
    computeLaunchDirection(dx, dy, ball->x, ball->y, aimX_, aimY_);
  }
  ball->x = paddle_.x + paddle_.width / 2;
  ball->y = paddle_.y - ball->radius - 1;
  ball->vx = dx * speed;
  ball->vy = dy * speed;
  sound(SoundName::Paddle);
}

void Game::computeLaunchDirection(float& dx, float& dy, float ballX, float ballY, float aimX, float aimY) {
  dx = 0.55f;
  dy = -0.835f;
  float vx = aimX - ballX, vy = aimY - ballY;
  float len = std::hypot(vx, vy);
  if (len <= 4) return; // 鼠标与球几乎重合:回退默认方向
  vx /= len;
  vy /= len;
  if (vy > -0.15f) { // 水平或向下:强制最小仰角,避免击穿挡板
    vy = -0.15f;
    float l = std::hypot(vx, vy);
    vx /= l;
    vy /= l;
  }
  dx = vx;
  dy = vy;
}

void Game::updateEffects(float dt) {
  for (ActiveEffect& e : effects_) e.remaining -= dt;
  bool expired = false;
  for (const ActiveEffect& e : effects_) if (e.remaining <= 0) { expired = true; break; }
  if (expired) {
    for (size_t i = 0; i < effects_.size();) {
      if (effects_[i].remaining <= 0) effects_.erase(effects_.begin() + static_cast<ptrdiff_t>(i));
      else i++;
    }
    recalculateEffects();
  }
}

void Game::recalculateEffects() {
  const LevelDefinition& base = LEVELS[static_cast<size_t>(levelIndex_)];
  paddle_.width = base.paddleWidth * (hasEffect(PowerUpKind::Wide) ? 1.45f : hasEffect(PowerUpKind::Narrow) ? 0.72f : 1.0f);
  paddle_.x = std::min(paddle_.x, WORLD_WIDTH - paddle_.width);
  float speedFactor = hasEffect(PowerUpKind::Slow) ? 0.78f : hasEffect(PowerUpKind::Fast) ? 1.25f : 1.0f;
  float radius = BALL_RADIUS * (hasEffect(PowerUpKind::Mega) ? MEGA_BALL_SCALE : 1.0f);
  for (BallState& ball : balls_) {
    if (&ball == stuckBall_) { ball.radius = radius; continue; }
    float current = std::max(1.0f, std::hypot(ball.vx, ball.vy));
    float target = base.ballSpeed * speedFactor;
    ball.vx *= target / current;
    ball.vy *= target / current;
    ball.piercing = hasEffect(PowerUpKind::Pierce);
    ball.radius = radius;
  }
}

void Game::setupLevel() {
  const LevelDefinition& level = LEVELS[static_cast<size_t>(levelIndex_)];
  int cols = 0;
  for (const std::string& row : level.pattern) cols = std::max(cols, static_cast<int>(row.size()));
  const float gap = 8;
  float width = std::min(82.0f, (WORLD_WIDTH - 90 - gap * (cols - 1)) / cols);
  const float height = 27;
  float total = cols * width + (cols - 1) * gap;
  float left = (WORLD_WIDTH - total) / 2;
  bricks_.clear();
  for (size_t rowIndex = 0; rowIndex < level.pattern.size(); rowIndex++) {
    const std::string& row = level.pattern[rowIndex];
    for (size_t col = 0; col < row.size(); col++) {
      char code = row[col];
      if (code == '.') continue;
      BrickKind kind = kindFor(code);
      int hp = (kind == BrickKind::Tough || kind == BrickKind::Rainbow) ? 3 : 1;
      BrickState b;
      b.id = nextId_++;
      b.x = left + static_cast<float>(col) * (width + gap);
      b.y = 72 + static_cast<float>(rowIndex) * (height + gap);
      b.width = width;
      b.height = height;
      b.kind = kind;
      b.hp = hp;
      b.maxHp = hp;
      b.points = kind == BrickKind::Tough ? 180 : kind == BrickKind::Bonus ? 250 : kind == BrickKind::Explosive ? 160 :
                 kind == BrickKind::Rainbow ? 150 : kind == BrickKind::Respawn ? 120 : kind == BrickKind::Indestructible ? 0 : 100;
      b.moving = std::find(level.movingRows.begin(), level.movingRows.end(), static_cast<int>(rowIndex)) != level.movingRows.end();
      b.moveOrigin = b.x;
      b.moveRange = 18;
      b.moveSpeed = 0.9f + static_cast<float>(rowIndex) * 0.08f;
      b.movePhase = static_cast<float>(col) * 0.55f;
      bricks_.push_back(b);
    }
  }
  paddle_.width = level.paddleWidth;
  effects_.clear();
  powerUps_.clear();
  shield_ = false;
  respawnQueue_.clear();
  lasers_.clear();
  stuckBall_ = nullptr;
  laserCooldown_ = 0;
}

BrickKind Game::kindFor(char code) const {
  switch (code) {
    case 'N': return BrickKind::Normal;
    case 'T': return BrickKind::Tough;
    case 'I': return BrickKind::Indestructible;
    case 'B': return BrickKind::Bonus;
    case 'E': case 'X': return BrickKind::Explosive;
    case 'R': return BrickKind::Rainbow;
    case 'G': return BrickKind::Respawn;
    default: return BrickKind::Normal;
  }
}

void Game::updateBricks(float dt) {
  for (BrickState& brick : bricks_) {
    brick.hitFlash = std::max(0.0f, brick.hitFlash - dt);
    if (brick.moving) {
      brick.movePhase += brick.moveSpeed * dt;
      brick.x = brick.moveOrigin + std::sin(brick.movePhase) * brick.moveRange;
    }
  }
  for (int i = static_cast<int>(respawnQueue_.size()) - 1; i >= 0; i--) {
    BrickState& brick = respawnQueue_[static_cast<size_t>(i)];
    brick.respawnTimer -= dt;
    if (brick.respawnTimer <= 0) {
      brick.respawnTimer = 0;
      brick.hp = brick.maxHp;
      bricks_.push_back(brick);
      respawnQueue_.erase(respawnQueue_.begin() + i);
      sound(SoundName::Respawn);
      fx_->burst(brick.x + brick.width / 2, brick.y + brick.height / 2, BRICK_COLOR_RESPAWN, 8);
      fx_->ring(brick.x + brick.width / 2, brick.y + brick.height / 2, "#45f5d2");
    }
  }
}

void Game::attachBall() {
  float speed = LEVELS[static_cast<size_t>(levelIndex_)].ballSpeed;
  balls_.clear();
  balls_.push_back({ paddle_.x + paddle_.width / 2, paddle_.y - BALL_RADIUS - 1, speed * 0.55f, -speed * 0.835f, BALL_RADIUS, false });
  stuckBall_ = nullptr;
}

void Game::centerPaddle() { paddle_.x = (WORLD_WIDTH - paddle_.width) / 2; }

void Game::newGame() {
  fx_->clear();
  score_ = 0;
  lives_ = INITIAL_LIVES;
  levelIndex_ = 0;
  combo_ = 0;
  setupLevel();
  centerPaddle();
  attachBall();
  state_ = GameState::Ready;
  audio_->unlock();
}

void Game::selectLevel(int index) {
  if (index < 0 || index >= static_cast<int>(LEVELS.size())) return;
  fx_->clear();
  score_ = 0;
  lives_ = INITIAL_LIVES;
  combo_ = 0;
  levelIndex_ = index;
  setupLevel();
  centerPaddle();
  attachBall();
  state_ = GameState::Ready;
  audio_->unlock();
  emit();
}

void Game::nextLevel() {
  levelIndex_++;
  combo_ = 0;
  setupLevel();
  centerPaddle();
  attachBall();
  state_ = GameState::Ready;
}

void Game::completeLevel() {
  state_ = levelIndex_ >= static_cast<int>(LEVELS.size()) - 1 ? GameState::Victory : GameState::LevelComplete;
  score_ += 1000 * (levelIndex_ + 1) + lives_ * 250;
  updateHighScore();
  sound(SoundName::Level);
}

void Game::loseLife() {
  fx_->clear();
  fx_->addShake(11, 0.4f);
  fx_->flash("rgba(255,45,85,0.22)", 0.3f);
  combo_ = 0;
  lives_--;
  effects_.clear();
  shield_ = false;
  lasers_.clear();
  stuckBall_ = nullptr;
  sound(SoundName::Lost);
  if (lives_ <= 0) {
    state_ = GameState::GameOver;
    sound(SoundName::GameOver);
    persist();
  } else {
    paddle_.width = LEVELS[static_cast<size_t>(levelIndex_)].paddleWidth;
    centerPaddle();
    attachBall();
    state_ = GameState::Ready;
  }
}

void Game::dropPowerUp(const BrickState& brick) {
  PowerUpKind kind = DROP_TABLE[std::rand() % DROP_TABLE_SIZE];
  powerUps_.push_back({ nextId_++, brick.x + brick.width / 2, brick.y + brick.height / 2, POWERUP_SPEED, kind, 13 });
}

void Game::sound(SoundName name) { audio_->play(name); }

void Game::updateHighScore() {
  if (score_ > highScore_) { highScore_ = score_; persist(); }
}

void Game::configureRenderer() {
  renderer_->configure(ballColor_, ballTrail_, language_, themeMode_);
}

void Game::persist() {
  StorageData d;
  d.highScore = highScore_;
  d.soundEnabled = audio_->enabled;
  d.language = language_;
  d.ballTrail = ballTrail_;
  d.ballColor = ballColor_;
  d.themeMode = themeMode_;
  d.impactFx = fx_->enabled;
  saveStorage(d);
}

void Game::emit() {
  GameSnapshot s;
  s.state = state_;
  s.score = score_;
  s.highScore = highScore_;
  s.lives = lives_;
  s.level = levelIndex_ + 1;
  s.levelCount = static_cast<int>(LEVELS.size());
  s.levelName = LEVELS[static_cast<size_t>(levelIndex_)].name;
  s.combo = combo_;
  s.multiplier = multiplier();
  s.soundEnabled = audio_->enabled;
  s.inputMethod = input_->inputMethod();
  s.effects = effects_;
  s.language = language_;
  s.ballTrail = ballTrail_;
  s.ballColor = ballColor_;
  s.themeMode = themeMode_;
  s.impactFx = fx_->enabled;
  onSnapshot_(s);
}

} // namespace neon
