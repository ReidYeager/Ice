
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library (Ice STATIC
  src/core/application.cpp
  src/core/camera.cpp
  src/core/event.cpp
  src/core/input.cpp
  src/core/memory_manager.cpp
  src/platform/file_system.cpp
  src/platform/platform_windows.cpp
  src/renderer/buffer.cpp
  src/renderer/renderer.cpp
  src/renderer/vulkan/vulkan_backend.cpp
  src/renderer/vulkan/vulkan_buffer.cpp
  src/renderer/vulkan/vulkan_material.cpp
  src/renderer/vulkan/vulkan_platform.cpp
)

add_compile_definitions(Ice _DEBUG ICE_DLL_EXPORT)

target_include_directories(Ice PRIVATE
  ./src/
  ${CMAKE_SOURCE_DIR}/libraries/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_directories(Ice PRIVATE
  ./src/
  ${CMAKE_SOURCE_DIR}/libraries/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Ice
  Vulkan::Vulkan
)

set_target_properties(Ice PROPERTIES
  PUBLIC_HEADER ice.h
)
