#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>
#include <vector>

#include "math/geometry.hpp"

namespace gc {

/// Maps parametric (u,v) coordinates to a color.
class Texture {
public:
  virtual ~Texture() = default;
  [[nodiscard]] virtual Spectrum evaluate(const Point2f& uv) const = 0;
};

/// A PNG image sampled with bilinear filtering and wrap-around addressing.
class ImageTexture : public Texture {
public:
  explicit ImageTexture(const std::string& filename);

  [[nodiscard]] bool valid() const { return m_width > 0 and m_height > 0; }
  [[nodiscard]] Spectrum evaluate(const Point2f& uv) const override;

private:
  [[nodiscard]] Spectrum texel(int x, int y) const;

  unsigned m_width{ 0 };
  unsigned m_height{ 0 };
  std::vector<unsigned char> m_pixels;  //!< RGBA, 8 bits per channel.
};

}  // namespace gc

#endif  // TEXTURE_HPP
