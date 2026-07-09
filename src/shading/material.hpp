#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include "math/geometry.hpp"
#include "shading/texture.hpp"

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
                     real_type glossiness,
                     const Spectrum& mirror = Spectrum{0, 0, 0},
                     std::shared_ptr<Texture> diffuse_texture = nullptr,
                     const Spectrum& emission = Spectrum{0, 0, 0})
      : m_ambient{ ambient }
      , m_diffuse{ diffuse }
      , m_specular{ specular }
      , m_glossiness{ std::max(0.F, glossiness) }
      , m_mirror{ mirror }
      , m_diffuse_texture{ std::move(diffuse_texture) }
      , m_emission{ emission } {}

  [[nodiscard]] Spectrum get_color() const override { return m_diffuse; }
  [[nodiscard]] Spectrum ka() const { return m_ambient; }
  [[nodiscard]] Spectrum kd() const { return m_diffuse; }

  // Diffuse color, tinted by the texture when one is set.
  [[nodiscard]] Spectrum kd(const Point2f& uv) const {
    if (m_diffuse_texture == nullptr) {
      return m_diffuse;
    }
    const Spectrum t = m_diffuse_texture->evaluate(uv);
    return Spectrum{ t.r * m_diffuse.r, t.g * m_diffuse.g, t.b * m_diffuse.b };
  }

  [[nodiscard]] Spectrum ks() const { return m_specular; }
  [[nodiscard]] real_type glossiness() const { return m_glossiness; }
  [[nodiscard]] Spectrum km() const { return m_mirror; }
  [[nodiscard]] Spectrum le() const { return m_emission; }  // self-emitted radiance

private:
  Spectrum m_ambient;
  Spectrum m_diffuse;
  Spectrum m_specular;
  real_type m_glossiness;
  Spectrum m_mirror;
  std::shared_ptr<Texture> m_diffuse_texture;
  Spectrum m_emission;
};

// Additive, semi-transparent material: the integrator adds glow() on top of
// whatever lies behind the surface. Used for the laser halo.
class GlowMaterial : public Material {
public:
  explicit GlowMaterial(const Spectrum& color, std::shared_ptr<Texture> texture = nullptr)
      : m_color{ color }, m_texture{ std::move(texture) } {}

  [[nodiscard]] Spectrum get_color() const override { return m_color; }

  [[nodiscard]] Spectrum glow(const Point2f& uv) const {
    if (m_texture == nullptr) {
      return m_color;
    }
    const Spectrum t = m_texture->evaluate(uv);
    return Spectrum{ m_color.r * t.r, m_color.g * t.g, m_color.b * t.b };
  }

private:
  Spectrum m_color;
  std::shared_ptr<Texture> m_texture;
};

}  // namespace gc

#endif  // MATERIAL_HPP
