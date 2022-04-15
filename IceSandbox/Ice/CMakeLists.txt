
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library (Ice STATIC
  
  "src/defines.h"
  "src/ice.h"
  "src/logger.h"

  "src/core/input.cpp"
  "src/core/input.h"

  "src/math/vector.h"
  "src/math/matrix.h"
  "src/math/linear.h"

#  "src/rendering/vulkan/vk_renderer.h"
#  "src/rendering/vulkan/vk_renderer.cpp"
#  "src/rendering/vulkan/vk_context.h"
#  "src/rendering/vulkan/vk_platform.cpp"
#  "src/rendering/vulkan/vk_image.cpp"
#  "src/rendering/vulkan/vk_material.cpp"
#  "src/rendering/vulkan/vk_descriptors.cpp"
#  "src/rendering/vulkan/vk_renderpass.cpp"
#  "src/rendering/vulkan/vk_mesh.cpp"
#  "src/rendering/vulkan/vk_command.cpp"
#  "src/rendering/vulkan/vk_buffer.cpp"
#  "src/rendering/vulkan/vk_deferred.cpp"
#  "src/rendering/vulkan/vk_shadow.cpp"
#  "src/rendering/vulkan/vk_forward.cpp"
#  "src/rendering/vulkan/vk_imgui.cpp"

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

  "src/tools/lexer.h"
  
  "src/core/application.h"
  "src/core/application.cpp"
  
  "src/platform/platform.h"
  "src/platform/platform_win32.cpp"
  
  "src/rendering/renderer.h"

  "src/rendering/vulkan/vulkan.h"
  "src/rendering/vulkan/vulkan.cpp"
  "src/rendering/vulkan/vulkan_context.h"
  "src/rendering/vulkan/vulkan_platform.cpp"
  "src/rendering/vulkan/vulkan_image.cpp"
  "src/rendering/vulkan/vulkan_command.cpp"
  "src/rendering/vulkan/vulkan_renderpass.cpp"
  "src/rendering/vulkan/vulkan_material.cpp"
 "src/core/ecs.h"
 "src/tools/lexer.cpp"
 "src/rendering/renderer_context.h"
 "src/rendering/vulkan/vulkan_buffer.cpp"
 "src/core/scene.h"
 "src/core/object.h")

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
