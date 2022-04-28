
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library (Ice STATIC
  "src/ice.h"
  "src/defines.h"
  "src/logger.h"

  "src/math/vector.h"
  "src/math/matrix.h"
  "src/math/linear.h"

  "src/tools/lexer.h"
  "src/tools/lexer.cpp"

  "src/core/application.h"
  "src/core/application.cpp"
  "src/core/input.cpp"
  "src/core/input.h"
  "src/core/ecs.h"
  "src/core/scene.h"
  "src/math/transform.h"

  "src/platform/platform.h"
  "src/platform/platform_win32.cpp"

  "src/rendering/renderer.h"
  "src/rendering/renderer_defines.h"

  "src/rendering/vulkan/vulkan.h"
  "src/rendering/vulkan/vulkan.cpp"
  "src/rendering/vulkan/vulkan_buffer.cpp"
  "src/rendering/vulkan/vulkan_command.cpp"
  "src/rendering/vulkan/vulkan_defines.h"
  "src/rendering/vulkan/vulkan_image.cpp"
  "src/rendering/vulkan/vulkan_material.cpp"
  "src/rendering/vulkan/vulkan_platform.cpp"
  "src/rendering/vulkan/vulkan_renderpass.cpp"

  "src/libraries/imgui/imconfig.h"
  "src/libraries/imgui/imgui.h"
  "src/libraries/imgui/imgui.cpp"
  "src/libraries/imgui/imgui_demo.cpp"
  "src/libraries/imgui/imgui_draw.cpp"
  "src/libraries/imgui/imgui_impl_vulkan.h"
  "src/libraries/imgui/imgui_impl_vulkan.cpp"
  "src/libraries/imgui/imgui_impl_win32.h"
  "src/libraries/imgui/imgui_impl_win32.cpp"
  "src/libraries/imgui/imgui_internal.h"
  "src/libraries/imgui/imgui_tables.cpp"
  "src/libraries/imgui/imgui_widgets.cpp"
  "src/libraries/imgui/imstb_rectpack.h"
  "src/libraries/imgui/imstb_textedit.h"
  "src/libraries/imgui/imstb_truetype.h"
)

add_compile_definitions(ICE_DLL_EXPORT)

target_include_directories(Ice PUBLIC
  ./src/
  ./libraries/
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
