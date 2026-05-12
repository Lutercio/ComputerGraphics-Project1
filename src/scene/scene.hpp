#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>
#include <vector>

#include "ray/camera.hpp"
#include "scene/background.hpp"
#include "output/film.hpp"
#include "shading/light.hpp"
#include "scene/primitive.hpp"

namespace gc {

struct Scene {
  const Camera* camera;
  const Background* background;
  const Film* film;
  const std::vector<std::shared_ptr<Primitive>>& primitives;
  const std::vector<std::shared_ptr<Light>>& lights;

  static const std::vector<std::shared_ptr<Light>>& empty_lights() {
    static const std::vector<std::shared_ptr<Light>> lights;
    return lights;
  }

  Scene(const Camera* cam,
        const Background* bkg,
        const Film* f,
        const std::vector<std::shared_ptr<Primitive>>& prims)
      : Scene{ cam, bkg, f, prims, empty_lights() } {}

  Scene(const Camera* cam,
        const Background* bkg,
        const Film* f,
        const std::vector<std::shared_ptr<Primitive>>& prims,
        const std::vector<std::shared_ptr<Light>>& scene_lights)
      : camera{ cam }
      , background{ bkg }
      , film{ f }
      , primitives{ prims }
      , lights{ scene_lights } {}

  [[nodiscard]] bool intersect(const Ray& ray, Surfel* isect) const {
    bool hit = false;
    real_type closest_t = INFINITY;
    Surfel closest;

    for (const auto& primitive : primitives) {
      Surfel candidate;
      if (primitive->intersect(ray, &candidate) and candidate.time < closest_t) {
        hit = true;
        closest_t = candidate.time;
        closest = candidate;
      }
    }

    if (hit and isect != nullptr) {
      *isect = closest;
    }
    return hit;
  }

  [[nodiscard]] bool intersect_p(const Ray& ray) const {
    for (const auto& primitive : primitives) {
      if (primitive->intersect_p(ray)) {
        return true;
      }
    }
    return false;
  }
};

}  // namespace gc

#endif  // SCENE_HPP
