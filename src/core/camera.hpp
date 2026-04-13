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
        // Left hand rule: w points into the scene
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

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(const Point3f& look_from, const Point3f& look_at,
                      const Vector3f& vup, real_type fovy, real_type aspect) : e{look_from} {
        // Left hand rule: w points into the scene
        w = normalize(Vector3f(look_at.x - look_from.x,
                               look_at.y - look_from.y,
                               look_at.z - look_from.z));   
        u = normalize(cross(vup, w));
        v = normalize(cross(w, u));
        // screen window
        real_type half_h = std::tan(fovy * M_PI / 180.f / 2.f);
        real_type half_w = aspect * half_h;
        top    =  half_h;
        bottom = -half_h;
        right  =  half_w;
        left   = -half_w;}
        
        Ray generate_ray(int i, int j, int nx, int ny) const override {
            real_type uu = left + (right - left) * (i + 0.5f) / nx;
            real_type vv = bottom + (top - bottom) * (ny - j - 0.5f) / ny;
            Vector3f dir = normalize(Vector3f(
                uu * u.x + vv * v.x + w.x,
                uu * u.y + vv * v.y + w.y,
                uu * u.z + vv * v.z + w.z
            ));
            return Ray(e, dir);
        }

        private:
            Point3f e;
            Vector3f u, v, w;
            real_type left, right, bottom, top;
    };

} // namespace gc

#endif // CAMERA_HPP