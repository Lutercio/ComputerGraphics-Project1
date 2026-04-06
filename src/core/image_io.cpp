#include <cstdint>
#include <fstream>
#include "image_io.hpp"
#include <lodepng.h>

namespace gc {

/// Saves an image as a **binary** PPM file.
bool save_ppm6(const std::uint8_t* data,
               size_t w,
               size_t h,
               size_t d,
               const std::string& file_name_) {
  std::ofstream out(file_name_, std::ios::binary);
  if (!out) return false;
  out << "P6\n" << w << " " << h << "\n255\n";
  out.write(reinterpret_cast<const char*>(data), w * h * d);
  return true;
}

/// Saves an image as a **ascii** PPM file.
bool save_ppm3(const std::uint8_t* data,
               size_t w,
               size_t h,
               size_t d,
               const std::string& file_name_) {
  std::ofstream out(file_name_);
  if (!out) return false;
  out << "P3\n" << w << " " << h << "\n255\n";
  for (size_t i = 0; i < w * h * d; ++i) {
    out << static_cast<int>(data[i]) << (i % d == d - 1 ? "\n" : " ");
  }
  return true;
}

bool save_png(const std::uint8_t* data,
              size_t w,
              size_t h,
              size_t d,
              const std::string& file_name_) {
  LodePNGColorType color_type = d == 3 ? LCT_RGB : LCT_RGBA;
  unsigned error = lodepng::encode(file_name_, data, w, h, color_type, 8);
  return error == 0;
}

}  // namespace gc

//================================[ imagem_io.h ]================================//
