#=== SETTING LIBRARY ===#
# add_library(${LIB_LODEPNG} SHARED lib_name.cpp)
add_library(${LIB_LODEPNG} lodepng.cpp)
set_target_properties(${LIB_LODEPNG} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER lodepng.h)
# Make header available for other targets in the project.
target_include_directories(${LIB_LODEPNG} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} .)
target_compile_features( ${LIB_LODEPNG} PRIVATE cxx_std_17 )
target_compile_options( ${LIB_LODEPNG} PRIVATE -Wall -pedantic -O3)
