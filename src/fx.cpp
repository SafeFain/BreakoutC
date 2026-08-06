#include "fx.h"

#include <cstdlib>
#include <cmath>

namespace neon {

static float randf() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }
static float randRange(float a, float b) { return a + (b - a) * randf(); }

void FxSystem::update(float dt) {
  if (shakeTime_ > 0) shakeTime_ -= dt;
  if (!enabled) { particles.clear(); texts_.clear(); debris_.clear(); rings_.clear(); flashes_.clear(); shakeTime_ = 0; return; }
  for (int i = static_cast<int>(particles.size()) - 1; i >= 0; i--) {
    ParticleState& p = particles[static_cast<size_t>(i)];
    p.life -= dt; p.x += p.vx * dt; p.y += p.vy * dt; p.vy += 80 * dt;
    if (p.life <= 0) particles.erase(particles.begin() + i);
  }
  for (int i = static_cast<int>(texts_.size()) - 1; i >= 0; i--) {
    FloatText& t = texts_[static_cast<size_t>(i)];
    t.life -= dt; t.y += t.vy * dt;
    if (t.life <= 0) texts_.erase(texts_.begin() + i);
  }
  for (int i = static_cast<int>(debris_.size()) - 1; i >= 0; i--) {
    Debris& d = debris_[static_cast<size_t>(i)];
    d.life -= dt; d.x += d.vx * dt; d.y += d.vy * dt; d.vy += 460 * dt; d.rotation += d.vrot * dt;
    if (d.life <= 0) debris_.erase(debris_.begin() + i);
  }
  for (int i = static_cast<int>(rings_.size()) - 1; i >= 0; i--) {
    Ring& r = rings_[static_cast<size_t>(i)];
    r.life -= dt; r.radius += r.vradius * dt;
    if (r.life <= 0) rings_.erase(rings_.begin() + i);
  }
  for (int i = static_cast<int>(flashes_.size()) - 1; i >= 0; i--) {
    if (flashes_[static_cast<size_t>(i)].life <= 0) flashes_.erase(flashes_.begin() + i);
    else flashes_[static_cast<size_t>(i)].life -= dt;
  }
}

void FxSystem::clear() {
  particles.clear(); texts_.clear(); debris_.clear(); rings_.clear(); flashes_.clear();
  shakeTime_ = 0;
}

float FxSystem::shakeOffset(float sign) const {
  if (shakeTime_ <= 0) return 0;
  float intensity = (shakeTime_ / shakeDuration_) * shakeMagnitude_;
  return (randf() * 2 - 1) * intensity * sign;
}
float FxSystem::shakeX() const { return shakeOffset(1); }
float FxSystem::shakeY() const { return shakeOffset(1); }

void FxSystem::addShake(float magnitude, float duration) {
  if (!enabled) return;
  shakeMagnitude_ = magnitude; shakeDuration_ = duration; shakeTime_ = duration;
}

void FxSystem::burst(float x, float y, const std::string& color, int count) {
  if (!enabled) return;
  for (int i = 0; i < count; i++) {
    float angle = randf() * 6.2831853f;
    float speed = randRange(35, 165);
    particles.push_back({ x, y, std::cos(angle) * speed, std::sin(angle) * speed,
                          randRange(0.35f, 0.7f), 0.7f, color, randRange(2, 6) });
  }
  if (particles.size() > 180) particles.erase(particles.begin(), particles.begin() + static_cast<ptrdiff_t>(particles.size() - 180));
}

void FxSystem::spawnDebris(float x, float y, const std::string& color, int count) {
  if (!enabled) return;
  for (int i = 0; i < count; i++) {
    float angle = -1.5707963f + (randf() - 0.5f) * 2.2f;
    float speed = randRange(60, 260);
    debris_.push_back({ x, y, std::cos(angle) * speed, std::sin(angle) * speed,
                        randf() * 6.2831853f, (randf() - 0.5f) * 14, randRange(0.5f, 0.9f), 0.9f,
                        color, randRange(4, 10) });
  }
}

void FxSystem::addText(float x, float y, const std::string& text, const std::string& color, float size) {
  if (!enabled) return;
  texts_.push_back({ x, y, -46, 0.9f, 0.9f, text, color, size });
}

void FxSystem::ring(float x, float y, const std::string& color) {
  if (!enabled) return;
  rings_.push_back({ x, y, 8, 230, 0.45f, 0.45f, color });
}

void FxSystem::flash(const std::string& color, float life) {
  if (!enabled) return;
  flashes_.push_back({ life, life, color });
}

} // namespace neon
