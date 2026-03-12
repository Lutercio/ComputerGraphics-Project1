#ifndef IMAGE_HPP
# define IMAGE_HPP

# include <cstdint>
# include <string>

namespace gc {
/// Routines to write images to a file.
bool save_ppm6(const std::uint8_t*, size_t, size_t, size_t = 1, const std::string& = "image.ppm");

/// Saves an image as a **ascii** PPM file.
bool save_ppm3(const std::uint8_t*, size_t, size_t, size_t = 1, const std::string& = "image.ppm");

/// Saves an image as a PNG file.
bool save_png(const std::uint8_t*, size_t, size_t, size_t = 1, const std::string& = "image.png");
}  // namespace gc

#endif

//================================[ imagem.h ]================================//
