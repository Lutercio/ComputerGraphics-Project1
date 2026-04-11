#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "primitive.hpp"

namespace gc {

class Sphere : public Primitive {
public:
    Sphere(const Point3f& center, real_type radius)
        : center{center}, radius{radius} {}

    bool intersect(const Ray& r, Surfel* sf) const override;
    bool intersect_p(const Ray& r) const override;

private:
    Point3f center;
    real_type radius;
};

} // namespace gc

#endif // SPHERE_HPP