add_executable(camera_regression
  camera_regression.cpp
  ${CMAKE_SOURCE_DIR}/src/scene/sphere.cpp
)

target_compile_features(camera_regression PUBLIC cxx_std_17)
target_compile_options(camera_regression PRIVATE -Wall -pedantic -O3)
target_include_directories(camera_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)

find_package(glm CONFIG REQUIRED)
target_link_libraries(camera_regression PRIVATE glm::glm)

add_test(NAME camera_regression COMMAND camera_regression)

add_executable(scene_intersection_regression
  scene_intersection_regression.cpp
  ${CMAKE_SOURCE_DIR}/src/scene/sphere.cpp
)

target_compile_features(scene_intersection_regression PUBLIC cxx_std_17)
target_compile_options(scene_intersection_regression PRIVATE -Wall -pedantic -O3)
target_include_directories(scene_intersection_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(scene_intersection_regression PRIVATE glm::glm)

add_test(NAME scene_intersection_regression COMMAND scene_intersection_regression)

add_executable(blinn_light_regression
  blinn_light_regression.cpp
  ${CMAKE_SOURCE_DIR}/src/scene/sphere.cpp
  ${CMAKE_SOURCE_DIR}/src/shading/light.cpp
)

target_compile_features(blinn_light_regression PUBLIC cxx_std_17)
target_compile_options(blinn_light_regression PRIVATE -Wall -pedantic -O3)
target_include_directories(blinn_light_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(blinn_light_regression PRIVATE glm::glm)

add_test(NAME blinn_light_regression COMMAND blinn_light_regression)
