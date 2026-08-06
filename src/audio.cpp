#include "audio.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <cstring>

namespace neon {

// 波形:0=sine 1=square 2=triangle 3=sawtooth
static constexpr float TWO_PI = 6.283185307179586f;

AudioManager::AudioManager(bool enabled) : enabled(enabled) { synthAll(); }
AudioManager::~AudioManager() { shutdown(); }

void AudioManager::shutdown() {
  if (stream_) {
    SDL_DestroyAudioStream(static_cast<SDL_AudioStream*>(stream_));
    stream_ = nullptr;
    opened_ = false;
  }
}

void AudioManager::synthAll() {
  struct Spec { SoundName name; float freq; float dur; int wave; float vol; };
  static const Spec specs[] = {
    { SoundName::Paddle, 220, 0.045f, 0, 0.05f },
    { SoundName::Brick, 390, 0.035f, 1, 0.035f },
    { SoundName::Break, 560, 0.07f, 2, 0.045f },
    { SoundName::Power, 720, 0.16f, 0, 0.06f },
    { SoundName::Lost, 150, 0.25f, 3, 0.045f },
    { SoundName::Level, 620, 0.35f, 2, 0.06f },
    { SoundName::GameOver, 95, 0.45f, 3, 0.05f },
    { SoundName::Laser, 980, 0.08f, 3, 0.04f },
    { SoundName::Sticky, 300, 0.1f, 0, 0.05f },
    { SoundName::Rainbow, 520, 0.05f, 2, 0.04f },
    { SoundName::Respawn, 330, 0.16f, 0, 0.045f },
    { SoundName::Combo, 660, 0.1f, 1, 0.035f },
  };
  pcm_.resize(12);
  for (const Spec& s : specs) pcm_[static_cast<int>(s.name)] = synth(s.freq, s.dur, s.wave, s.vol);
}

std::vector<int16_t> AudioManager::synth(float freq, float duration, int wave, float volume) {
  constexpr int RATE = 44100;
  const int n = static_cast<int>(duration * RATE);
  std::vector<int16_t> out(static_cast<size_t>(n));
  float phase = 0;
  const float decay = std::log(0.001f / volume) / duration; // 指数衰减到 0.001,与 Web 版 gain ramp 对应
  for (int i = 0; i < n; i++) {
    float t = static_cast<float>(i) / RATE;
    float ft = freq * std::pow(0.72f, t / duration); // exponentialRampToValueAtTime(f*0.72)
    phase += TWO_PI * ft / RATE;
    if (phase > TWO_PI) phase -= TWO_PI;
    float v;
    switch (wave) {
      case 1: v = phase < 3.14159265f ? 1.0f : -1.0f; break;
      case 2: v = phase < 3.14159265f ? phase / 3.14159265f * 2 - 1 : 1 - (phase - 3.14159265f) / 3.14159265f * 2; break;
      case 3: v = 2 * (phase / TWO_PI - std::floor(0.5f + phase / TWO_PI)); break;
      default: v = std::sin(phase); break;
    }
    float amp = volume * std::exp(decay * t);
    out[static_cast<size_t>(i)] = static_cast<int16_t>(v * amp * 32767.0f);
  }
  return out;
}

void AudioManager::unlock() {
  if (!enabled) return;
  if (opened_) return;
  SDL_AudioSpec spec;
  SDL_zero(spec);
  spec.format = SDL_AUDIO_S16;
  spec.channels = 1;
  spec.freq = 44100;
  SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
  if (!stream) { stream_ = nullptr; return; }
  stream_ = stream;
  opened_ = true;
  SDL_ResumeAudioStreamDevice(stream);
}

void AudioManager::play(SoundName name) {
  if (!enabled || !stream_) return;
  const std::vector<int16_t>& pcm = pcm_[static_cast<int>(name)];
  if (pcm.empty()) return;
  SDL_PutAudioStreamData(static_cast<SDL_AudioStream*>(stream_), pcm.data(), static_cast<int>(pcm.size() * sizeof(int16_t)));
}

} // namespace neon
