#include "sphere.hpp"
#include <cmath>

namespace gc {

bool Sphere::intersect_p(const Ray& r) const {
    Vector3f oc = Vector3f(r.origin().x - center.x,
                           r.origin().y - center.y,
                           r.origin().z - center.z);

    real_type a = dot(r.direction(), r.direction());
    real_type b = 2.f * dot(r.direction(), oc);
    real_type c = dot(oc, oc) - radius * radius;

    real_type delta = b * b - 4.f * a * c;

    if (delta < 0) return false;

    real_type sqrt_delta = std::sqrt(delta);
    real_type t1 = (-b - sqrt_delta) / (2.f * a);
    real_type t2 = (-b + sqrt_delta) / (2.f * a);

    // Verifica se algum t está dentro do intervalo válido
    if (t1 >= r.t_min_val() and t1 <= r.t_max_val()) return true;
    if (t2 >= r.t_min_val() and t2 <= r.t_max_val()) return true;

    return false;
}

bool Sphere::intersect(const Ray& r, Surfel* sf) const {
    Vector3f oc = Vector3f(r.origin().x - center.x,
                           r.origin().y - center.y,
                           r.origin().z - center.z);

    real_type a = dot(r.direction(), r.direction());
    real_type b = 2.f * dot(r.direction(), oc);
    real_type c = dot(oc, oc) - radius * radius;

    real_type delta = b * b - 4.f * a * c;

    if (delta < 0) return false;

    real_type sqrt_delta = std::sqrt(delta);
    real_type t1 = (-b - sqrt_delta) / (2.f * a);
    real_type t2 = (-b + sqrt_delta) / (2.f * a);

    real_type t = -1.f;
    if (t1 >= r.t_min_val() and t1 <= r.t_max_val()) t = t1;
    else if (t2 >= r.t_min_val() and t2 <= r.t_max_val()) t = t2;
    else return false;

    if (sf != nullptr) {
        Point3f p = r(t);
        Vector3f n = Vector3f((p.x - center.x) / radius,
                              (p.y - center.y) / radius,
                              (p.z - center.z) / radius);
        sf->p = p;
        sf->n = n;
        sf->wo = -r.direction();
        sf->time = t;
        sf->uv = Point2f(0, 0); // Simple UV mapping (could be improved)
        sf->primitive = this;
    }

    return true;
}

} // namespace gc