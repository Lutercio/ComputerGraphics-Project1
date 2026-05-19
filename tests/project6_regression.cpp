#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include "scene/plane.hpp"
#include "scene/scene.hpp"
#include "scene/sphere.hpp"
#include "shading/light.hpp"
#include "shading/material.hpp"

namespace {

constexpr gc::real_type kEpsilon = 1e-4F;

bool nearly_equal(gc::real_type lhs, gc::real_type rhs) {
  return std::abs(lhs - rhs) <= kEpsilon;
}

bool nearly_equal(const gc::Vector3f& lhs, const gc::Vector3f& rhs) {
  return nearly_equal(lhs.x, rhs.x) and nearly_equal(lhs.y, rhs.y)
         and nearly_equal(lhs.z, rhs.z);
}

bool nearly_equal(const gc::Point3f& lhs, const gc::Point3f& rhs) {
  return nearly_equal(lhs.x, rhs.x) and nearly_equal(lhs.y, rhs.y)
         and nearly_equal(lhs.z, rhs.z);
}

void require(bool condition, const char* message) {
  if (not condition) {
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  const gc::BlinnPhongMaterial matte{gc::Spectrum{0.1F, 0.1F, 0.1F},
                                     gc::Spectrum{0.7F, 0.6F, 0.5F},
                                     gc::Spectrum{0.2F, 0.2F, 0.2F},
                                     32.F};
  require(nearly_equal(matte.km(), gc::Spectrum{0, 0, 0}),
          "Blinn material should default to no mirror reflection");

  const gc::BlinnPhongMaterial mirror{gc::Spectrum{0.1F, 0.1F, 0.1F},
                                      gc::Spectrum{0.7F, 0.6F, 0.5F},
                                      gc::Spectrum{0.2F, 0.2F, 0.2F},
                                      32.F,
                                      gc::Spectrum{0.4F, 0.5F, 0.6F}};
  require(nearly_equal(mirror.km(), gc::Spectrum{0.4F, 0.5F, 0.6F}),
          "Blinn material should preserve mirror coefficients");

  const auto material = std::make_shared<gc::FlatMaterial>(gc::Spectrum{1, 1, 1});
  const gc::Plane plane{gc::Point3f{0, 0, 0}, gc::Vector3f{0, 1, 0}, material};
  gc::Surfel plane_hit;
  require(plane.intersect(gc::Ray{gc::Point3f{0, 2, 0}, gc::Vector3f{0, -1, 0}}, &plane_hit),
          "plane should intersect rays crossing it");
  require(nearly_equal(plane_hit.p, gc::Point3f{0, 0, 0}),
          "plane hit point should be calculated from ray time");
  require(not plane.intersect_p(gc::Ray{gc::Point3f{0, 2, 0}, gc::Vector3f{1, 0, 0}}),
          "plane should reject parallel rays");

  gc::Surfel hit;
  hit.p = gc::Point3f{0, 0, 2};
  const gc::SpotLight spot{gc::Spectrum{1, 1, 1},
                           gc::Point3f{0, 0, 0},
                           gc::Point3f{0, 0, 1},
                           gc::Vector3f{1, 0, 0},
                           30.F,
                           15.F};
  gc::Vector3f wi;
  gc::real_type max_t = 0.F;
  require(nearly_equal(spot.sample_li(hit, &wi, &max_t), gc::Spectrum{1, 1, 1}),
          "spotlight should be full intensity inside falloff angle");
  require(nearly_equal(wi, gc::Vector3f{0, 0, -1}),
          "spotlight should point from hit point back to light position");
  require(nearly_equal(max_t, 2.F - gc::shadow_epsilon),
          "point-like lights should limit shadow rays to light distance");

  hit.p = gc::Point3f{2, 0, 2};
  require(nearly_equal(spot.sample_li(hit, &wi, &max_t), gc::Spectrum{0, 0, 0}),
          "spotlight should return no intensity outside cutoff angle");

  std::vector<std::shared_ptr<gc::Primitive>> primitives;
  primitives.push_back(std::make_shared<gc::Sphere>(gc::Point3f{0, 0, 10}, 2.F, material));
  const gc::Scene scene{nullptr, nullptr, nullptr, primitives};

  gc::DirectionalLight directional{gc::Spectrum{1, 1, 1},
                                   gc::Point3f{0, 0, -1},
                                   gc::Point3f{0, 0, 0}};
  directional.preprocess(scene);
  const auto directional_intensity = directional.sample_li(hit, &wi, &max_t);
  require(nearly_equal(directional_intensity, gc::Spectrum{1, 1, 1}),
          "directional light should preserve intensity after preprocessing");
  require(max_t >= 4.F, "directional light should derive a finite shadow range from scene bounds");

  gc::Bounds3f bounds;
  require(scene.world_bounds(&bounds), "scene should expose finite primitive bounds");
  require(nearly_equal(bounds.p_min, gc::Point3f{-2, -2, 8}),
          "sphere bounds should expose minimum extent");
  require(nearly_equal(bounds.p_max, gc::Point3f{2, 2, 12}),
          "sphere bounds should expose maximum extent");

  return EXIT_SUCCESS;
}
