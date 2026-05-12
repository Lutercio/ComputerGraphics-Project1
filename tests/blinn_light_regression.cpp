#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include "shading/light.hpp"
#include "shading/material.hpp"
#include "scene/scene.hpp"
#include "scene/sphere.hpp"

namespace {

constexpr gc::real_type kEpsilon = 1e-5F;

bool nearly_equal(gc::real_type lhs, gc::real_type rhs) {
  return std::abs(lhs - rhs) <= kEpsilon;
}

bool nearly_equal(const gc::Vector3f& lhs, const gc::Vector3f& rhs) {
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
  const gc::BlinnPhongMaterial material{ gc::Spectrum{ 0.2F, 0.3F, 0.4F },
                                         gc::Spectrum{ 0.7F, 0.6F, 0.5F },
                                         gc::Spectrum{ 0.1F, 0.2F, 0.3F },
                                         64.F };

  require(nearly_equal(material.ka(), gc::Spectrum{ 0.2F, 0.3F, 0.4F }),
          "Blinn material should preserve ambient coefficients");
  require(nearly_equal(material.kd(), gc::Spectrum{ 0.7F, 0.6F, 0.5F }),
          "Blinn material should preserve diffuse coefficients");
  require(nearly_equal(material.ks(), gc::Spectrum{ 0.1F, 0.2F, 0.3F }),
          "Blinn material should preserve specular coefficients");
  require(nearly_equal(material.glossiness(), 64.F),
          "Blinn material should preserve glossiness");

  gc::Surfel hit;
  hit.p = gc::Point3f{ 0, 0, 0 };
  hit.n = gc::Vector3f{ 0, 0, 1 };

  gc::Vector3f wi{ 9, 9, 9 };
  const gc::AmbientLight ambient{ gc::Spectrum{ 0.1F, 0.2F, 0.3F } };
  require(nearly_equal(ambient.sample_li(hit, &wi), gc::Spectrum{ 0.1F, 0.2F, 0.3F }),
          "ambient light should return its intensity");
  require(nearly_equal(wi, gc::Vector3f{ 0, 0, 0 }),
          "ambient light should not provide a directional sample");

  const gc::DirectionalLight directional{ gc::Spectrum{ 0.4F, 0.5F, 0.6F },
                                          gc::Point3f{ 0, 0, -1 },
                                          gc::Point3f{ 0, 0, 1 } };
  require(nearly_equal(directional.sample_li(hit, &wi), gc::Spectrum{ 0.4F, 0.5F, 0.6F }),
          "directional light should return its scaled intensity");
  require(nearly_equal(wi, gc::Vector3f{ 0, 0, -1 }),
          "directional light should point from the hit point toward the light");

  const gc::PointLight point{ gc::Spectrum{ 1.F, 0.5F, 0.25F },
                              gc::Point3f{ 0, 0, 2 },
                              gc::Vector3f{ 1, 0, 1 } };
  require(nearly_equal(point.sample_li(hit, &wi), gc::Spectrum{ 0.2F, 0.1F, 0.05F }),
          "point light should apply inverse attenuation");
  require(nearly_equal(wi, gc::Vector3f{ 0, 0, 1 }),
          "point light should point from the hit point to the light position");

  std::vector<std::shared_ptr<gc::Primitive>> primitives;
  primitives.push_back(std::make_shared<gc::Sphere>(gc::Point3f{ 0, 0, 5 }, 1.F));

  std::vector<std::shared_ptr<gc::Light>> lights;
  lights.push_back(std::make_shared<gc::AmbientLight>(gc::Spectrum{ 0.1F, 0.1F, 0.1F }));

  const gc::Scene scene(nullptr, nullptr, nullptr, primitives, lights);
  gc::Surfel isect;
  const gc::Ray ray(gc::Point3f{ 0, 0, 0 }, gc::Vector3f{ 0, 0, 1 });

  require(scene.lights.size() == 1, "scene should expose parsed lights");
  require(scene.intersect(ray, &isect), "adding lights should not break primitive intersection");

  return EXIT_SUCCESS;
}
