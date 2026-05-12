#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <vector>

#include <glm/ext/scalar_constants.hpp>

#include "scene/background.hpp"
#include "math/common.hpp"
#include "math/geometry.hpp"
#include "input/paramset.hpp"

namespace gc {

namespace {

Spectrum rgb_values_to_spectrum(const std::vector<real_type>& values,
                                const Spectrum& fallback = Spectrum{ 0, 0, 0 }) {
  if (values.size() < 3) {
    return fallback;
  }

  Spectrum color{ values[0], values[1], values[2] };
  if (std::max({ color.r, color.g, color.b }) > 1.F) {
    color = color / Background::max_channel_value;
  }

  color.r = clamp(color.r, 0.F, 1.F);
  color.g = clamp(color.g, 0.F, 1.F);
  color.b = clamp(color.b, 0.F, 1.F);
  return color;
}

}  // namespace

BackgroundSingleColor::BackgroundSingleColor(const Color24& color, mapping_t mt)
    : Background{ mt } {
  m_single_color.r = color.r / max_channel_value;
  m_single_color.g = color.g / max_channel_value;
  m_single_color.b = color.b / max_channel_value;
}

BackgroundSingleColor::BackgroundSingleColor(const Spectrum& color, mapping_t mt)
    : Background{ mt }, m_single_color{ color } {}

Spectrum BackgroundMultiColor::lerp(const Spectrum& S, const Spectrum& E, float t) {
  // Color in float, so we can interpolate.
  const Spectrum& sf{ S };  // Start
  const Spectrum& ef{ E };  // End
  // the interpolation
  Spectrum r;
  r.x = ((1.F - t) * sf.x) + (t * ef.x);
  r.y = ((1.F - t) * sf.y) + (t * ef.y);
  r.z = ((1.F - t) * sf.z) + (t * ef.z);
  // Make sure we have a regular color.
  r.x = clamp(r.x, 0.F, 1.F);
  r.y = clamp(r.y, 0.F, 1.F);
  r.z = clamp(r.z, 0.F, 1.F);
  return r;
}

Spectrum BackgroundSingleColor::sampleUV(real_type u, real_type v) const { return m_single_color; }

Spectrum BackgroundMultiColor::sampleUV(real_type u, real_type v) const {
  // Ok, now we got the (u,v) coordinate from the mapping process.
  // interpolate horizontally first.
  Spectrum bottom = BackgroundMultiColor::lerp(m_corners[bl], m_corners[br], u);
  Spectrum top = BackgroundMultiColor::lerp(m_corners[tl], m_corners[tr], u);
  //  now, interpolate vertically, based on the (interpolated) colors from previous step.
  return BackgroundMultiColor::lerp(top, bottom, v);
}

Background* create_color_background(std::string_view type, const ParamSet& ps) {
  // List of name ids for each corner of the background.
  std::array<std::string, 4> corner_name{ "tl", "bl", "br", "tr" };

  // Possible colored background types: single_color, 4_colors.
  if (type == "single_color") {
    // The tag:
    // <background type="single_color" color="153 204 255"/>
    auto color_values = ps.retrieve<std::vector<real_type>>("color", { 0, 0, 0 });
    return new BackgroundSingleColor(rgb_values_to_spectrum(color_values));
  }
  if (type == "4_colors" or type == "colors") {
    // List of color from the scene to be passed onto the constructor.
    std::array<Spectrum, 4> color_list;
    // The tag:
    // <background type="4_colors"  bl="0 0 51" tl="0 255 51" tr="255 255 51" br="255 0 51" />
    size_t idx{ 0 };
    for (const auto& label : corner_name) {
      Color24 color_24{ ps.retrieve<Color24>(label, black) };
      color_list[idx++] = Spectrum{ color_24.r / Background::max_channel_value,
                                    color_24.g / Background::max_channel_value,
                                    color_24.b / Background::max_channel_value };
    }
    return new BackgroundMultiColor(color_list);
  }
  // If we got here it means we received an invalid colored background specification.
  std::ostringstream oss;
  oss << "create_color_background(): Unknown type of colored background specified "
      << std::quoted(type) << ", using black background.";
  WARNING(oss.str());
  return new BackgroundSingleColor(black);
}

}  // namespace gc
