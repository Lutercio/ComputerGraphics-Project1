#include "scene/bvh.hpp"

#include <algorithm>
#include <cstdlib>

namespace gc {

bool bounds_intersect_p(const Bounds3f& b, const Ray& ray, float& t_hit1, float& t_hit2) {
    float t0 = ray.t_min_val();
    float t1 = ray.t_max_val();
    for (int i = 0; i < 3; ++i) {
        float inv_d = 1.f / ray.direction()[i];
        float t_near = (b.p_min[i] - ray.origin()[i]) * inv_d;
        float t_far  = (b.p_max[i] - ray.origin()[i]) * inv_d;
        if (t_near > t_far) std::swap(t_near, t_far);
        t_far *= 1.f + 2.f * 1e-7f;
        t0 = std::max(t0, t_near);
        t1 = std::min(t1, t_far);
        if (t0 > t1) return false;
    }
    t_hit1 = t0;
    t_hit2 = t1;
    return true;
}

BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> prims, int max_per_leaf)
    : Primitive{nullptr}
{
    if (prims.empty()) return;

    std::vector<std::shared_ptr<Primitive>> bounded;
    for (auto& p : prims) {
        Bounds3f b;
        if (p->world_bounds(&b))
            bounded.push_back(p);
        else
            unbounded.push_back(p);
    }

    if (!bounded.empty()) {
        auto root = build(bounded, 0, bounded.size(), max_per_leaf);
        auto* bvh = static_cast<BVHAccel*>(root.get());
        left  = std::move(bvh->left);
        right = std::move(bvh->right);
        bbox  = bvh->bbox;
    }
}

std::shared_ptr<Primitive> BVHAccel::build(std::vector<std::shared_ptr<Primitive>>& prims,
                                            std::size_t start, std::size_t end, int max_per_leaf)
{
    auto node = std::make_shared<BVHAccel>();
    std::size_t n = end - start;

    if (n == 1) {
        node->left = node->right = prims[start];
        prims[start]->world_bounds(&node->bbox);
        return node;
    }

    if (n == 2) {
        node->left  = prims[start];
        node->right = prims[start + 1];
        Bounds3f lb, rb;
        node->left->world_bounds(&lb);
        node->right->world_bounds(&rb);
        node->bbox = lb;
        node->bbox.expand(rb);
        return node;
    }

    int axis = std::rand() % 3;
    std::sort(prims.begin() + static_cast<std::ptrdiff_t>(start),
              prims.begin() + static_cast<std::ptrdiff_t>(end),
              [axis](const std::shared_ptr<Primitive>& a, const std::shared_ptr<Primitive>& b) {
                  Bounds3f ba, bb;
                  a->world_bounds(&ba);
                  b->world_bounds(&bb);
                  float ca = (ba.p_min[axis] + ba.p_max[axis]) * 0.5f;
                  float cb = (bb.p_min[axis] + bb.p_max[axis]) * 0.5f;
                  return ca < cb;
              });

    std::size_t mid = start + n / 2;
    node->left  = build(prims, start, mid, max_per_leaf);
    node->right = build(prims, mid,   end, max_per_leaf);

    Bounds3f lb, rb;
    node->left->world_bounds(&lb);
    node->right->world_bounds(&rb);
    node->bbox = lb;
    node->bbox.expand(rb);
    return node;
}

bool BVHAccel::intersect(const Ray& r, Surfel* sf) const {
    bool hit = false;
    Surfel closest;

    for (const auto& p : unbounded) {
        Surfel candidate;
        if (p->intersect(r, &candidate) && candidate.time < closest.time) {
            hit     = true;
            closest = candidate;
        }
    }

    if (left || right) {
        float t_hit1, t_hit2;
        if (bounds_intersect_p(bbox, r, t_hit1, t_hit2)) {
            Surfel left_sf, right_sf;
            bool hit_left  = left  && left->intersect(r, &left_sf);
            bool hit_right = right && right->intersect(r, &right_sf);

            Surfel bvh_best;
            if (hit_left && hit_right)
                bvh_best = (left_sf.time < right_sf.time) ? left_sf : right_sf;
            else if (hit_left)
                bvh_best = left_sf;
            else if (hit_right)
                bvh_best = right_sf;

            if ((hit_left || hit_right) && bvh_best.time < closest.time) {
                hit     = true;
                closest = bvh_best;
            }
        }
    }

    if (hit && sf) *sf = closest;
    return hit;
}

bool BVHAccel::intersect_p(const Ray& r) const {
    for (const auto& p : unbounded)
        if (p->intersect_p(r)) return true;

    if (!left && !right) return false;
    float t_hit1, t_hit2;
    if (!bounds_intersect_p(bbox, r, t_hit1, t_hit2)) return false;
    return (left  && left->intersect_p(r))
        || (right && right->intersect_p(r));
}

bool BVHAccel::world_bounds(Bounds3f* bounds) const {
    if (bounds) *bounds = bbox;
    return true;
}

} // namespace gc
