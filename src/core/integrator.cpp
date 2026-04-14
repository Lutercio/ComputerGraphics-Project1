#include "integrator.hpp"

namespace gc {

void FlatIntegrator::render(const Scene& scene) {
    auto res = scene.film->get_resolution();
    int w = res.x;
    int h = res.y;

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            Ray ray = scene.camera->generate_ray(i, j, w, h);

            auto color = scene.background->sampleUV(
                float(i) / float(w), float(h - j - 1) / float(h - 1));

            for (const auto& prim : scene.primitives) {
                if (prim->intersect_p(ray)) {
                    const Material* mat = prim->get_material();
                    if (mat) color = mat->get_color();
                    break;
                }
            }

            scene.film->add_sample(Point2i{i, j}, color);
        }
    }

    scene.film->write_image();
}

}  // namespace gc
