#ifndef SURFEL_HPP
#define SURFEL_HPP

#include "geometry.hpp"
#include "common.hpp"

namespace gc {

// Forward declaration
class Primitive;

struct Surfel {
    Surfel(const Point3f& p, const Vector3f& n, const Vector3f& wo, 
           real_type time, const Point2f& uv, const Primitive* pri)
        : p{p}, n{n}, wo{wo}, time{time}, uv{uv}, primitive{pri} {}

    Point3f p;                      //!< Contact point.
    Vector3f n;                     //!< Surface normal.
    Vector3f wo;                    //!< Outgoing direction of light (-ray).
    real_type time;                 //!< Time of intersection.
    Point2f uv;                     //!< Parametric coordinates (u,v).
    const Primitive* primitive = nullptr; //!< Pointer to the primitive hit.
};

}

#endif // SURFEL_HPP