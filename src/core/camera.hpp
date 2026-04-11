#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "ray.hpp"

namespace gc {

class Camera {
public:
    virtual ~Camera() = default;
    virtual Ray generate_ray(int i, int j, int w, int h) const = 0;
};

class OrthographicCamera : public Camera {
public:
    OrthographicCamera(const Point3f& look_from, const Point3f& look_at,
                       const Vector3f& vup,
                       real_type left, real_type right,
                       real_type bottom, real_type top)
        : e{look_from}
        , left{left}, right{right}, bottom{bottom}, top{top}
    {
        // Mão esquerda: w aponta para dentro da cena
        w = normalize(Vector3f(look_at.x - look_from.x,
                               look_at.y - look_from.y,
                               look_at.z - look_from.z));
        u = normalize(cross(vup, w));
        v = normalize(cross(w, u));
    }

    Ray generate_ray(int i, int j, int nx, int ny) const override {
        real_type uu = left + (right - left) * (i + 0.5f) / nx;
        real_type vv = bottom + (top - bottom) * (ny - j + 0.5f) / ny;
        Point3f origin(e.x + uu * u.x + vv * v.x,
                       e.y + uu * u.y + vv * v.y,
                       e.z + uu * u.z + vv * v.z);
        return Ray(origin, w);
    }

private:
    Point3f e;          // look_from
    Vector3f u, v, w;  // camera frame
    real_type left, right, bottom, top;
};

} // namespace gc

#endif // CAMERA_HPP