#include "integrator.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "../msg_system/error.hpp"
#include "material.hpp"

namespace gc {

namespace {

Spectrum lerp(const Spectrum& start, const Spectrum& end, real_type t) {
  t = clamp(t, 0.F, 1.F);
  return (start * (1.F - t)) + (end * t);
}

std::optional<real_type> optional_real(const ParamSet& ps, const std::string& key) {
  if (ps.contains<real_type>(key)) {
    return ps.retrieve<real_type>(key);
  }
  return {};
}

Spectrum optional_color(const ParamSet& ps, const std::string& key, const Spectrum& fallback) {
  if (ps.contains<std::vector<real_type>>(key)) {
    return rgb_values_to_spectrum(ps.retrieve<std::vector<real_type>>(key), fallback);
  }
  return fallback;
}

}  // namespace

void SamplerIntegrator::preprocess(const Scene& scene) {}

void SamplerIntegrator::render(const Scene& scene) {
  preprocess(scene);

  auto resolution = scene.film->get_resolution();
  int width = resolution.x;
  int height = resolution.y;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      Ray ray = scene.camera->generate_ray(x, y, width, height);
      auto radiance = Li(ray, scene);

      real_type u = width > 1 ? real_type(x) / real_type(width - 1) : 0.F;
      real_type v = height > 1 ? real_type(height - y - 1) / real_type(height - 1) : 0.F;
      Spectrum color = radiance.value_or(scene.background->sampleUV(u, v));

      scene.film->add_sample(Point2i{ x, y }, color);
    }
  }

  scene.film->write_image();
}

std::optional<Spectrum> RayCastIntegrator::Li(const Ray& ray, const Scene& scene) const {
  Surfel isect;
  if (not scene.intersect(ray, &isect)) {
    return {};
  }

  const Material* material = isect.primitive != nullptr ? isect.primitive->get_material() : nullptr;
  if (material == nullptr) {
    return Spectrum{ 1, 1, 1 };
  }

  if (const auto* flat_material = dynamic_cast<const FlatMaterial*>(material)) {
    return flat_material->kd();
  }

  return material->get_color();
}

DepthMapIntegrator::DepthMapIntegrator(std::optional<real_type> z_min,
                                       std::optional<real_type> z_max,
                                       const Spectrum& near_color,
                                       const Spectrum& far_color)
    : m_requested_z_min{z_min}
    , m_requested_z_max{z_max}
    , m_near_color{near_color}
    , m_far_color{far_color} {}

void DepthMapIntegrator::preprocess(const Scene& scene) {
  auto resolution = scene.film->get_resolution();
  real_type auto_z_min = INFINITY;
  real_type auto_z_max = 0.F;

  for (int y = 0; y < resolution.y; ++y) {
    for (int x = 0; x < resolution.x; ++x) {
      Ray ray = scene.camera->generate_ray(x, y, resolution.x, resolution.y);
      Surfel isect;
      if (scene.intersect(ray, &isect)) {
        auto_z_min = std::min(auto_z_min, isect.time);
        auto_z_max = std::max(auto_z_max, isect.time);
      }
    }
  }

  if (auto_z_min == INFINITY) {
    auto_z_min = 0.F;
    auto_z_max = 1.F;
  }

  m_z_min = m_requested_z_min.value_or(auto_z_min);
  m_z_max = m_requested_z_max.value_or(auto_z_max);

  if (m_requested_z_min and m_requested_z_max and *m_requested_z_min >= 0.F
      and *m_requested_z_max <= 1.F and auto_z_max > auto_z_min) {
    m_z_min = auto_z_min + ((*m_requested_z_min) * (auto_z_max - auto_z_min));
    m_z_max = auto_z_min + ((*m_requested_z_max) * (auto_z_max - auto_z_min));
  }

  if (m_z_max <= m_z_min) {
    m_z_max = m_z_min + 1.F;
  }
}

std::optional<Spectrum> DepthMapIntegrator::Li(const Ray& ray, const Scene& scene) const {
  Surfel isect;
  if (not scene.intersect(ray, &isect)) {
    return {};
  }

  real_type t = (isect.time - m_z_min) / (m_z_max - m_z_min);
  return lerp(m_near_color, m_far_color, t);
}

std::optional<Spectrum> NormalMapIntegrator::Li(const Ray& ray, const Scene& scene) const {
  Surfel isect;
  if (not scene.intersect(ray, &isect)) {
    return {};
  }

  Vector3f n = normalize(isect.n);
  return Spectrum{ (n.x + 1.F) * 0.5F, (n.y + 1.F) * 0.5F, (n.z + 1.F) * 0.5F };
}

std::unique_ptr<Integrator> create_integrator(const ParamSet& ps) {
  auto type = ps.retrieve<std::string>("type", "flat");

  if (type == "flat" or type == "raycast" or type == "ray_cast") {
    return std::make_unique<RayCastIntegrator>();
  }
  if (type == "depth" or type == "depth_map") {
    return std::make_unique<DepthMapIntegrator>(
      optional_real(ps, "zmin"),
      optional_real(ps, "zmax"),
      optional_color(ps, "near_color", Spectrum{0, 0, 0}),
      optional_color(ps, "far_color", Spectrum{1, 1, 1}));
  }
  if (type == "normal" or type == "normal_map") {
    return std::make_unique<NormalMapIntegrator>();
  }

  std::ostringstream oss;
  oss << "Integrator type \"" << type << "\" unknown; using flat integrator.";
  WARNING(oss.str());
  return std::make_unique<RayCastIntegrator>();
}

}  // namespace gc
