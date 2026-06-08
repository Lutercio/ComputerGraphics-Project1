#ifndef BVH_HPP
#define BVH_HPP

#include <memory>
#include <vector>

#include "scene/primitive.hpp"

namespace gc {

bool bounds_intersect_p(const Bounds3f& b, const Ray& ray, float& t_hit1, float& t_hit2);

class BVHAccel : public Primitive {
public:
    BVHAccel() : Primitive{nullptr} {}
    BVHAccel(std::vector<std::shared_ptr<Primitive>> prims, int max_per_leaf = 1);

    bool intersect(const Ray& r, Surfel* sf) const override;
    bool intersect_p(const Ray& r) const override;
    bool world_bounds(Bounds3f* bounds) const override;

    static std::shared_ptr<Primitive> build(std::vector<std::shared_ptr<Primitive>>& prims,
                                            std::size_t start, std::size_t end, int max_per_leaf);

    std::shared_ptr<Primitive> left;
    std::shared_ptr<Primitive> right;
    Bounds3f bbox;
    std::vector<std::shared_ptr<Primitive>> unbounded;
};

} // namespace gc

#endif // BVH_HPP
