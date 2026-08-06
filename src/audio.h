#pragma once
#include <cstdint>
#include <vector>

namespace neon {

enum class SoundName {
  Paddle, Brick, Break, Power, Lost, Level, GameOver, Laser, Sticky, Rainbow, Respawn, Combo
};

// 与 web 版 audio.ts 对应:Web Audio 振荡器合成 → SDL3 音频流 PCM 合成
class AudioManager {
public:
  explicit AudioManager(bool enabled);
  ~AudioManager();

  bool enabled = true;
  void unlock();              // 打开音频设备(等价于 AudioContext.resume)
  void shutdown();            // 在 SDL_Quit 之前显式关闭音频流
  void play(SoundName name);  // 合成音效并推入播放队列

private:
  void* stream_ = nullptr;                       // SDL_AudioStream*
  bool opened_ = false;
  std::vector<std::vector<int16_t>> pcm_;        // 预合成音效(44100Hz 单声道 S16)
  void synthAll();
  static std::vector<int16_t> synth(float freq, float duration, int wave, float volume);
};

} // namespace neon
