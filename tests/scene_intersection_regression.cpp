#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include "shading/material.hpp"
#include "scene/scene.hpp"
#include "scene/sphere.hpp"

namespace {

void require(bool condition, const char* message) {
  if (not condition) {
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  auto far_material = std::make_shared<gc::FlatMaterial>(gc::Spectrum{ 1, 0, 0 });
  auto near_material = std::make_shared<gc::FlatMaterial>(gc::Spectrum{ 0, 1, 0 });

  std::vector<std::shared_ptr<gc::Primitive>> primitives;
  primitives.push_back(std::make_shared<gc::Sphere>(gc::Point3f{ 0, 0, 5 }, 1.F, far_material));
  primitives.push_back(std::make_shared<gc::Sphere>(gc::Point3f{ 0, 0, 3 }, 1.F, near_material));

  gc::Scene scene(nullptr, nullptr, nullptr, primitives);
  gc::Surfel isect;
  gc::Ray ray(gc::Point3f{ 0, 0, 0 }, gc::Vector3f{ 0, 0, 1 });

  require(scene.intersect(ray, &isect), "ray should hit at least one sphere");
  require(isect.primitive == primitives[1].get(), "scene intersection should return closest hit");
  require(isect.primitive->get_material()->get_color() == gc::Spectrum{ 0, 1, 0 },
          "closest hit should preserve its material");

  return EXIT_SUCCESS;
}
