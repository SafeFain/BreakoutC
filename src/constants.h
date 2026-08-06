#pragma once
namespace neon {

constexpr float WORLD_WIDTH = 960;
constexpr float WORLD_HEIGHT = 600;
constexpr float FIXED_STEP = 1.0f / 120.0f;
constexpr float MAX_FRAME_TIME = 0.1f;
constexpr int INITIAL_LIVES = 3;
constexpr int MAX_LIVES = 6;
constexpr float BALL_RADIUS = 7;
constexpr float PADDLE_Y = 552;
constexpr float PADDLE_HEIGHT = 16;
constexpr float PADDLE_SPEED = 650;
constexpr float POWERUP_SPEED = 145;
constexpr float COMBO_TIMEOUT = 3.5f;
constexpr float EFFECT_DURATION = 12;
constexpr int MAX_BALLS = 5;
constexpr float RESPAWN_TIME = 5;
constexpr float LASER_COOLDOWN = 2.2f;
constexpr float LASER_SPEED = 520;
constexpr float MEGA_BALL_SCALE = 1.5f;

// 砖块颜色(与 web 版一致)
inline const char* BRICK_COLOR_NORMAL = "#42d9ff";
inline const char* BRICK_COLOR_TOUGH = "#aa7dff";
inline const char* BRICK_COLOR_INDESTRUCTIBLE = "#64748b";
inline const char* BRICK_COLOR_BONUS = "#ffd166";
inline const char* BRICK_COLOR_EXPLOSIVE = "#ff5d8f";
inline const char* BRICK_COLOR_RAINBOW = "#42d9ff";
inline const char* BRICK_COLOR_RESPAWN = "#45f5d2";

// 彩虹砖按 hp 变色:hp=3 青 → hp=2 紫 → hp=1 粉
inline const char* RAINBOW_HP_COLORS[3] = { "#42d9ff", "#aa7dff", "#ff5d8f" };

inline const char* POWERUP_LABELS[11] = {
  "加宽", "缩窄", "多球", "减速", "加速", "生命", "穿透", "护盾", "激光", "粘球", "巨球"
};

} // namespace neon
