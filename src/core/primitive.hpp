#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include <memory>
#include "ray.hpp"
#include "surfel.hpp"
#include "material.hpp"

namespace gc {

class Primitive {
public:
    explicit Primitive(std::shared_ptr<Material> mat = nullptr) : material{std::move(mat)} {}
    virtual ~Primitive() = default;

    // Returns true if ray hits the primitive, fills Surfel with hit info.
    virtual bool intersect(const Ray& r, Surfel* sf) const = 0;

    // Faster version — only returns true/false, no hit info.
    virtual bool intersect_p(const Ray& r) const = 0;

    virtual const Material* get_material() const { return material.get(); }

private:
    std::shared_ptr<Material> material;
};

} // namespace gc

#endif // PRIMITIVE_HPP