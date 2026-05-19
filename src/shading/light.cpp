#include "shading/light.hpp"

#include <algorithm>
#include <cmath>

#include "scene/scene.hpp"

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

real_type degrees_to_radians(real_type degrees) {
  return degrees * (pi / 180.F);
}

}  // namespace

void Light::preprocess(const Scene& scene) {
  (void)scene;
}

Spectrum AmbientLight::sample_li(const Surfel& hit, Vector3f* wi, real_type* max_t) const {
  (void)hit;
  if (wi != nullptr) {
    *wi = Vector3f{ 0, 0, 0 };
  }
  if (max_t != nullptr) {
    *max_t = 0.F;
  }
  return m_intensity;
}

DirectionalLight::DirectionalLight(const Spectrum& intensity,
                                   const Point3f& from,
                                   const Point3f& to,
                                   real_type world_radius)
    : Light{ intensity }
    , m_direction_to_light{ normalize_or(from - to, Vector3f{ 0, 0, -1 }) }
    , m_requested_world_radius{ std::max(0.F, world_radius) } {
  if (m_requested_world_radius > 0.F) {
    m_shadow_distance = m_requested_world_radius;
  }
}

void DirectionalLight::preprocess(const Scene& scene) {
  if (m_requested_world_radius > 0.F) {
    m_shadow_distance = m_requested_world_radius;
    return;
  }

  Bounds3f bounds;
  if (scene.world_bounds(&bounds)) {
    m_shadow_distance = std::max(bounds.diagonal().length(), 1.F);
  }
}

Spectrum DirectionalLight::sample_li(const Surfel& hit, Vector3f* wi, real_type* max_t) const {
  (void)hit;
  if (wi != nullptr) {
    *wi = m_direction_to_light;
  }
  if (max_t != nullptr) {
    *max_t = m_shadow_distance;
  }
  return m_intensity;
}

PointLight::PointLight(const Spectrum& intensity,
                       const Point3f& position,
                       const Vector3f& attenuation)
    : Light{ intensity }
    , m_position{ position }
    , m_attenuation{ attenuation } {}

Spectrum PointLight::sample_li(const Surfel& hit, Vector3f* wi, real_type* max_t) const {
  const Vector3f to_light = m_position - hit.p;
  const real_type distance_to_light = to_light.length();
  if (wi != nullptr) {
    *wi = normalize_or(to_light, Vector3f{ 0, 0, 0 });
  }
  if (max_t != nullptr) {
    *max_t = std::max(0.F, distance_to_light - shadow_epsilon);
  }

  return m_intensity * attenuation_factor(m_attenuation, distance_to_light);
}

SpotLight::SpotLight(const Spectrum& intensity,
                     const Point3f& position,
                     const Point3f& target,
                     const Vector3f& attenuation,
                     real_type cutoff_degrees,
                     real_type falloff_degrees)
    : PointLight{intensity, position, attenuation}
    , m_direction{normalize_or(target - position, Vector3f{0, -1, 0})} {
  const real_type outer_angle = clamp(std::max(cutoff_degrees, 0.F), 0.F, 90.F);
  const real_type inner_angle = clamp(std::max(falloff_degrees, 0.F), 0.F, outer_angle);
  m_cos_cutoff = std::cos(degrees_to_radians(outer_angle));
  m_cos_falloff = std::cos(degrees_to_radians(inner_angle));
}

Spectrum SpotLight::sample_li(const Surfel& hit, Vector3f* wi, real_type* max_t) const {
  const Spectrum point_intensity = PointLight::sample_li(hit, wi, max_t);
  const Vector3f light_to_hit = normalize_or(hit.p - position(), Vector3f{0, 0, 0});
  const real_type cos_angle = dot(light_to_hit, m_direction);

  if (cos_angle < m_cos_cutoff) {
    return Spectrum{0, 0, 0};
  }

  if (cos_angle >= m_cos_falloff or m_cos_falloff <= m_cos_cutoff + machine_epsilon) {
    return point_intensity;
  }

  const real_type factor = (cos_angle - m_cos_cutoff) / (m_cos_falloff - m_cos_cutoff);
  return point_intensity * clamp(factor, 0.F, 1.F);
}

}  // namespace gc
