cmake_minimum_required(VERSION 3.22)
project (RT3 VERSION 1.0.0 LANGUAGES CXX )

#=== CREATING COMPILING DATABASE ===#
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#=== SETTING VARIABLES ===#
# Appending to existing flags the correct way (two methods)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fstandalone-debug")
# string(APPEND CMAKE_CXX_FLAGS " -Wall -Werror")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -fstandalone-debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
# Use this to define the symbol DEBUG
# add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>")

#=== ADDING SUBDIRECTORIES ===#
set( LIB_TINYXML2 "tinyxml2" )
add_subdirectory(lib_tinyxml2) # This will ask this lib to be build
set( LIB_LODEPNG "lodepng" )
add_subdirectory(lib_lodepng) # This will ask this lib to be build
set ( APP_NAME "rt3")
add_subdirectory(src)
