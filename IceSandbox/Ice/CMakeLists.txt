
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library (Ice STATIC
 "src/asserts.h"
 "src/defines.h"
 "src/ice.h"
 "src/logger.h"
 "src/profiler.h"
 "src/service_hub.h"

 "src/core/application.cpp"
 "src/core/application.h"
 "src/core/camera.cpp"
 "src/core/camera.h"
 "src/core/ecs_components.h"
 "src/core/ecs_controller.h"
 "src/core/event.cpp"
 "src/core/event.h"
 "src/core/gameobject.h"
 "src/core/input.cpp"
 "src/core/input.h"
 "src/core/memory_manager.cpp"
 "src/core/memory_manager.h"

 "src/platform/file_system.cpp"
 "src/platform/file_system.h"
 "src/platform/platform.h"
 "src/platform/platform_windows.cpp"

 "src/renderer/buffer.cpp"
 "src/renderer/buffer.h"
 "src/renderer/image.h"
 "src/renderer/material.h"
 "src/renderer/mesh.h"
 "src/renderer/renderer.cpp"
 "src/renderer/renderer.h"
 "src/renderer/renderer_backend.h"
 "src/renderer/renderer_backend_context.h"
 "src/renderer/shader.h"
 "src/renderer/shader_parameters.h"

 "src/renderer/vulkan/vulkan_backend.cpp"
 "src/renderer/vulkan/vulkan_backend.h"
 "src/renderer/vulkan/vulkan_buffer.cpp"
 "src/renderer/vulkan/vulkan_buffer.h"
 "src/renderer/vulkan/vulkan_context.h"
 "src/renderer/vulkan/vulkan_material.cpp"
 "src/renderer/vulkan/vulkan_material.h"
 "src/renderer/vulkan/vulkan_platform.cpp"
 "src/renderer/vulkan/vulkan_shader.h"

 "src/math/vector.h"
 "src/math/matrix.h"
 "src/math/linear.h"
 "src/renderer/vulkan/vulkan_descriptor.cpp"
 "src/renderer/vulkan/vulkan_descriptor.h"

 "src/core/re_application.h"
 "src/core/re_application.cpp"
 "src/platform/re_platform.h"
 "src/platform/re_platfrom.cpp"
 "src/rendering/re_renderer.h"
 "src/rendering/re_renderer.cpp"
 "src/rendering/re_renderer_backend.h"
 "src/rendering/vulkan/re_renderer_vulkan.h"
 "src/rendering/vulkan/re_renderer_vulkan.cpp"
 "src/rendering/vulkan/re_vulkan_context.h"
 "src/rendering/vulkan/re_vulkan_assert.h"
 "src/rendering/vulkan/re_vulkan_platform.cpp"
 "src/rendering/vulkan/re_vulkan_image.cpp"
)

add_compile_definitions(Ice _DEBUG ICE_DLL_EXPORT)

target_include_directories(Ice PUBLIC
  ./src/
  ${CMAKE_SOURCE_DIR}/libraries/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_directories(Ice PUBLIC
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
