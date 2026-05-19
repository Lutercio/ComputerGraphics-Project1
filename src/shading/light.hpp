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
  spot,
};

struct Scene;

class Light {
public:
  explicit Light(const Spectrum& intensity) : m_intensity{ intensity } {}
  virtual ~Light() = default;

  [[nodiscard]] virtual LightType type() const = 0;
  [[nodiscard]] bool is_ambient() const { return type() == LightType::ambient; }

  virtual void preprocess(const Scene& scene);
  [[nodiscard]] virtual Spectrum sample_li(const Surfel& hit,
                                           Vector3f* wi,
                                           real_type* max_t = nullptr) const = 0;

protected:
  Spectrum m_intensity;
};

class AmbientLight : public Light {
public:
  explicit AmbientLight(const Spectrum& intensity) : Light{ intensity } {}

  [[nodiscard]] LightType type() const override { return LightType::ambient; }
  [[nodiscard]] Spectrum sample_li(const Surfel& hit,
                                   Vector3f* wi,
                                   real_type* max_t = nullptr) const override;
};

class DirectionalLight : public Light {
public:
  DirectionalLight(const Spectrum& intensity,
                   const Point3f& from,
                   const Point3f& to,
                   real_type world_radius = 0.F);

  [[nodiscard]] LightType type() const override { return LightType::directional; }
  void preprocess(const Scene& scene) override;
  [[nodiscard]] Spectrum sample_li(const Surfel& hit,
                                   Vector3f* wi,
                                   real_type* max_t = nullptr) const override;

private:
  Vector3f m_direction_to_light;
  real_type m_shadow_distance{ 1000.F };
  real_type m_requested_world_radius{ 0.F };
};

class PointLight : public Light {
public:
  PointLight(const Spectrum& intensity, const Point3f& position, const Vector3f& attenuation);

  [[nodiscard]] LightType type() const override { return LightType::point; }
  [[nodiscard]] Spectrum sample_li(const Surfel& hit,
                                   Vector3f* wi,
                                   real_type* max_t = nullptr) const override;

protected:
  [[nodiscard]] Point3f position() const { return m_position; }
  [[nodiscard]] Vector3f attenuation() const { return m_attenuation; }

private:
  Point3f m_position;
  Vector3f m_attenuation;
};

class SpotLight : public PointLight {
public:
  SpotLight(const Spectrum& intensity,
            const Point3f& position,
            const Point3f& target,
            const Vector3f& attenuation,
            real_type cutoff_degrees,
            real_type falloff_degrees);

  [[nodiscard]] LightType type() const override { return LightType::spot; }
  [[nodiscard]] Spectrum sample_li(const Surfel& hit,
                                   Vector3f* wi,
                                   real_type* max_t = nullptr) const override;

private:
  Vector3f m_direction;
  real_type m_cos_cutoff{ 0.F };
  real_type m_cos_falloff{ 0.F };
};

}  // namespace gc

#endif  // LIGHT_HPP
