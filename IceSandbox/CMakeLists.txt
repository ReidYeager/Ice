# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.16)

project ("IceSandbox")

add_compile_definitions(SOURCE_DIRECTORY=${CMAKE_SOURCE_DIR})

# Include sub-projects.
add_subdirectory ("Ice")
add_subdirectory ("Sandbox")
