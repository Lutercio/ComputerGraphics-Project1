# It is possible to omit the keyword defining what type of library to build. Unless the project
# specifically requires a particular type of library, the preferred practice is to not specify it and leave
# the choice up to the developer when building the project. In such cases, the library will be either
# STATIC or SHARED, with the choice determined by the value of a CMake variable called
# BUILD_SHARED_LIBS. If BUILD_SHARED_LIBS has been set to true, the library target will be a shared
# library, otherwise it will be static. Working with variables is covered in detail in Chapter 5,
# Variables, but for now, one way to set this variable is by including a -D option on the cmake

# cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=YES /path/to/source

# or

# set(BUILD_SHARED_LIBS YES)

#=== SETTING LIBRARY ===#
# add_library(${LIB_NAME} SHARED lib_name.cpp)
add_library(${LIB_TINYXML2} tinyxml2.cpp)
set_target_properties(${LIB_TINYXML2} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER tinyxml2.h)
# Make header available for other targets in the project.
target_include_directories(${LIB_TINYXML2} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} .)
target_compile_features( ${LIB_TINYXML2} PRIVATE cxx_std_17 )
target_compile_options( ${LIB_TINYXML2} PRIVATE -Wall -pedantic -O3)
