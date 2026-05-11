add_executable(camera_regression
  camera_regression.cpp
  ${CMAKE_SOURCE_DIR}/src/core/sphere.cpp
)

target_compile_features(camera_regression PUBLIC cxx_std_17)
target_compile_options(camera_regression PRIVATE -Wall -pedantic -O3)
target_include_directories(camera_regression PRIVATE ${CMAKE_SOURCE_DIR}/src/core)

find_package(glm CONFIG REQUIRED)
target_link_libraries(camera_regression PRIVATE glm::glm)

add_test(NAME camera_regression COMMAND camera_regression)

add_executable(scene_intersection_regression
  scene_intersection_regression.cpp
  ${CMAKE_SOURCE_DIR}/src/core/sphere.cpp
)

target_compile_features(scene_intersection_regression PUBLIC cxx_std_17)
target_compile_options(scene_intersection_regression PRIVATE -Wall -pedantic -O3)
target_include_directories(scene_intersection_regression PRIVATE ${CMAKE_SOURCE_DIR}/src/core)
target_link_libraries(scene_intersection_regression PRIVATE glm::glm)

add_test(NAME scene_intersection_regression COMMAND scene_intersection_regression)
