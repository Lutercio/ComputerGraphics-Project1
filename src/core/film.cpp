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
  // TODO:
}

Film::~Film() = default;

/// Add the Spectrum color to image. Pixel coords comes as (x,y).
void Film::add_sample(const Point2i& pixel_coord, const Spectrum& pixel_color) const {
  // TODO:
}

/// Convert Spectrum image information to RGB, compute final pixel values, write image.
void Film::write_image() const {
  // TODO:
}

/// Chooses the filename based on the CLI and scene file info.
std::string handles_filename(const ParamSet& ps) {
  // TODO:
  return "unknown.png";  // STUB, replace it!
}

// /// Process ParamSet, extracts, validates a valid crop window.
// gc::Bounds2f handles_cropwindow(const ParamSet& ps) {
//   // TODO:
// }

/// Parses the dimensions of the film from the ParamSet.
gc::Point2i handles_dimensions(const ParamSet& ps) {
  Point2i film_dimension{ ps.retrieve<int>("w_res", 1280), ps.retrieve<int>("h_res", 720) };
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
