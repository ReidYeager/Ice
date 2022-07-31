
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

project("ICE")

add_compile_definitions(CMAKE_SOURCE_DIRECTORY=${ICE_SOURCE_DIR})

add_library (Ice
  # ==========
  # Base
  # ==========
  "src/ice.h"
  "src/defines.h"

  # ==========
  # Math
  # ==========
  "src/math/linear.h"
  "src/math/matrix.hpp"
  "src/math/quaternion.hpp"
  "src/math/transform.h"
  "src/math/vector.hpp"

  # ==========
  # Tools
  # ==========
  "src/tools/lexer.h"
  "src/tools/logger.h"
  "src/tools/logger.cpp"

  # ==========
  # Core
  # ==========
  "src/core/application_defines.h"
  "src/core/application.h"
  "src/core/application.cpp"

  "src/core/input.cpp"
  "src/core/input.h"

  # ==========
  # Core/Ecs
  # ==========
  "src/core/ecs/ecs.h"
  "src/core/ecs/entity.h"
  "src/core/ecs/entity.cpp"

  # ==========
  # Platform
  # ==========
  "src/platform/platform_defines.h"
  "src/platform/platform.h"
  "src/platform/platform_win32.cpp"
  "src/platform/memory_pool.h"

  # ==========
  # Rendering
  # ==========
  "src/rendering/renderer_defines.h"
  "src/rendering/renderer.h"

  # ==========
  # Rendering/Vulkan
  # ==========
  "src/rendering/vulkan/vulkan_defines.h"
  "src/rendering/vulkan/vulkan.h"
  "src/rendering/vulkan/vulkan_buffer.cpp"
  "src/rendering/vulkan/vulkan_core.cpp"
  "src/rendering/vulkan/vulkan_command.cpp"
  "src/rendering/vulkan/vulkan_image.cpp"
  "src/rendering/vulkan/vulkan_material.cpp"
  "src/rendering/vulkan/vulkan_platform.cpp"
  "src/rendering/vulkan/vulkan_renderpass.cpp"

)

set_target_properties(Ice PROPERTIES LINKER_LANGUAGE C)

add_compile_definitions(ICE_DLL_EXPORT)

target_include_directories(Ice PUBLIC
  ./src/
  ./libraries/include/
  ${Vulkan_INCLUDE_DIRS}
)

#target_link_directories(Ice PUBLIC
#  ${Vulkan_LIB_DIRS}
#)

target_link_libraries(Ice
  Vulkan::Vulkan
)

set_target_properties(Ice PROPERTIES PUBLIC_HEADER ice.h)
