#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "common.hpp"

namespace gc {

class Material {
public:
    virtual ~Material() = default;
    virtual Spectrum get_color() const = 0;
};

class FlatMaterial : public Material {
public:
    explicit FlatMaterial(const Spectrum& color) : m_color{color} {}
    Spectrum get_color() const override { return m_color; }
private:
    Spectrum m_color;
};

}  // namespace gc

#endif  // MATERIAL_HPP
