#include "scene/plane.hpp"

#include <cmath>

namespace gc {

namespace {

Vector3f normalize_or(const Vector3f& value, const Vector3f& fallback) {
  if (value.length_squared() <= machine_epsilon) {
    return fallback;
  }
  return normalize(value);
}

}  // namespace

Plane::Plane(const Point3f& point, const Vector3f& normal, std::shared_ptr<Material> mat)
    : Primitive{std::move(mat)}
    , m_point{point}
    , m_normal{normalize_or(normal, Vector3f{0, 1, 0})} {}

bool Plane::intersect_p(const Ray& r) const {
  const real_type denominator = dot(m_normal, r.direction());
  if (std::abs(denominator) <= machine_epsilon) {
    return false;
  }

  const real_type t = dot(m_point - r.origin(), m_normal) / denominator;
  return t >= r.t_min_val() and t <= r.t_max_val();
}

bool Plane::intersect(const Ray& r, Surfel* sf) const {
  const real_type denominator = dot(m_normal, r.direction());
  if (std::abs(denominator) <= machine_epsilon) {
    return false;
  }

  const real_type t = dot(m_point - r.origin(), m_normal) / denominator;
  if (t < r.t_min_val() or t > r.t_max_val()) {
    return false;
  }

  if (sf != nullptr) {
    Vector3f normal = m_normal;
    if (dot(normal, r.direction()) > 0.F) {
      normal = -normal;
    }

    sf->p = r(t);
    sf->n = normal;
    sf->wo = -r.direction();
    sf->time = t;
    sf->uv = Point2f{0, 0};
    sf->primitive = this;
  }

  return true;
}

}  // namespace gc
