# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.16)

project ("IceSandbox")

# Include sub-projects.
add_subdirectory ("Ice")
add_subdirectory ("Sandbox")
