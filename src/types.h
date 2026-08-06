#pragma once
#include <string>
#include <vector>

namespace neon {

// 与 web 版 types.ts 对应的类型定义
enum class GameState { Menu, Ready, Playing, Paused, LevelComplete, GameOver, Victory };
enum class InputMethod { Keyboard, Mouse, Touch, Pen };
enum class Language { ZH, EN };
enum class ThemeMode { System, Light, Dark };
enum class BrickKind { Normal, Tough, Indestructible, Bonus, Explosive, Rainbow, Respawn };
enum class PowerUpKind { Wide, Narrow, Multiball, Slow, Fast, Life, Pierce, Shield, Laser, Sticky, Mega };

const char* brickKindName(BrickKind kind);   // 供调试/日志
const char* powerUpKindName(PowerUpKind kind);

struct BallState { float x = 0, y = 0, vx = 0, vy = 0, radius = 0; bool piercing = false; };

struct PaddleState { float x = 0, y = 0, width = 0, height = 0; };

struct BrickState {
  int id = 0; float x = 0, y = 0, width = 0, height = 0;
  BrickKind kind = BrickKind::Normal; int hp = 0, maxHp = 0, points = 0;
  bool moving = false; float moveOrigin = 0, moveRange = 0, moveSpeed = 0, movePhase = 0;
  float hitFlash = 0, respawnTimer = 0;
};

struct PowerUpState { int id = 0; float x = 0, y = 0, vy = 0; PowerUpKind kind = PowerUpKind::Wide; float radius = 0; };

struct LaserState { float x = 0, y = 0; };

struct ParticleState { float x = 0, y = 0, vx = 0, vy = 0, life = 0, maxLife = 0; std::string color; float size = 0; };

struct ActiveEffect { PowerUpKind kind = PowerUpKind::Wide; float remaining = 0; };

struct LevelDefinition {
  std::string name;
  std::vector<std::string> pattern;
  float ballSpeed = 0;
  float paddleWidth = 0;
  std::vector<int> movingRows;
  float dropChance = 0;
};

struct GameSnapshot {
  GameState state = GameState::Menu;
  int score = 0, highScore = 0, lives = 0, level = 1, levelCount = 5;
  std::string levelName;
  int combo = 0; float multiplier = 1;
  bool soundEnabled = true;
  InputMethod inputMethod = InputMethod::Keyboard;
  std::vector<ActiveEffect> effects;
  Language language = Language::ZH;
  bool ballTrail = true;
  std::string ballColor = "#f5fbff";
  ThemeMode themeMode = ThemeMode::System;
  bool impactFx = true;
};

struct InputActions {
  float moveAxis = 0;
  float pointerX = -1; // <0 表示无指针输入
  float pointerY = -1;
  bool primary = false, pause = false, restart = false;
};

struct StorageData {
  int highScore = 0;
  bool soundEnabled = true;
  Language language = Language::ZH;
  bool ballTrail = true;
  std::string ballColor = "#f5fbff";
  ThemeMode themeMode = ThemeMode::System;
  bool impactFx = true;
};

} // namespace neon
