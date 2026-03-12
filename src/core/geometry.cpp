#include <cstdint>
#include <glm/geometric.hpp>
#include <iomanip>
#include <iostream>

// #include "common.hpp"
#include "common.hpp"
#include "geometry.hpp"

namespace gc {

/// Prints a 3D vector.
std::ostream& operator<<(std::ostream& os, const Vector3f& v) {
#define REMOVE_NEG_ZERO(val) ((fabs(val) < 0.0005) ? 0.0 : val)
  float vx = REMOVE_NEG_ZERO(v.x);
  float vy = REMOVE_NEG_ZERO(v.y);
  float vz = REMOVE_NEG_ZERO(v.z);
#undef REMOVE_NEG_ZERO
  os << "[ " << vx << ", " << vy << ", " << vz << " ]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Vector3i& v) {
  os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Point3f& v) {
#define REMOVE_NEG_ZERO(val) ((fabs(val) < 0.0005) ? 0.0 : val)
  float vx = REMOVE_NEG_ZERO(v.x);
  float vy = REMOVE_NEG_ZERO(v.y);
  float vz = REMOVE_NEG_ZERO(v.z);
#undef REMOVE_NEG_ZERO
  os << "[ " << vx << ", " << vy << ", " << vz << " ]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const glm::mat4& mat) {
  // clang-format off
        os << std::setprecision(2) << std::fixed <<
              "[ " << std::setw(8) <<  mat[0][0]<< ","  << std::setw(8) << mat[0][1]<< ","  << std::setw(8) << mat[0][2]<< ","  << std::setw(8) << mat[0][3]<< " ]\n" <<
              "[ " << std::setw(8) <<  mat[1][0]<< ","  << std::setw(8) << mat[1][1]<< ","  << std::setw(8) << mat[1][2]<< ","  << std::setw(8) << mat[1][3]<< " ]\n" <<
              "[ " << std::setw(8) <<  mat[2][0]<< ","  << std::setw(8) << mat[2][1]<< ","  << std::setw(8) << mat[2][2]<< ","  << std::setw(8) << mat[2][3]<< " ]\n" <<
              "[ " << std::setw(8) <<  mat[3][0]<< ","  << std::setw(8) << mat[3][1]<< ","  << std::setw(8) << mat[3][2]<< ","  << std::setw(8) << mat[3][3]<< " ]\n";
  // clang-format on
  return os;
}

float radians(float deg) {
  constexpr float one_eighty{ 180 };
  return ((float)M_PI / one_eighty) * deg;
}

float degrees(float rad) {
  constexpr float one_eighty{ 180 };
  return (one_eighty / (float)M_PI) * rad;
}

}  // namespace gc
