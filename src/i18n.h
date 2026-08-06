#pragma once
#include <map>
#include <string>
#include "types.h"

namespace neon {

// 与 web 版 i18n.ts 对应的双语字典
using Dict = std::map<std::string, const char*>;

inline const Dict ZH_DICT = {
  {"subtitle", "霓虹破界"}, {"settings", "设置"}, {"score", "分数"}, {"highScore", "最高分"},
  {"level", "关卡"}, {"lives", "生命"}, {"comboMultiplier", "连击倍率"}, {"combo", "连击"},
  {"sound", "音效"}, {"soundOn", "关闭音效"}, {"soundOff", "开启音效"}, {"pause", "暂停游戏"}, {"resume", "继续游戏"}, {"restart", "重新开始"},
  {"menuEyebrow", "街机协议已连接"}, {"menuTitle", "霓虹破界"}, {"menuText", "击碎核心矩阵，驾驭每一次反弹。"}, {"start", "开始游戏"},
  {"readyEyebrow", "系统就绪"}, {"readyTitle", "准备发球"}, {"readyText", "移动挡板选择位置，然后发射能量球。"}, {"launch", "发射"},
  {"pausedEyebrow", "游戏已暂停"}, {"pausedTitle", "暂停中"}, {"pausedText", "进度已冻结，准备好后继续。"},
  {"levelEyebrow", "区域净化完成"}, {"levelTitle", "关卡完成"}, {"levelText", "下一组核心矩阵已锁定。"}, {"nextLevel", "进入下一关"},
  {"gameOverEyebrow", "连接中断"}, {"gameOverTitle", "游戏结束"}, {"gameOverText", "核心能量耗尽，再试一次突破纪录。"},
  {"victoryEyebrow", "矩阵全部清除"}, {"victoryTitle", "全部通关"}, {"victoryText", "你已穿越所有霓虹防线。"}, {"again", "再次挑战"},
  {"settingsTitle", "游戏设置"}, {"settingsDesc", "个性化你的游戏体验。设置会自动保存在本机。"},
  {"language", "界面语言"}, {"chinese", "中文"}, {"english", "English"},
  {"theme", "界面主题"}, {"system", "跟随系统"}, {"light", "浅色"}, {"dark", "深色"},
  {"trail", "小球拖尾"}, {"trailDesc", "显示小球运动轨迹"},
  {"fx", "粒子特效"}, {"fxDesc", "砖块碎片、得分飘字与屏幕震动"},
  {"ballColor", "小球颜色"}, {"close", "完成"},
  {"keyboardHint", "A / D 或 ← / → 移动 · 空格发球 · Esc 暂停 · R 重开"},
  {"touchHint", "在游戏区域内拖动挡板 · 点击发球或继续"},
  {"livesLabel", "条生命"},
  {"levelSelect", "关卡选择"}, {"back", "返回"},
  {"power", ""},
};

inline const Dict EN_DICT = {
  {"subtitle", "Arcade protocol"}, {"settings", "Settings"}, {"score", "Score"}, {"highScore", "Best"},
  {"level", "Level"}, {"lives", "Lives"}, {"comboMultiplier", "Multiplier"}, {"combo", "Combo"},
  {"sound", "Sound"}, {"soundOn", "Mute sound"}, {"soundOff", "Enable sound"}, {"pause", "Pause"}, {"resume", "Resume"}, {"restart", "Restart"},
  {"menuEyebrow", "Arcade protocol online"}, {"menuTitle", "Neon Breaker"}, {"menuText", "Shatter the core matrix. Master every rebound."}, {"start", "Start game"},
  {"readyEyebrow", "System ready"}, {"readyTitle", "Ready to launch"}, {"readyText", "Position the paddle, then launch the energy ball."}, {"launch", "Launch"},
  {"pausedEyebrow", "Game paused"}, {"pausedTitle", "Paused"}, {"pausedText", "Progress is frozen. Resume when ready."},
  {"levelEyebrow", "Sector cleared"}, {"levelTitle", "Level complete"}, {"levelText", "The next core matrix is locked on."}, {"nextLevel", "Next level"},
  {"gameOverEyebrow", "Connection lost"}, {"gameOverTitle", "Game over"}, {"gameOverText", "Core energy depleted. Try again and beat your best."},
  {"victoryEyebrow", "All matrices cleared"}, {"victoryTitle", "Victory"}, {"victoryText", "You have broken through every neon defense."}, {"again", "Play again"},
  {"settingsTitle", "Game settings"}, {"settingsDesc", "Personalize your game. Changes are saved on this device."},
  {"language", "Language"}, {"chinese", "中文"}, {"english", "English"},
  {"theme", "Appearance"}, {"system", "System"}, {"light", "Light"}, {"dark", "Dark"},
  {"trail", "Ball trail"}, {"trailDesc", "Show the ball motion trail"},
  {"fx", "Particle FX"}, {"fxDesc", "Brick debris, floating text and screen shake"},
  {"ballColor", "Ball color"}, {"close", "Done"},
  {"keyboardHint", "A / D or \xe2\x86\x90 / \xe2\x86\x92 Move \xc2\xb7 Space Launch \xc2\xb7 Esc Pause \xc2\xb7 R Restart"},
  {"touchHint", "Drag in the game area \xc2\xb7 Tap to launch or resume"},
  {"livesLabel", "lives"},
  {"levelSelect", "Select Level"}, {"back", "Back"},
  {"power", ""},
};

inline const char* powerLabel(Language lang, PowerUpKind kind) {
  static const char* ZH[11] = { "加宽", "缩窄", "多球", "减速", "加速", "生命", "穿透", "护盾", "激光", "粘球", "巨球" };
  static const char* EN[11] = { "WIDE", "SHORT", "MULTI", "SLOW", "FAST", "LIFE", "PIERCE", "SHIELD", "LASER", "STICKY", "MEGA" };
  int i = static_cast<int>(kind);
  return (lang == Language::EN ? EN : ZH)[i];
}

inline const char* t(Language lang, const char* key) {
  const Dict& d = (lang == Language::EN) ? EN_DICT : ZH_DICT;
  auto it = d.find(key);
  return it != d.end() ? it->second : key;
}

} // namespace neon
