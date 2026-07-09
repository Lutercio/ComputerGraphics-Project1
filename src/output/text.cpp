#include "output/text.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>

#include "diagnostics/error.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace gc {

namespace {

constexpr int kSS = 4;  // supersampling factor

std::vector<unsigned char> read_file_bytes(const std::string& path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (not file) {
    return {};
  }
  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<unsigned char> data(static_cast<std::size_t>(size));
  if (not file.read(reinterpret_cast<char*>(data.data()), size)) {
    return {};
  }
  return data;
}

}  // namespace

void draw_caption_bottom_right(std::vector<Spectrum>& buffer,
                               int width,
                               int height,
                               const std::string& text,
                               const std::string& font_path,
                               real_type pixel_height,
                               const Spectrum& color,
                               int margin_x,
                               int margin_y,
                               real_type slant) {
  if (text.empty() or width <= 0 or height <= 0) {
    return;
  }

  const auto ttf = read_file_bytes(font_path);
  if (ttf.empty()) {
    WARNING(std::string{ "Caption font not found: \"" } + font_path + "\"; skipping text.");
    return;
  }

  stbtt_fontinfo font;
  if (stbtt_InitFont(&font, ttf.data(), stbtt_GetFontOffsetForIndex(ttf.data(), 0)) == 0) {
    WARNING(std::string{ "Failed to parse font: \"" } + font_path + "\"; skipping text.");
    return;
  }

  const float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(pixel_height) * kSS);
  int ascent = 0;
  int descent = 0;
  int line_gap = 0;
  stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
  const int ascent_ss = static_cast<int>(std::lround(ascent * scale));
  const int descent_ss = static_cast<int>(std::lround(-descent * scale));

  int total_advance = 0;
  for (std::size_t i = 0; i < text.size(); ++i) {
    int advance = 0;
    int lsb = 0;
    stbtt_GetCodepointHMetrics(&font, text[i], &advance, &lsb);
    total_advance += advance;
    if (i + 1 < text.size()) {
      total_advance += stbtt_GetCodepointKernAdvance(&font, text[i], text[i + 1]);
    }
  }
  const int line_ss = static_cast<int>(std::lround(total_advance * scale));

  const int pad_ss = kSS * 2;
  const int slant_pad_ss = static_cast<int>(std::lround(slant * (ascent_ss + descent_ss)));

  const int cov_w = line_ss + slant_pad_ss + (2 * pad_ss);
  const int cov_h = ascent_ss + descent_ss + (2 * pad_ss);
  if (cov_w <= 0 or cov_h <= 0) {
    return;
  }
  std::vector<float> cov(static_cast<std::size_t>(cov_w) * cov_h, 0.F);

  const int baseline_ss = pad_ss + ascent_ss;
  int cursor_ss = pad_ss;

  for (std::size_t i = 0; i < text.size(); ++i) {
    int gx0 = 0;
    int gy0 = 0;
    int gx1 = 0;
    int gy1 = 0;
    stbtt_GetCodepointBitmapBox(&font, text[i], scale, scale, &gx0, &gy0, &gx1, &gy1);
    const int gw = gx1 - gx0;
    const int gh = gy1 - gy0;

    if (gw > 0 and gh > 0) {
      std::vector<unsigned char> glyph(static_cast<std::size_t>(gw) * gh);
      stbtt_MakeCodepointBitmap(&font, glyph.data(), gw, gh, gw, scale, scale, text[i]);
      for (int gy = 0; gy < gh; ++gy) {
        const int shear = static_cast<int>(std::lround(slant * (gh - gy)));  // synthetic italic
        const int cy = baseline_ss + gy0 + gy;
        if (cy < 0 or cy >= cov_h) {
          continue;
        }
        for (int gx = 0; gx < gw; ++gx) {
          const float a = static_cast<float>(glyph[(gy * gw) + gx]) / 255.F;
          if (a <= 0.F) {
            continue;
          }
          const int cx = cursor_ss + gx0 + gx + shear;
          if (cx < 0 or cx >= cov_w) {
            continue;
          }
          float& c = cov[(static_cast<std::size_t>(cy) * cov_w) + cx];
          c = std::max(c, a);
        }
      }
    }

    int advance = 0;
    int lsb = 0;
    stbtt_GetCodepointHMetrics(&font, text[i], &advance, &lsb);
    cursor_ss += static_cast<int>(std::lround(advance * scale));
    if (i + 1 < text.size()) {
      cursor_ss += static_cast<int>(
        std::lround(stbtt_GetCodepointKernAdvance(&font, text[i], text[i + 1]) * scale));
    }
  }

  const int box_w = cov_w / kSS;
  const int box_h = cov_h / kSS;
  std::vector<float> alpha(static_cast<std::size_t>(box_w) * box_h, 0.F);
  const float inv_samples = 1.F / static_cast<float>(kSS * kSS);
  for (int by = 0; by < box_h; ++by) {
    for (int bx = 0; bx < box_w; ++bx) {
      float sum = 0.F;
      for (int sy = 0; sy < kSS; ++sy) {
        const int cy = (by * kSS) + sy;
        for (int sx = 0; sx < kSS; ++sx) {
          sum += cov[(static_cast<std::size_t>(cy) * cov_w) + (bx * kSS) + sx];
        }
      }
      alpha[(static_cast<std::size_t>(by) * box_w) + bx] = sum * inv_samples;
    }
  }

  const int box_x0 = width - margin_x - box_w;
  const int box_y0 = height - margin_y - box_h;

  auto blend = [&](int fx, int fy, float a, const Spectrum& col) {
    if (a <= 0.F or fx < 0 or fx >= width or fy < 0 or fy >= height) {
      return;
    }
    Spectrum& dst = buffer[(static_cast<std::size_t>(fy) * width) + fx];
    dst = (dst * (1.F - a)) + (col * a);
  };

  const int shadow_dx = 1;
  const int shadow_dy = 1;
  const Spectrum shadow{ 0.F, 0.F, 0.F };
  for (int by = 0; by < box_h; ++by) {
    for (int bx = 0; bx < box_w; ++bx) {
      const float a = alpha[(static_cast<std::size_t>(by) * box_w) + bx];
      blend(box_x0 + bx + shadow_dx, box_y0 + by + shadow_dy, a * 0.7F, shadow);
    }
  }
  for (int by = 0; by < box_h; ++by) {
    for (int bx = 0; bx < box_w; ++bx) {
      const float a = alpha[(static_cast<std::size_t>(by) * box_w) + bx];
      blend(box_x0 + bx, box_y0 + by, a, color);
    }
  }
}

}  // namespace gc
