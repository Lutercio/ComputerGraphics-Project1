#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include "ray.hpp"
#include "surfel.hpp"

namespace gc {

class Primitive {
public:
    virtual ~Primitive() = default;

    // Returns true if ray hits the primitive, fills Surfel with hit info.
    virtual bool intersect(const Ray& r, Surfel* sf) const = 0;

    // Faster version — only returns true/false, no hit info.
    virtual bool intersect_p(const Ray& r) const = 0;
};

} // namespace gc

#endif // PRIMITIVE_HPP