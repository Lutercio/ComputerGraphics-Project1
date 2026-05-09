#include <cmath>
#include <cstdlib>
#include <iostream>

#include "camera.hpp"
#include "sphere.hpp"

namespace {

constexpr int kWidth = 800;
constexpr int kHeight = 600;
constexpr gc::real_type kEpsilon = 1e-5F;

bool nearly_equal(gc::real_type lhs, gc::real_type rhs) {
    return std::abs(lhs - rhs) <= kEpsilon;
}

bool nearly_equal(const gc::Vector3f& lhs, const gc::Vector3f& rhs) {
    return nearly_equal(lhs.x, rhs.x) and
           nearly_equal(lhs.y, rhs.y) and
           nearly_equal(lhs.z, rhs.z);
}

template <typename CameraT>
int count_hits(const CameraT& camera, const gc::Sphere& sphere) {
    int hits = 0;
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            if (sphere.intersect_p(camera.generate_ray(x, y, kWidth, kHeight))) {
                ++hits;
            }
        }
    }
    return hits;
}

void require(bool condition, const char* message) {
    if (not condition) {
        std::cerr << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace

int main() {
    const gc::Point3f look_from{0, 0, 0};
    const gc::Point3f look_at{0, 0, 10};
    const gc::Vector3f up{0, 1, 0};

    const gc::OrthographicCamera ortho{
        look_from, look_at, up,
        -4, 4, -3, 3
    };

    const auto center_ray = ortho.generate_ray(kWidth / 2, kHeight / 2, kWidth, kHeight);
    const auto corner_ray = ortho.generate_ray(0, 0, kWidth, kHeight);
    const auto opposite_corner_ray = ortho.generate_ray(kWidth - 1, kHeight - 1, kWidth, kHeight);

    require(nearly_equal(center_ray.direction(), corner_ray.direction()),
            "orthographic rays must have the same direction");
    require(nearly_equal(center_ray.direction(), opposite_corner_ray.direction()),
            "orthographic rays must have the same direction across the image");

    const gc::Sphere near_sphere{{-1, 0.5F, 5}, 0.4F};
    const gc::Sphere far_sphere{{-1, 0.5F, 50}, 0.4F};

    const int near_ortho_hits = count_hits(ortho, near_sphere);
    const int far_ortho_hits = count_hits(ortho, far_sphere);
    require(near_ortho_hits == far_ortho_hits,
            "orthographic sphere footprint must not change with depth");

    const gc::PerspectiveCamera perspective{look_from, look_at, up, 60, 4.F / 3.F};
    const int near_perspective_hits = count_hits(perspective, near_sphere);
    const int far_perspective_hits = count_hits(perspective, far_sphere);
    require(near_perspective_hits > far_perspective_hits,
            "perspective sphere footprint should shrink with depth");

    return EXIT_SUCCESS;
}
