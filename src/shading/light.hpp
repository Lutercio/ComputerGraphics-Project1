#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <cstdint>

#include "math/geometry.hpp"
#include "scene/surfel.hpp"

namespace gc {

enum class LightType : std::uint8_t {
  ambient,
  directional,
  point,
};

class Light {
public:
  explicit Light(const Spectrum& intensity) : m_intensity{ intensity } {}
  virtual ~Light() = default;

  [[nodiscard]] virtual LightType type() const = 0;
  [[nodiscard]] bool is_ambient() const { return type() == LightType::ambient; }

  [[nodiscard]] virtual Spectrum sample_li(const Surfel& hit, Vector3f* wi) const = 0;

protected:
  Spectrum m_intensity;
};

class AmbientLight : public Light {
public:
  explicit AmbientLight(const Spectrum& intensity) : Light{ intensity } {}

  [[nodiscard]] LightType type() const override { return LightType::ambient; }
  [[nodiscard]] Spectrum sample_li(const Surfel& hit, Vector3f* wi) const override;
};

class DirectionalLight : public Light {
public:
  DirectionalLight(const Spectrum& intensity, const Point3f& from, const Point3f& to);

  [[nodiscard]] LightType type() const override { return LightType::directional; }
  [[nodiscard]] Spectrum sample_li(const Surfel& hit, Vector3f* wi) const override;

private:
  Vector3f m_direction_to_light;
};

class PointLight : public Light {
public:
  PointLight(const Spectrum& intensity, const Point3f& position, const Vector3f& attenuation);

  [[nodiscard]] LightType type() const override { return LightType::point; }
  [[nodiscard]] Spectrum sample_li(const Surfel& hit, Vector3f* wi) const override;

private:
  Point3f m_position;
  Vector3f m_attenuation;
};

}  // namespace gc

#endif  // LIGHT_HPP
