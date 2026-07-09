#ifndef TEXT_HPP
#define TEXT_HPP

#include <string>
#include <vector>

#include "math/common.hpp"
#include "math/geometry.hpp"

namespace gc {

/// Draws one line of text into an RGB buffer, anchored bottom-right, using a
/// TrueType font (stb_truetype). `slant` shears the glyphs for a faux italic.
void draw_caption_bottom_right(std::vector<Spectrum>& buffer,
                               int width,
                               int height,
                               const std::string& text,
                               const std::string& font_path,
                               real_type pixel_height,
                               const Spectrum& color,
                               int margin_x,
                               int margin_y,
                               real_type slant);

}  // namespace gc

#endif  // TEXT_HPP
