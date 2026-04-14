#ifndef APP_HPP
#define APP_HPP

#include <cstdint>
#include <memory>
#include "background.hpp"
#include <unordered_map>

#include "common.hpp"
#include "paramset.hpp"
#include "camera.hpp"
#include "primitive.hpp"

// Type of map we want to use.
#define Dictionary std::unordered_map

namespace gc {
// Collection of objects and diretives that control rendering,
// such as camera, lights, prims.
struct RenderOptions {
  /// This is a map of attribute names (keys) and paramset (values).
  Dictionary<std::string, gc::ParamSet> actors;
  /// Background object
  std::unique_ptr<Background> background;
  // NOTE: The film object will be owned by Camera in the future
  /// Film object
  std::unique_ptr<Film> film;
  std::unique_ptr<Camera> camera;
  std::vector<std::shared_ptr<Primitive>> primitives;  
};

/*!
 * The Graphics State comprises the current material, lib of material, and flip normals.
 * If affects all incoming elements of the scene and may be saved on a stack.
 * It represents a transient (current) state that affects all graphics.
 * --------------------------------------------------------------------------------
 * I'm following the same approach described in the PBRT code: we only effectively
 * copy a lib (texture, material, or otherwise), if, on the new GS, the scene file
 * defines a new named element (material, texture, etc.). In that case, we evaluate
 * if the clone flag is off, which means we need to actually copy the lib before
 * inserting the new element and then turn the flag on. They call this approach
 * "copy on write", which means: "We only actually copy the dictionary to the new GS
 * context if the client defines a new element on the current GS context level."
 * --------------------------------------------------------------------------------
 */
struct GraphicsState {
  // TODO

  using MaterialLib = std::map<std::string, std::shared_ptr<Material>>;
  std::shared_ptr<Material> current_material; 
  std::string current_material_name;
  bool flip_normals;
  std::shared_ptr<MaterialLib> material_lib;
  
  GraphicsState()
      : current_material(nullptr)
      , flip_normals(false)
      , material_lib(std::make_shared<MaterialLib>()) {}

  std::shared_ptr<Material> get_current_material() const {
      if (current_material_name.empty()) return nullptr;
      auto it = material_lib->find(current_material_name);
      return (it != material_lib->end()) ? it->second : nullptr;
  }    
  
  void define_material(const std::string& name, std::shared_ptr<Material> mat) {
      ensure_material_lib_owned();
      (*material_lib)[name] = std::move(mat);
  }

  bool set_current_material(const std::string& name) {
      auto it = material_lib->find(name);
      if (it == material_lib->end()) return false;
      current_material = it->second;
      return true;
  }


  private:
  
  void ensure_material_lib_owned() {
      if (material_lib.use_count() > 1)
          material_lib = std::make_shared<MaterialLib>(*material_lib);
  }
};

/*!
 * Application class that represents our graphics library (GL) or API.
 *
 * This GL works in **retained mode**.
 * In this mode the client calls do not directly cause actual rendering.
 * Instead, it update an abstract internal model (typically a list of objects, camera, lights, etc.)
 * which is maintained within the library's data space. This allows the library to optimize when
 * actual rendering takes place along with the processing of related objects.
 *
 * Therefore, the client calls into the graphics library do not directly cause actual rendering, but
 * make use of extensive indirection to resources, managed – thus retained – by the graphics
 * library.
 *
 * Only after the whole scene file is processed the GL triggers the actual rendering.
 */
class App {
public:
  /// Defines the current state the API may be at a given time
  enum class AppState : std::uint8_t {
    Uninitialized,  //!< Initial state, before parsing.
    SetupBlock,     //!< Parsing render setup.
    WorldBlock
  };  //!< Parsing the world's information.

  // == Public interface
  static void render();
  static void init_engine(const RunningOptions&);
  static void run();
  static void clean_up();
  static void soft_engine_reset();
  static void hard_engine_reset();
  static void reset_engine();

  // == API functions

  static bool check_in_initialized_state(std::string_view func_name);
  static bool check_in_setup_block_state(std::string_view func_name);
  static bool check_in_world_block_state(std::string_view func_name);

  static void camera(const ParamSet& ps);
  static void look_at(const ParamSet& ps);
  static void background(const ParamSet& ps);
  static void material(const ParamSet& ps);
  static void world_begin(const ParamSet& ps);
  static void world_end(const ParamSet& ps);
  static void film(const ParamSet& ps);
  static Film* make_film(const ParamSet&);
  static void integrator(const ParamSet& ps);
  static void aggregator(const ParamSet& ps);
  static void object(const ParamSet& ps);

  /// Stores the running options passed to the main().
  static RunningOptions m_current_run_options;
  /// The infrastructure to render a scene (camera, integrator, etc.).
  static std::unique_ptr<RenderOptions> m_render_options;
  /// Current API state
  static AppState m_current_block_state;
  /// Current graphics state (active material, etc.)
  static GraphicsState m_graphics_state;
};
}  // namespace gc
#endif  // APP_HPP
