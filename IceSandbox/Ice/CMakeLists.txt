
cmake_minimum_required (VERSION 3.16)

find_package(Vulkan REQUIRED)

add_library (Ice STATIC
 "src/asserts.h"
 "src/defines.h"
 "src/logger.h"
 "src/ice.h"

 "src/core/application.c"
 "src/core/application.h"
 "src/math/vectors.h"
  "src/platform/platform.h" "src/platform/platform_win32.c")

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
