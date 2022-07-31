
cmake_minimum_required (VERSION 3.16)

project ("IceEngine")

add_compile_definitions(SOURCE_DIRECTORY=${CMAKE_SOURCE_DIR})

add_subdirectory("Ice")
add_subdirectory("TestApp")
