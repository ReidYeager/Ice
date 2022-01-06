
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library (Ice STATIC
 "src/asserts.h"
 "src/defines.h"
 "src/ice.h"
 "src/logger.h"

 "src/platform/file_system.cpp"
 "src/platform/file_system.h"
 "src/platform/platform.h"
 "src/platform/platform.cpp"

 "src/core/application.h"
 "src/core/application.cpp"
 "src/core/input.cpp"
 "src/core/input.h"
 "src/core/camera.h"
 "src/core/gui.h"
 "src/core/object.h"
 "src/core/scene.h"

 "src/math/vector.h"
 "src/math/matrix.h"
 "src/math/linear.h"

 "src/rendering/renderer.h"
 "src/rendering/renderer.cpp"
 "src/rendering/mesh.h"

 "src/rendering/vulkan/vulkan_renderer.h"
 "src/rendering/vulkan/vulkan_renderer.cpp"
 "src/rendering/vulkan/vulkan_context.h"
 "src/rendering/vulkan/vulkan_platform.cpp"
 "src/rendering/vulkan/vulkan_image.cpp"
 "src/rendering/vulkan/vulkan_material.cpp"
 "src/rendering/vulkan/vulkan_descriptors.cpp"
 "src/rendering/vulkan/vulkan_renderpass.cpp"
 "src/rendering/vulkan/vulkan_mesh.cpp"
 "src/rendering/vulkan/vulkan_command.cpp"
 "src/rendering/vulkan/vulkan_buffer.cpp"

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

add_compile_definitions(Ice _DEBUG ICE_DLL_EXPORT)

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
