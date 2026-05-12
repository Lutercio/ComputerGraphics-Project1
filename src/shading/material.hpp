#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <algorithm>
#include <vector>

#include "math/geometry.hpp"

namespace gc {

inline Spectrum rgb_values_to_spectrum(const std::vector<real_type>& values,
                                       const Spectrum& fallback = Spectrum{1, 1, 1}) {
  if (values.size() < 3) {
    return fallback;
  }

  Spectrum color{ values[0], values[1], values[2] };
  if (std::max({ color.r, color.g, color.b }) > 1.F) {
    color = color / 255.F;
  }

  color.r = clamp(color.r, 0.F, 1.F);
  color.g = clamp(color.g, 0.F, 1.F);
  color.b = clamp(color.b, 0.F, 1.F);
  return color;
}

class Material {
public:
  virtual ~Material() = default;
  [[nodiscard]] virtual Spectrum get_color() const = 0;
};

class FlatMaterial : public Material {
public:
  explicit FlatMaterial(const Spectrum& color) : m_color{ color } {}
  [[nodiscard]] Spectrum get_color() const override { return m_color; }
  [[nodiscard]] Spectrum kd() const { return m_color; }

private:
  Spectrum m_color;
};

class BlinnPhongMaterial : public Material {
public:
  BlinnPhongMaterial(const Spectrum& ambient,
                     const Spectrum& diffuse,
                     const Spectrum& specular,
                     real_type glossiness)
      : m_ambient{ ambient }
      , m_diffuse{ diffuse }
      , m_specular{ specular }
      , m_glossiness{ std::max(0.F, glossiness) } {}

  [[nodiscard]] Spectrum get_color() const override { return m_diffuse; }
  [[nodiscard]] Spectrum ka() const { return m_ambient; }
  [[nodiscard]] Spectrum kd() const { return m_diffuse; }
  [[nodiscard]] Spectrum ks() const { return m_specular; }
  [[nodiscard]] real_type glossiness() const { return m_glossiness; }

private:
  Spectrum m_ambient;
  Spectrum m_diffuse;
  Spectrum m_specular;
  real_type m_glossiness;
};

}  // namespace gc

#endif  // MATERIAL_HPP
