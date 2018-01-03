# cmake version
# XXX what version do I actually need?
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# project name
project(spectrum)

# version
set(spectrum_VERSION_MAJOR 0)
set(spectrum_VERSION_MINOR 3)

# directories XXX what does this do exactly?
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# search for include files rooted at the project directory
include_directories(${CMAKE_SOURCE_DIR})

# flags for all build types
# XXX these are gcc specific!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --pedantic -Wno-long-long")

# add profiling options if asked for
# XXX these are gcc specific!
if (PROFILING)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_LINK_FLAGS "{$CMAKE_LINK_FLAGS} -coverage")
endif()

# find boost libraries
# XXX which version do I actually need?
find_package(Boost 1.36 REQUIRED COMPONENTS date_time system thread)

# find SDL and OpenGL
find_package(SDL)
find_package(OpenGL)
find_package(PkgConfig REQUIRED)
pkg_search_module(PA REQUIRED portaudio-2.0)
pkg_search_module(FFTWF REQUIRED fftw3f)

# add relevant paths
# XXX why no libdirs for SDL and OpenGL?
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SDL_INCLUDE_DIR})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${PA_INCLUDE_DIRS})
include_directories(${FFTWF_INCLUDE_DIRS})

# add subdirectories
add_subdirectory(display)
add_subdirectory(input)
add_subdirectory(interface)
add_subdirectory(processor)
add_subdirectory(sdl)
add_subdirectory(glutils)
add_subdirectory(utils)
add_subdirectory(animation)
add_subdirectory(tests)
