#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include <algorithm>
#include <memory>

#include "ray/ray.hpp"
#include "scene/surfel.hpp"
#include "shading/material.hpp"

namespace gc {

struct Bounds3f {
    Point3f p_min;
    Point3f p_max;

    Bounds3f()
        : p_min{INFINITY, INFINITY, INFINITY}
        , p_max{-INFINITY, -INFINITY, -INFINITY} {}

    Bounds3f(const Point3f& min_point, const Point3f& max_point)
        : p_min{min_point}
        , p_max{max_point} {}

    void expand(const Bounds3f& other) {
        p_min = min(p_min, other.p_min);
        p_max = max(p_max, other.p_max);
    }

    [[nodiscard]] Vector3f diagonal() const { return p_max - p_min; }

    /// Returns the index of the longest axis (0=x, 1=y, 2=z).
    [[nodiscard]] int max_extent() const {
        Vector3f d = diagonal();
        if (d.x > d.y && d.x > d.z) return 0;
        if (d.y > d.z) return 1;
        return 2;
    }

    /// Centroid of the bounding box.
    [[nodiscard]] Point3f centroid() const {
        return p_min + diagonal() * 0.5f;
    }
};

class Primitive {
public:
    explicit Primitive(std::shared_ptr<Material> mat = nullptr) : material{std::move(mat)} {}
    virtual ~Primitive() = default;

    // Returns true if ray hits the primitive, fills Surfel with hit info.
    virtual bool intersect(const Ray& r, Surfel* sf) const = 0;

    // Faster version — only returns true/false, no hit info.
    virtual bool intersect_p(const Ray& r) const = 0;

    virtual const Material* get_material() const { return material.get(); }

    [[nodiscard]] virtual bool world_bounds(Bounds3f* bounds) const {
        (void)bounds;
        return false;
    }

private:
    std::shared_ptr<Material> material;
};

} // namespace gc

#endif // PRIMITIVE_HPP
