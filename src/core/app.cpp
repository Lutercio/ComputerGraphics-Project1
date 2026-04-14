#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <string_view>

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
  check_in_setup_block_state("App::world_begin()");
  m_current_block_state = AppState::WorldBlock;  // correct machine state.
  hard_engine_reset();
}

/// Erase temporary engine states so that we may render another scene with the same configuration.
void App::hard_engine_reset() {
  // Render options reset
  // TODO: in the future.
  // m_render_options.reset();
}

void App::world_end(const ParamSet& ps) {
  MESSAGE("====================================================================");
  MESSAGE("   Parsing Phase has ended. Rendering process starts now...");
  MESSAGE("====================================================================");

  check_in_world_block_state("App::world_end()");

  // ===============================================================
  // 1) Create the integrator.
  // 2) Create the scene (requires the list of objects and background)
  // ===============================================================
  // For now, we create the film here but in the future it will be
  // instantiated somewhere else.
  Film* film = make_film(m_render_options->actors["film"]);
  if (film == nullptr) {
    ERROR("App::setup_camera(): Unable to create film.");
  }
  m_render_options->film.reset(film);

  // Cria a câmera
  auto lookat_ps = m_render_options->actors["lookat"];
  auto camera_ps = m_render_options->actors["camera"];

  auto look_from_v = lookat_ps.retrieve<std::vector<float>>("look_from", {});
  auto look_at_v   = lookat_ps.retrieve<std::vector<float>>("look_at", {});
  auto up_v        = lookat_ps.retrieve<std::vector<float>>("up", {});

  Point3f look_from(look_from_v[0], look_from_v[1], look_from_v[2]);
  Point3f look_at_pt(look_at_v[0], look_at_v[1], look_at_v[2]);
  Vector3f up(up_v[0], up_v[1], up_v[2]);

  auto camera_type = camera_ps.retrieve<std::string>("type", "orthographic");

  if (camera_type == "orthographic") {
      auto sw = camera_ps.retrieve<std::vector<float>>("screen_window", {});
      m_render_options->camera = std::make_unique<OrthographicCamera>(
          look_from, look_at_pt, up, sw[0], sw[1], sw[2], sw[3]
      );
  } else if (camera_type == "perspective") {
      auto fovy = camera_ps.retrieve<float>("fovy", 60.f);
      auto aspect = float(m_render_options->film->get_resolution().x) /
                    float(m_render_options->film->get_resolution().y);
      m_render_options->camera = std::make_unique<PerspectiveCamera>(
          look_from, look_at_pt, up, fovy, aspect
      );
  }

  // The scene has already been parsed and properly set up. It's time to render the scene.
  // [1] Create the integrator.
  // [2] Create the scene.
  // [3] Run integrator if previous instantiations went ok
  bool scene_and_integrator_ok{ true };  // THIS is a STUB.
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
  if (not check_in_setup_block_state("App::film()")) {
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
  check_in_world_block_state("App::background");

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
    Scene scene(
        m_render_options->camera.get(),
        m_render_options->background.get(),
        m_render_options->film.get(),
        m_render_options->primitives
    );

    auto int_ps   = m_render_options->actors["integrator"];
    auto int_type = int_ps.retrieve<std::string>("type", "flat");

    std::unique_ptr<Integrator> integrator;
    if (int_type == "flat")
        integrator = std::make_unique<FlatIntegrator>();

    if (integrator)
        integrator->render(scene);
}

Film* App::make_film(const ParamSet& ps) {
  Film* film{ nullptr };
  auto film_type = ps.retrieve<std::string>("type");
  if (film_type == "image") {
    film = create_film(ps);
  } else {
    WARNING(std::string{ "Film \"" } + film_type + std::string{ "\" unknown." });
  }
  return film;
}

void App::integrator(const ParamSet& ps) {
    // TODO: criar integrador
    m_render_options->actors["integrator"] = ps;
}

void App::aggregator(const ParamSet& ps) {
    // TODO: criar agregador
    m_render_options->actors["aggregator"] = ps;
}

void App::object(const ParamSet& ps) {
    auto type = ps.retrieve<std::string>("type", "unknown");
    if (type == "sphere") {
        auto center_v = ps.retrieve<std::vector<float>>("center", {});
        auto radius   = ps.retrieve<float>("radius", 1.f);
        Point3f center(center_v[0], center_v[1], center_v[2]);
        auto sphere = std::make_shared<Sphere>(center, radius, m_graphics_state.current_material);
        m_render_options->primitives.push_back(sphere);
    }

    MESSAGE("Object type: " + type);

}

void App::look_at(const ParamSet& ps) {
    m_render_options->actors["lookat"] = ps;
}

void App::camera(const ParamSet& ps) {
    m_render_options->actors["camera"] = ps;
}

void App::material(const ParamSet& ps) {
    auto type = ps.retrieve<std::string>("type", "flat");
    if (type == "flat") {
        auto cv = ps.retrieve<std::vector<float>>("color", {1.f, 0.f, 0.f});
        m_graphics_state.current_material =
            std::make_shared<FlatMaterial>(Spectrum{cv[0], cv[1], cv[2]});
    }
}

}  // namespace gc
