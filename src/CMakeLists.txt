# =============[ Main App ]=============
add_executable( ${APP_NAME}
   core/app.cpp
   core/background.cpp
   core/film.cpp
   core/image_io.cpp
   core/integrator.cpp
   core/main.cpp
   core/parser.cpp
   core/ray.cpp
   core/sphere.cpp
   msg_system/error.cpp
   extras/pbar.cpp
)
# ------------------------------------------------------------

# =============[ Compile for release/debug ]=================
target_compile_features( ${APP_NAME}  PUBLIC cxx_std_17 )
# target_compile_options( ${APP_NAME} PRIVATE -Wall -pedantic -Wgnu-anonymous-struct -Wnested-anon-types -g)
target_compile_options( ${APP_NAME} PRIVATE -Wall -pedantic -O3)
# Use this to define the symbol DEBUG
target_compile_definitions(${APP_NAME} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
# ------------------------------------------------------------

# =============[ Include folders ]=================
# XML parser
target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib_tinyxml2)
# Image I/O lib
target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib_lodepng)
# Math library
target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib_cybase)
# Core folder
target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/core)
# Camera folder
# target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/camera)
# Integrator folder
# target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/integrators)
# msg system foldedr
# target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/msg_system)
# # External headers
# target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/ext)
# ------------------------------------------------------------

# =============[ Libs ]=================
find_package(glm CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE glm::glm)
target_link_libraries( ${APP_NAME} PRIVATE ${LIB_TINYXML2})
target_link_libraries( ${APP_NAME} PRIVATE ${LIB_LODEPNG})
# ------------------------------------------------------------

# =============[ App properties ]=================
# Define where the binary will be output to.
set_target_properties(${APP_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
# ------------------------------------------------------------
