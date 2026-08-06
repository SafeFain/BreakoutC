#pragma once
#include <string>
#include <vector>

#include "types.h"

namespace neon {

struct FloatText { float x = 0, y = 0, vy = 0, life = 0, maxLife = 0; std::string text; std::string color; float size = 0; };
struct Debris { float x = 0, y = 0, vx = 0, vy = 0, rotation = 0, vrot = 0, life = 0, maxLife = 0; std::string color; float size = 0; };
struct Ring { float x = 0, y = 0, radius = 0, vradius = 0, life = 0, maxLife = 0; std::string color; };
struct Flash { float life = 0, maxLife = 0; std::string color; };

// 与 web 版 fx.ts 对应:粒子/碎片/飘字/圆环/闪光/屏震
class FxSystem {
public:
  std::vector<ParticleState> particles;
  bool enabled = true;

  void update(float dt);
  void clear();

  void addShake(float magnitude, float duration);
  float shakeX() const;
  float shakeY() const;
  bool shaking() const { return shakeTime_ > 0; }

  void burst(float x, float y, const std::string& color, int count);
  void spawnDebris(float x, float y, const std::string& color, int count);
  void addText(float x, float y, const std::string& text, const std::string& color, float size = 15);
  void ring(float x, float y, const std::string& color);
  void flash(const std::string& color, float life = 0.25f);

  const std::vector<FloatText>& texts() const { return texts_; }
  const std::vector<Debris>& debris() const { return debris_; }
  const std::vector<Ring>& rings() const { return rings_; }
  const std::vector<Flash>& flashes() const { return flashes_; }

private:
  std::vector<FloatText> texts_;
  std::vector<Debris> debris_;
  std::vector<Ring> rings_;
  std::vector<Flash> flashes_;
  float shakeTime_ = 0, shakeDuration_ = 1, shakeMagnitude_ = 0;
  float shakeOffset(float sign) const;
};

} // namespace neon
