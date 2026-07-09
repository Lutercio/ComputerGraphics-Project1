#include "shading/texture.hpp"

#include <cmath>
#include <cstddef>

#include <lodepng.h>

#include "diagnostics/error.hpp"

namespace gc {

ImageTexture::ImageTexture(const std::string& filename) {
  const unsigned error = lodepng::decode(m_pixels, m_width, m_height, filename);
  if (error != 0) {
    WARNING(std::string{ "Could not load texture \"" } + filename + "\": "
            + lodepng_error_text(error));
    m_width = 0;
    m_height = 0;
    m_pixels.clear();
  }
}

Spectrum ImageTexture::texel(int x, int y) const {
  const int w = static_cast<int>(m_width);
  const int h = static_cast<int>(m_height);
  x = ((x % w) + w) % w;
  y = ((y % h) + h) % h;

  const std::size_t index =
    ((static_cast<std::size_t>(y) * m_width) + static_cast<std::size_t>(x)) * 4;
  const real_type inv = 1.F / 255.F;
  return Spectrum{ static_cast<real_type>(m_pixels[index + 0]) * inv,
                   static_cast<real_type>(m_pixels[index + 1]) * inv,
                   static_cast<real_type>(m_pixels[index + 2]) * inv };
}

Spectrum ImageTexture::evaluate(const Point2f& uv) const {
  if (not valid()) {
    return Spectrum{ 1, 1, 1 };
  }

  const real_type u = uv.x;
  const real_type v = 1.F - uv.y;  // v flipped for OBJ convention

  const real_type fx = (u * static_cast<real_type>(m_width)) - 0.5F;
  const real_type fy = (v * static_cast<real_type>(m_height)) - 0.5F;

  const int x0 = static_cast<int>(std::floor(fx));
  const int y0 = static_cast<int>(std::floor(fy));
  const real_type dx = fx - static_cast<real_type>(x0);
  const real_type dy = fy - static_cast<real_type>(y0);

  const Spectrum c00 = texel(x0, y0);
  const Spectrum c10 = texel(x0 + 1, y0);
  const Spectrum c01 = texel(x0, y0 + 1);
  const Spectrum c11 = texel(x0 + 1, y0 + 1);

  const Spectrum top = (c00 * (1.F - dx)) + (c10 * dx);
  const Spectrum bottom = (c01 * (1.F - dx)) + (c11 * dx);
  return (top * (1.F - dy)) + (bottom * dy);
}

}  // namespace gc
