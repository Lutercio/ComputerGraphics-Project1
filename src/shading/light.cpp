#include "shading/light.hpp"

namespace gc {

namespace {

Vector3f normalize_or(const Vector3f& value, const Vector3f& fallback) {
  if (value.length_squared() <= machine_epsilon) {
    return fallback;
  }
  return normalize(value);
}

real_type attenuation_factor(const Vector3f& attenuation, real_type distance_to_light) {
  const real_type denominator = attenuation.x + (attenuation.y * distance_to_light)
                                + (attenuation.z * distance_to_light * distance_to_light);
  if (denominator <= machine_epsilon) {
    return 1.F;
  }
  return 1.F / denominator;
}

}  // namespace

Spectrum AmbientLight::sample_li(const Surfel& hit, Vector3f* wi) const {
  (void)hit;
  if (wi != nullptr) {
    *wi = Vector3f{ 0, 0, 0 };
  }
  return m_intensity;
}

DirectionalLight::DirectionalLight(const Spectrum& intensity,
                                   const Point3f& from,
                                   const Point3f& to)
    : Light{ intensity }
    , m_direction_to_light{ normalize_or(from - to, Vector3f{ 0, 0, -1 }) } {}

Spectrum DirectionalLight::sample_li(const Surfel& hit, Vector3f* wi) const {
  (void)hit;
  if (wi != nullptr) {
    *wi = m_direction_to_light;
  }
  return m_intensity;
}

PointLight::PointLight(const Spectrum& intensity,
                       const Point3f& position,
                       const Vector3f& attenuation)
    : Light{ intensity }
    , m_position{ position }
    , m_attenuation{ attenuation } {}

Spectrum PointLight::sample_li(const Surfel& hit, Vector3f* wi) const {
  const Vector3f to_light = m_position - hit.p;
  const real_type distance_to_light = to_light.length();
  if (wi != nullptr) {
    *wi = normalize_or(to_light, Vector3f{ 0, 0, 0 });
  }

  return m_intensity * attenuation_factor(m_attenuation, distance_to_light);
}

}  // namespace gc
