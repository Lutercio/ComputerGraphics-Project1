#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../msg_system/error.hpp"
#include "app.hpp"
#include "common.hpp"
#include "film.hpp"
#include "geometry.hpp"
#include "image_io.hpp"
#include "paramset.hpp"
#include <string_view>

namespace gc {

//=== Film Method Definitions
Film::Film(const Point2i& resolution,
           const std::string& filename,
           image_type_e image_type,
           bool gamma_corrected)
    : m_full_resolution{ resolution }, m_filename{ filename },
      m_activate_gamma_correction{ gamma_corrected }, m_image_type{ image_type } {
  m_color_buffer_ptr = std::make_unique<std::vector<Spectrum>>(resolution.x * resolution.y, Spectrum{0, 0, 0});
}

Film::~Film() = default;

/// Add the Spectrum color to image. Pixel coords comes as (x,y).
void Film::add_sample(const Point2i& pixel_coord, const Spectrum& pixel_color) const {
  if (pixel_coord.x >= 0 && pixel_coord.x < m_full_resolution.x &&
      pixel_coord.y >= 0 && pixel_coord.y < m_full_resolution.y) {
    (*m_color_buffer_ptr)[pixel_coord.y * m_full_resolution.x + pixel_coord.x] = pixel_color;
  }
}

/// Convert Spectrum image information to RGB, compute final pixel values, write image.
void Film::write_image() const {
  float max_val = 0.0f;
  for (const auto& color : *m_color_buffer_ptr) {
    max_val = std::max({max_val, color.x, color.y, color.z});
  }
  float multiplier = (max_val <= 1.0f && max_val > 0.0f) ? 255.0f : 1.0f;

  std::vector<std::uint8_t> byte_buffer;
  byte_buffer.reserve(m_full_resolution.x * m_full_resolution.y * 3);
  for (const auto& color : *m_color_buffer_ptr) {
    auto r = std::clamp(color.x * multiplier, 0.0f, 255.0f);
    auto g = std::clamp(color.y * multiplier, 0.0f, 255.0f);
    auto b = std::clamp(color.z * multiplier, 0.0f, 255.0f);

    if (m_activate_gamma_correction) {
      r = std::pow(r / 255.0f, 1.0f / 2.2f) * 255.0f;
      g = std::pow(g / 255.0f, 1.0f / 2.2f) * 255.0f;
      b = std::pow(b / 255.0f, 1.0f / 2.2f) * 255.0f;
    }

    byte_buffer.push_back(static_cast<std::uint8_t>(r));
    byte_buffer.push_back(static_cast<std::uint8_t>(g));
    byte_buffer.push_back(static_cast<std::uint8_t>(b));
  }

  if (m_image_type == image_type_e::PNG) {
    save_png(byte_buffer.data(), m_full_resolution.x, m_full_resolution.y, 3, m_filename);
  } else if (m_image_type == image_type_e::PPM3) {
    save_ppm3(byte_buffer.data(), m_full_resolution.x, m_full_resolution.y, 3, m_filename);
  } else if (m_image_type == image_type_e::PPM6) {
    save_ppm6(byte_buffer.data(), m_full_resolution.x, m_full_resolution.y, 3, m_filename);
  }
}

/// Chooses the filename based on the CLI and scene file info.
std::string handles_filename(const ParamSet& ps) {
  if (!App::m_current_run_options.outfile.empty()) {
    return App::m_current_run_options.outfile;
  }
  return ps.retrieve<std::string>("filename", "unknown.png");
}

// /// Process ParamSet, extracts, validates a valid crop window.
// gc::Bounds2f handles_cropwindow(const ParamSet& ps) {
//   // TODO:
// }

/// Parses the dimensions of the film from the ParamSet.
gc::Point2i handles_dimensions(const ParamSet& ps) {
  Point2i film_dimension{ ps.retrieve<int>("w_res", ps.retrieve<int>("x_res", 1280)), 
                          ps.retrieve<int>("h_res", ps.retrieve<int>("y_res", 720)) };
  // Quick render?
  if (App::m_current_run_options.quick_render) {
    // decrease resolution.
    film_dimension.x = std::max(1, film_dimension.x / 4);
    film_dimension.y = std::max(1, film_dimension.y / 4);
  }
  return film_dimension;
}

/// Creates and returns a `Film` objected based on the `ParamSet` provided.
Film* create_film(const ParamSet& ps) {
#ifdef DEBUG
  std::cout << ">>> Inside create_film()\n";
#endif
  //==[1] Choose the filename.
  auto filename = handles_filename(ps);

  //==[2] Define the crop window information.
  // auto crop_window = handles_cropwindow(ps);

  //==[3] Retrieve film dimensions and handles quick_render option.
  Point2i dimensions = handles_dimensions(ps);

  //==[4] Retrieve film type.
  std::unordered_map<std::string, Film::image_type_e> image_type{
    { "png", Film::image_type_e::PNG },
    { "ppm3", Film::image_type_e::PPM3 },
    { "ppm6", Film::image_type_e::PPM6 },
    { "ppm", Film::image_type_e::PPM6 },
  };
  auto type{ image_type[ps.retrieve<std::string>("img_type", "png")] };

  //==[5] Get gamma correction request.
  bool apply_gamma_correction = ps.retrieve<bool>("gamma_corrected", false);

#ifdef DEBUG
  std::cout << "================================================\n";
  std::cout << ">>> create_film() - film parameters are:\n";
  std::cout << "    - filename: " << std::quoted(filename) << "\n";
  std::cout << "    - crop window: " << crop_window << "\n";
  std::cout << "    - w_res: " << dimensions.x << "\n";
  std::cout << "    - h_res: " << dimensions.y << "\n";
  std::cout << "    - image type: " << ps.retrieve<std::string>("img_type", "png") << "\n";
  std::cout << "    - gamma correction: " << std::boolalpha << apply_gamma_correction << "\n";
  std::cout << "================================================\n";
#endif

  return new Film(dimensions, filename, type, apply_gamma_correction);
}
}  // namespace gc
