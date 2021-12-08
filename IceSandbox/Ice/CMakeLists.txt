
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

 "src/math/vector.h"
 "src/math/matrix.h"
 "src/math/linear.h"

 "src/rendering/renderer.h"
 "src/rendering/renderer.cpp"
 "src/rendering/renderer_backend.h"

 "src/rendering/mesh.h"

 "src/rendering/vulkan/renderer_vulkan.h"
 "src/rendering/vulkan/renderer_vulkan.cpp"
 "src/rendering/vulkan/vulkan_context.h"
 "src/rendering/vulkan/vulkan_platform.cpp"
 "src/rendering/vulkan/vulkan_image.cpp"





  "src/rendering/vulkan/vulkan_buffer.cpp")

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
