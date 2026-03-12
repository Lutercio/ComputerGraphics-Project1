#ifndef BACKGROUND_HPP
#define BACKGROUND_HPP

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

#include "common.hpp"
#include "geometry.hpp"

namespace gc {
// TODO: Create a virtual class Background and derive BackgroundColor, BackgroundSphereImage,
// BackgroundSkyBoxImage.
/*!
 * A background is basically a rectangle, have a color associated to each corner.
 * A background might be sampled based on a normalized coordinate in \f$[0,1]^2.\f$.
 * We generate samples with bilinear interpolation of the corner colors.
 * We consider the corners in a circular clockwise order, starting at the lower-left corner.
 *
 * Alternatively, a background might be a regular image.
 */
class Background {
public:
  /// Types of color mapping scheme
  enum class mapping_t : std::uint8_t {
    screen = 0,  //!< background color is defined over a rectangle.
    spherical    //!< background color is defined over the inside of a sphere, camera at the center.
  };
  mapping_t mapping_type{ mapping_t::spherical };  //!< sampling type we use to pick a color.
  static constexpr float max_channel_value{ 255.F };

  /// Basic Ctro
  Background(mapping_t mt = mapping_t::spherical) : mapping_type{ mt } { /* empty */ }
  /// Default Dtro.
  virtual ~Background() = default;
  /// Returns a color to client.
  [[nodiscard]] virtual Spectrum sampleUV(real_type u, real_type v) const = 0;
};

class BackgroundSingleColor : public Background {
private:
  /// Each corner has a color associated with.
  Spectrum m_single_color{ 0, 0, 0 };

public:
  /// Ctro receives a single color.
  BackgroundSingleColor(const Spectrum& color, mapping_t mt = mapping_t::screen);
  /// Ctro receives a single color.
  BackgroundSingleColor(const Color24& color, mapping_t mt = mapping_t::screen);

  /// Defaul Dtro.
  ~BackgroundSingleColor() override = default;

  /// Sample and returns a color, based on the raster coordinates (u,v) of the screen.
  /*!
   * Assuming u,v \in [0,1], and origin at lower left corner.
   */
  [[nodiscard]] Spectrum sampleUV(real_type u, real_type v) const override;
};

class BackgroundMultiColor : public Background {
private:
  /// Each corner has a color associated with.
  std::array<Spectrum, 4> m_corners
    = { Spectrum{ 0, 0, 0 }, Spectrum{ 0, 0, 0 }, Spectrum{ 0, 0, 0 }, Spectrum{ 0, 0, 0 } };
  /// Corner indices.
  enum Corners_e : std::uint8_t {
    bl = 0,  //!< Bottom left corner.
    tl,      //!< Top left corner.
    tr,      //!< Top right corner.
    br       //!< Bottom right corner.
  };

  /// Return the linearly interpolated color in [A;B], based on the parameter \f$0\leq t \leq 1.\f$
  [[nodiscard]] static Spectrum lerp(const Spectrum& S, const Spectrum& E, float t);

public:
  /// Ctro receives a list of four colors, for each corner.
  BackgroundMultiColor(const std::array<Spectrum, 4>& colors, mapping_t mt = mapping_t::spherical)
      : Background{ mt } {
    // It's precondition to have 4 colors.
    assert(colors.size() == 4);
    // Copy colors to corners.
    m_corners = colors;
#ifdef DEBUG
    std::cout << ">>>> Inside BackgroundMultiColor()\n";
    std::cout << "    bl: " << m_corners[bl] << "\n";
    std::cout << "    tl: " << m_corners[tl] << "\n";
    std::cout << "    tr: " << m_corners[tr] << "\n";
    std::cout << "    br: " << m_corners[br] << "\n";
    std::cout << "<<<< Leaving BackgroundMultiColor()\n\n";
#endif
  }

  ~BackgroundMultiColor() override = default;

  /// Sample and returns a color, based on the raster coordinate or ray.
  [[nodiscard]] Spectrum sampleUV(real_type u, real_type v) const override;
};

// Factory pattern functions.
Background* create_color_background(std::string_view type, const ParamSet& ps);
}  // namespace gc
#endif
