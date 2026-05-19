#ifndef PLANE_HPP
#define PLANE_HPP

#include "scene/primitive.hpp"

namespace gc {

class Plane : public Primitive {
public:
  Plane(const Point3f& point, const Vector3f& normal, std::shared_ptr<Material> mat = nullptr);

  bool intersect(const Ray& r, Surfel* sf) const override;
  bool intersect_p(const Ray& r) const override;

private:
  Point3f m_point;
  Vector3f m_normal;
};

}  // namespace gc

#endif  // PLANE_HPP
