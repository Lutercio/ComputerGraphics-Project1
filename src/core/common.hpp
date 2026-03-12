#ifndef COMMON_HPP
#define COMMON_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int3.hpp>

namespace gc {

//== Global Forward Declarations
class AmbientLight;
class AreaLight;
class BlinnMaterial;
class Camera;
class DirectionalLight;
class Film;
class FlatMaterial;
class FlatShader;
class Integrator;
class Light;
class Material;
class OrthographicCamera;
class ParamSet;
class PerspectiveCamera;
class PointLight;
class Primitive;
class Ray;
class Scene;
class Shape;
class Sphere;
struct Surfel;
class Transform;
class WhittedIntegrator;
// template <typename T>
// class Vector2;
// template <typename T>
// class Vector3;
// template <typename T>
// class Point3;
// template <typename T>
// class Point2;
// template <typename T>
// class Normal3;
// template <typename T>
// class Bounds2;
// template <typename T>
// class Bounds3;

//=== aliases
// Forward declarations
template <typename T>
class Vector3;

using real_type = float;
using size_type = size_t;
using byte = std::uint8_t;
using Color24 = Vector3<int>;
using ColorXYZ = Vector3<real_type>;
using Spectrum = Vector3<real_type>;
using Offset3f = Vector3<real_type>;
using Radiance3f = Vector3<real_type>;
using Biradiance3f = Vector3<real_type>;
using Power3f = Vector3<real_type>;

#ifdef M_PI
# undef M_PI
#endif
#define M_PI       3.14159265358979323846f
#define INV_PI     0.31830988618379067154f
#define INV_TWOPI  0.15915494309189533577f
#define INV_FOURPI 0.07957747154594766788f
#ifndef INFINITY
# define INFINITY FLT_MAX
#endif

// Add here Option structure.
struct RunningOptions {
  RunningOptions(std::string filename = "", std::string outfile = "")
      : filename(std::move(filename)), outfile(std::move(outfile)) {}
  // -----------------------------------------
  // [ x0, x1 ] -> X values
  // [ y0, y1 ] -> Y values
  // where x0,x1,y0,y1 in [0.0, 1.0].
  // 1 = 100% of the full resolution.
  // -----------------------------------------
  /// Crop window to render.
  std::array<float, 4> crop_window{ 0, 1, 0, 1 };
  std::string filename;        //!< input scene file name.
  std::string outfile;         //!< output image file name.
  bool quick_render{ false };  //!< when set, render image with 1/4 of the requested resolition.
  bool verbose{ false };       //!< when set, the program shows lots of debug message.
  bool crop_window_provided{ false };  //!< when set, we got crop window specification via CLI.
};

/// Lambda expression that returns a lowercase version of the input string.
auto str_lowercase = [](std::string str) -> std::string {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
};

}  // namespace gc

#endif  // !COMMON_HPP
