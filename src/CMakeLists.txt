# =============[ Main App ]=============
add_executable( ${APP_NAME}
   diagnostics/error.cpp
   input/parser.cpp
   output/film.cpp
   output/image_io.cpp
   runtime/app.cpp
   runtime/main.cpp
   scene/background.cpp
   scene/sphere.cpp
   shading/integrator.cpp
   shading/light.cpp
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
# Project source root
target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
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
