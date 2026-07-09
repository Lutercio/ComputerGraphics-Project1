#include "shading/integrator.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "diagnostics/error.hpp"
#include "shading/material.hpp"

namespace gc {

namespace {

Spectrum lerp(const Spectrum& start, const Spectrum& end, real_type t) {
  t = clamp(t, 0.F, 1.F);
  return (start * (1.F - t)) + (end * t);
}

Spectrum multiply_components(const Spectrum& lhs, const Spectrum& rhs) {
  return Spectrum{ lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b };
}

Spectrum clamp_spectrum(const Spectrum& value) {
  return Spectrum{ clamp(value.r, 0.F, 1.F),
                   clamp(value.g, 0.F, 1.F),
                   clamp(value.b, 0.F, 1.F) };
}

Vector3f normalize_or(const Vector3f& value, const Vector3f& fallback) {
  if (value.length_squared() <= machine_epsilon) {
    return fallback;
  }
  return normalize(value);
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

void SamplerIntegrator::preprocess(const Scene& scene) {
  for (const auto& light : scene.lights) {
    if (light != nullptr) {
      light->preprocess(scene);
    }
  }
}

void SamplerIntegrator::render(const Scene& scene) {
  preprocess(scene);

  auto resolution = scene.film->get_resolution();
  int width = resolution.x;
  int height = resolution.y;

  const int n = m_samples;
  const real_type inv_samples = 1.F / real_type(n * n);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const real_type u = width > 1 ? real_type(x) / real_type(width - 1) : 0.F;
      const real_type v = height > 1 ? real_type(height - y - 1) / real_type(height - 1) : 0.F;

      Spectrum color{ 0, 0, 0 };
      for (int sy = 0; sy < n; ++sy) {
        for (int sx = 0; sx < n; ++sx) {
          const real_type dx = (real_type(sx) + 0.5F) / real_type(n);
          const real_type dy = (real_type(sy) + 0.5F) / real_type(n);
          Ray ray = scene.camera->generate_ray(x, y, width, height, dx, dy);
          auto radiance = Li(ray, scene);
          color += radiance.value_or(scene.background->sampleUV(u, v));
        }
      }
      scene.film->add_sample(Point2i{ x, y }, color * inv_samples);
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

BlinnPhongIntegrator::BlinnPhongIntegrator(int max_depth)
    : m_max_depth{ std::max(0, max_depth) } {}

std::optional<Spectrum> BlinnPhongIntegrator::Li(const Ray& ray, const Scene& scene) const {
  return Li(ray, scene, 0);
}

std::optional<Spectrum> BlinnPhongIntegrator::Li(const Ray& ray,
                                                 const Scene& scene,
                                                 int depth) const {
  Surfel isect;
  if (not scene.intersect(ray, &isect)) {
    return {};
  }

  const Material* material = isect.primitive != nullptr ? isect.primitive->get_material() : nullptr;

  // Additive glow: add glow color and keep tracing behind the hit.
  if (const auto* glow_material = dynamic_cast<const GlowMaterial*>(material)) {
    const Spectrum add = glow_material->glow(isect.uv);
    Spectrum behind{ 0.012F, 0.014F, 0.028F };  // deep-space fallback on a miss
    if (depth < 24) {
      const Ray continuation{ isect.p + (ray.direction() * shadow_epsilon), ray.direction(),
                              shadow_epsilon };
      if (auto b = Li(continuation, scene, depth + 1)) {
        behind = *b;
      }
    }
    return clamp_spectrum(behind + add);
  }

  Vector3f normal = normalize_or(isect.n, Vector3f{ 0, 0, 1 });
  if (dot(normal, ray.direction()) > 0.F) {
    return Spectrum{ 0, 0, 0 };
  }

  Spectrum ka{ 1, 1, 1 };
  Spectrum kd{ 1, 1, 1 };
  Spectrum ks{ 0, 0, 0 };
  Spectrum km{ 0, 0, 0 };
  Spectrum le{ 0, 0, 0 };
  real_type glossiness = 1.F;

  if (const auto* blinn_material = dynamic_cast<const BlinnPhongMaterial*>(material)) {
    ka = blinn_material->ka();
    kd = blinn_material->kd(isect.uv);
    ks = blinn_material->ks();
    km = blinn_material->km();
    le = blinn_material->le();
    glossiness = blinn_material->glossiness();
  } else if (material != nullptr) {
    ka = material->get_color();
    kd = material->get_color();
  }

  const Vector3f view = normalize_or(isect.wo, -ray.direction());
  Spectrum radiance = le;

  for (const auto& light : scene.lights) {
    if (light == nullptr) {
      continue;
    }

    if (light->is_ambient()) {
      radiance += multiply_components(light->sample_li(isect, nullptr), ka);
      continue;
    }

    Vector3f wi;
    real_type max_t = INFINITY;
    const Spectrum intensity = light->sample_li(isect, &wi, &max_t);
    wi = normalize_or(wi, Vector3f{ 0, 0, 0 });
    if (max_t <= shadow_epsilon or max_component(intensity) <= 0.F) {
      continue;
    }

    const real_type n_dot_l = std::max(0.F, dot(normal, wi));
    if (n_dot_l <= 0.F) {
      continue;
    }

    const Ray shadow_ray{ isect.p + (wi * shadow_epsilon), wi, shadow_epsilon, max_t };
    if (scene.intersect_p(shadow_ray)) {
      continue;
    }

    const Vector3f half_vector = normalize_or(wi + view, normal);
    const real_type n_dot_h = std::max(0.F, dot(normal, half_vector));
    const Spectrum diffuse = multiply_components(intensity, kd) * n_dot_l;
    const Spectrum specular = multiply_components(intensity, ks) * std::pow(n_dot_h, glossiness);
    radiance += diffuse + specular;
  }

  if (depth < m_max_depth and max_component(km) > 0.F) {
    Vector3f reflection_direction = ray.direction() - (2.F * dot(ray.direction(), normal) * normal);
    reflection_direction = normalize_or(reflection_direction, -ray.direction());
    const Ray reflected_ray{ isect.p + (reflection_direction * shadow_epsilon),
                             reflection_direction,
                             shadow_epsilon };
    auto reflected_radiance = Li(reflected_ray, scene, depth + 1);
    if (reflected_radiance) {
      radiance += multiply_components(km, *reflected_radiance);
    }
  }

  return clamp_spectrum(radiance);
}

std::unique_ptr<Integrator> create_integrator(const ParamSet& ps) {
  auto type = ps.retrieve<std::string>("type", "flat");
  const int samples = std::max(1, ps.retrieve<int>("samples", 1));

  auto make = [&]() -> std::unique_ptr<Integrator> {
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
  if (type == "blinn" or type == "blinn_phong" or type == "blinnphong") {
    int max_depth = 0;
    if (ps.contains<real_type>("depth")) {
      max_depth = static_cast<int>(std::max(0.F, ps.retrieve<real_type>("depth", 0.F)));
    } else if (ps.contains<real_type>("max_depth")) {
      max_depth = static_cast<int>(std::max(0.F, ps.retrieve<real_type>("max_depth", 0.F)));
    }
    return std::make_unique<BlinnPhongIntegrator>(max_depth);
  }

  std::ostringstream oss;
  oss << "Integrator type \"" << type << "\" unknown; using flat integrator.";
  WARNING(oss.str());
  return std::make_unique<RayCastIntegrator>();
  };

  auto integrator = make();
  if (auto* sampler = dynamic_cast<SamplerIntegrator*>(integrator.get())) {
    sampler->set_samples(samples);
  }
  return integrator;
}

}  // namespace gc
