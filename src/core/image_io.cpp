#include <cstdint>

#include <fstream>

#include "image_io.hpp"

namespace gc {

/// Saves an image as a **binary** PPM file.
bool save_ppm6(const std::uint8_t* data,
               size_t w,
               size_t h,
               size_t d,
               const std::string& file_name_) {
  // TODO:
  return true;  // STUB
}

/// Saves an image as a **ascii** PPM file.
bool save_ppm3(const std::uint8_t* data,
               size_t w,
               size_t h,
               size_t d,
               const std::string& file_name_) {
  // TODO:
  return true;  // STUB
}

bool save_png(const std::uint8_t* data,
              size_t w,
              size_t h,
              size_t d,
              const std::string& file_name_) {
  // TODO:
  return true;  // STUB
}

}  // namespace gc

//================================[ imagem_io.h ]================================//
