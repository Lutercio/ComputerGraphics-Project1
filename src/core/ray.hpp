#ifndef RAY_HPP
#define RAY_HPP

#include "common.hpp"
#include "geometry.hpp"

namespace gc {

class Ray {
public:
    Ray() : t_min{0.f}, t_max{INFINITY} {}

    Ray(const Point3f& o, const Vector3f& d,
        real_type start = 0, real_type end = INFINITY)
        : o{o}, d{d}, t_min{start}, t_max{end} {}

    // Returns point at parameter t: o + d*t
    Point3f operator()(real_type t) const {
        return o + d * t;
    }

    void normalize() {
        d = gc::normalize(d);
    }

    Point3f origin() const { return o; }
    Vector3f direction() const { return d; }
    real_type t_min_val() const { return t_min; }
    real_type t_max_val() const { return t_max; }

private:
    Point3f o;
    Vector3f d;
    mutable real_type t_min;
    mutable real_type t_max;
};

} // namespace gc

#endif // RAY_HPP