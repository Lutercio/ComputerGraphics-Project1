#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <string_view>
#include <vector>

#include <glm/trigonometric.hpp>
#include <glm/ext/vector_float3.hpp>

#include "../msg_system/error.hpp"

#include "sphere.hpp"
#include "app.hpp"
#include "background.hpp"
#include "common.hpp"
#include "film.hpp"
#include "integrator.hpp"
#include "material.hpp"
#include "paramset.hpp"
#include "parser.hpp"

namespace gc {

namespace {

ParamSet get_actor(const std::string& name) {
  auto it = App::m_render_options->actors.find(name);
  if (it != App::m_render_options->actors.end()) {
    return it->second;
  }
  return {};
}

std::vector<float> retrieve_vector(const ParamSet& ps,
                                   const std::string& key,
                                   const std::vector<float>& fallback)
{
  auto values = ps.retrieve<std::vector<float>>(key, fallback);
  return values.size() >= fallback.size() ? values : fallback;
}

std::shared_ptr<Material> make_material(const ParamSet& ps)
{
  auto type = ps.retrieve<std::string>("type", "flat");
  if (type == "flat") {
    auto color_values = ps.retrieve<std::vector<float>>("color", {1.F, 1.F, 1.F});
    auto color = rgb_values_to_spectrum(color_values);
    return std::make_shared<FlatMaterial>(color);
  }

  WARNING(std::string{"Unknown material type \""} + type + "\"; using flat white.");
  return std::make_shared<FlatMaterial>(Spectrum{1, 1, 1});
}

bool prepare_render_resources()
{
  ParamSet film_ps = get_actor("film");
  if (not film_ps.contains<std::string>("type")) {
    film_ps.assign("type", std::string{"image"});
  }

  Film* film = App::make_film(film_ps);
  if (film == nullptr) {
    ERROR("App::prepare_render_resources(): Unable to create film.");
    return false;
  }
  App::m_render_options->film.reset(film);

  auto lookat_ps = get_actor("lookat");
  auto camera_ps = get_actor("camera");

  auto look_from_v = retrieve_vector(lookat_ps, "look_from", {0.F, 0.F, 0.F});
  auto look_at_v = retrieve_vector(lookat_ps, "look_at", {0.F, 0.F, 1.F});
  auto up_v = retrieve_vector(lookat_ps, "up", {0.F, 1.F, 0.F});

  Point3f look_from(look_from_v[0], look_from_v[1], look_from_v[2]);
  Point3f look_at_pt(look_at_v[0], look_at_v[1], look_at_v[2]);
  Vector3f up(up_v[0], up_v[1], up_v[2]);

  auto camera_type = camera_ps.retrieve<std::string>("type", "orthographic");

  if (camera_type == "orthographic") {
    auto sw = retrieve_vector(camera_ps, "screen_window", {-1.F, 1.F, -1.F, 1.F});
    App::m_render_options->camera = std::make_unique<OrthographicCamera>(
        look_from, look_at_pt, up, sw[0], sw[1], sw[2], sw[3]);
  } else if (camera_type == "perspective") {
    auto fovy = camera_ps.retrieve<float>("fovy", 60.F);
    auto aspect = float(App::m_render_options->film->get_resolution().x)
                  / float(App::m_render_options->film->get_resolution().y);
    App::m_render_options->camera =
        std::make_unique<PerspectiveCamera>(look_from, look_at_pt, up, fovy, aspect);
  } else {
    WARNING(std::string{"Unknown camera type \""} + camera_type + "\"; using orthographic.");
    App::m_render_options->camera =
        std::make_unique<OrthographicCamera>(look_from, look_at_pt, up, -1.F, 1.F, -1.F, 1.F);
  }

  return App::m_render_options->camera != nullptr;
}

}  // namespace

//=== App's static members declaration and initialization.
App::AppState App::m_current_block_state = AppState::Uninitialized;
RunningOptions App::m_current_run_options;
std::unique_ptr<RenderOptions> App::m_render_options;
GraphicsState App::m_graphics_state;

/// Check whether the current state has been intialized.
bool App::check_in_initialized_state(std::string_view func_name) {
  if (m_current_block_state == AppState::Uninitialized) {
    std::ostringstream oss;
    oss << "App::init() must be called before " << func_name << ". Ignoring...";
    ERROR(oss.str());
    return false;
  }
  return true;
}

/// Check whether the current state corresponds to setup section.
bool App::check_in_setup_block_state(std::string_view func_name) {
  check_in_initialized_state(func_name);
  if (m_current_block_state == AppState::WorldBlock) {
    std::ostringstream oss;
    oss << "Rendering setup cannot happen inside World Definition block; ";
    oss << func_name << " not allowed. Ignoring...";
    ERROR(oss.str());
    return false;
  }
  return true;
}

/// Check whether the current state corresponds to the world section.
bool App::check_in_world_block_state(std::string_view func_name) {
  check_in_initialized_state(func_name);
  if (m_current_block_state == AppState::SetupBlock) {
    std::ostringstream oss;
    oss << "Scene description must happen inside World Definition block; ";
    oss << func_name << " not allowed. Ignoring...";
    ERROR(oss.str());
    return false;
  }
  return true;
}

//=== App's public methods implementation
void App::init_engine(const RunningOptions& run_options) {
  // Save running option sent from the main().
  m_current_run_options = run_options;
  // Check current machine state.
  if (m_current_block_state != AppState::Uninitialized) {
    ERROR("App::init_engine() has already been called! ");
  }
  // Set proper machine state
  m_current_block_state = AppState::SetupBlock;
  // Preprare render infrastructure for a new scene.
  m_render_options = std::make_unique<RenderOptions>();
  // Create a new initial GS
  // m_current_gs = GraphicsState();
  MESSAGE("[1] Rendering engine initiated.\n");
}

void App::run() {
  // Try to load and parse the scene from a file.
  MESSAGE("[2] Beginning scene file parsing...\n");
  // Recall that the file name comes from the running option struct.
  parse_scene_file(m_current_run_options.filename.c_str());
}

void App::world_begin(const ParamSet& ps) {
  if (not check_in_setup_block_state("App::world_begin()")) {
    return;
  }
  m_current_block_state = AppState::WorldBlock;  // correct machine state.
  hard_engine_reset();
}

/// Erase temporary engine states so that we may render another scene with the same configuration.
void App::hard_engine_reset() {
  m_render_options->background.reset();
  m_render_options->primitives.clear();
  m_graphics_state = GraphicsState();
}

void App::world_end(const ParamSet& ps) {
  MESSAGE("====================================================================");
  MESSAGE("   Parsing Phase has ended. Rendering process starts now...");
  MESSAGE("====================================================================");

  if (not check_in_world_block_state("App::world_end()")) {
    return;
  }

  bool scene_and_integrator_ok = prepare_render_resources();

  // The scene has already been parsed and properly set up. It's time to render the scene.
  // [1] Create the integrator.
  // [2] Create the scene.
  // [3] Run integrator if previous instantiations went ok
  if (scene_and_integrator_ok) {
    MESSAGE("    Parsing scene successfuly done!\n");
    MESSAGE("[2] Starting ray tracing progress.\n");
    MESSAGE("    Ray tracing is usually a slow process, please be patient: \n");
    //================================================================================
    auto start = std::chrono::steady_clock::now();
    // m_integrator->render(*m_scene);
    render();
    auto end = std::chrono::steady_clock::now();
    //================================================================================
    auto diff = end - start;  // Store the time difference between start and end
    // Seconds
    auto diff_sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    MESSAGE("    Time elapsed: " + std::to_string(diff_sec.count()) + " seconds ("
            + std::to_string(std::chrono::duration<double, std::milli>(diff).count()) + " ms) \n");
  }
  // [4] Basic clean up, preparing for new rendering, in case we have
  // several scene setup + world in a single input scene file.
  m_current_block_state = AppState::SetupBlock;  // correct machine state.
}

void App::film(const ParamSet& ps) {
  if (not check_in_initialized_state("App::film()")) {
    return;
  }
  // Store the ps associated with camera for later retrieval.
  m_render_options->actors["film"] = ps;
  if (m_current_run_options.verbose) {
    auto type = ps.retrieve<std::string>("type", "unknown");
    std::cout << ">>> film type: " << std::quoted(type) << '\n';
  }
}

void App::background(const ParamSet& ps) {
  if (not check_in_world_block_state("App::background")) {
    return;
  }

  auto type = ps.retrieve<std::string>("type", "unknown");
  if (type == "unknown") {
    ERROR("API::background(): Missing \"type\" specificaton for the background.");
  }
  Background* bkg{ nullptr };
  if (type == "single_color" or type == "4_colors" or type == "colors") {
    bkg = create_color_background(type, ps);
  } else {
    WARNING(std::string{ "API::background(): unknown background type \"" } + type
            + std::string{ "\" provided; assuming colored background." });
    bkg = create_color_background(type, ps);
  }
  // Store current background objec.
  m_render_options->background.reset(bkg);
}

void App::render() {
  if (m_render_options->background == nullptr) {
    m_render_options->background = std::make_unique<BackgroundSingleColor>(Spectrum{0, 0, 0});
  }

  Scene scene(m_render_options->camera.get(),
              m_render_options->background.get(),
              m_render_options->film.get(),
              m_render_options->primitives);

  auto int_ps = m_render_options->actors["integrator"];
  std::unique_ptr<Integrator> integrator = create_integrator(int_ps);

  if (integrator) {
    integrator->render(scene);
  }
}

Film* App::make_film(const ParamSet& ps) {
  Film* film{ nullptr };
  auto film_type = ps.retrieve<std::string>("type", "image");
  if (film_type == "image") {
    film = create_film(ps);
  } else {
    WARNING(std::string{ "Film \"" } + film_type + std::string{ "\" unknown." });
  }
  return film;
}

void App::integrator(const ParamSet& ps) {
  if (not check_in_initialized_state("App::integrator()")) {
    return;
  }
  m_render_options->actors["integrator"] = ps;
}

void App::aggregator(const ParamSet& ps) {
  if (not check_in_initialized_state("App::aggregator()")) {
    return;
  }
  m_render_options->actors["aggregator"] = ps;
}

void App::object(const ParamSet& ps) {
  if (not check_in_world_block_state("App::object()")) {
    return;
  }

  auto type = ps.retrieve<std::string>("type", "unknown");
  if (type == "sphere") {
    auto center_v = retrieve_vector(ps, "center", {0.F, 0.F, 0.F});
    auto radius = ps.retrieve<float>("radius", 1.F);
    Point3f center(center_v[0], center_v[1], center_v[2]);

    std::shared_ptr<Material> material = m_graphics_state.get_current_material();
    if (ps.contains<std::string>("material")) {
      auto material_name = ps.retrieve<std::string>("material");
      auto named_material = m_graphics_state.find_material(material_name);
      if (named_material != nullptr) {
        material = named_material;
      } else {
        WARNING(std::string{"Material \""} + material_name
                + "\" not found; using current material.");
      }
    }

    auto sphere = std::make_shared<Sphere>(center, radius, material);
    m_render_options->primitives.push_back(sphere);
  } else {
    WARNING(std::string{"Object type \""} + type + "\" unknown. Ignoring...");
  }

  MESSAGE("Object type: " + type);
}

void App::look_at(const ParamSet& ps) {
  if (not check_in_initialized_state("App::look_at()")) {
    return;
  }
  m_render_options->actors["lookat"] = ps;
}

void App::camera(const ParamSet& ps) {
  if (not check_in_initialized_state("App::camera()")) {
    return;
  }
  m_render_options->actors["camera"] = ps;
}

void App::material(const ParamSet& ps) {
  if (not check_in_world_block_state("App::material()")) {
    return;
  }
  m_graphics_state.set_current_material(make_material(ps));
}

void App::make_named_material(const ParamSet& ps) {
  if (not check_in_world_block_state("App::make_named_material()")) {
    return;
  }

  auto name = ps.retrieve<std::string>("name", "");
  if (name.empty()) {
    WARNING("Named material declaration missing name. Ignoring...");
    return;
  }

  m_graphics_state.define_material(name, make_material(ps));
}

void App::named_material(const ParamSet& ps) {
  if (not check_in_world_block_state("App::named_material()")) {
    return;
  }

  auto name = ps.retrieve<std::string>("name", "");
  if (name.empty() or not m_graphics_state.set_current_material(name)) {
    WARNING(std::string{"Named material \""} + name + "\" not found. Keeping current material.");
  }
}

void App::render_again(const ParamSet& ps) {
  if (not check_in_setup_block_state("App::render_again()")) {
    return;
  }

  MESSAGE("====================================================================");
  MESSAGE("   Rendering scene again with the current setup...");
  MESSAGE("====================================================================");

  if (prepare_render_resources()) {
    render();
  }
}

}  // namespace gc
