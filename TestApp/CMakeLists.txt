
cmake_minimum_required (VERSION 3.8)

project ("TestApp")

find_package(Vulkan REQUIRED)

add_compile_definitions(SOURCE_DIRECTORY=${CMAKE_SOURCE_DIR})

add_executable (TestApp
  "src/source.cpp"

)

target_include_directories(TestApp PUBLIC
  ./src/
  #./libraries/include/
  ../Ice/src/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(TestApp
  Vulkan::Vulkan
  Ice
)
