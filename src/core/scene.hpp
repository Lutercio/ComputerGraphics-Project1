#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>
#include <vector>

#include "camera.hpp"
#include "background.hpp"
#include "film.hpp"
#include "primitive.hpp"

namespace gc {

struct Scene {
    const Camera* camera;
    const Background* background;
    const Film* film;
    const std::vector<std::shared_ptr<Primitive>>& primitives;

    Scene(const Camera* cam, const Background* bkg, const Film* f,
          const std::vector<std::shared_ptr<Primitive>>& prims)
        : camera{cam}, background{bkg}, film{f}, primitives{prims} {}
};

}  // namespace gc

#endif  // SCENE_HPP
