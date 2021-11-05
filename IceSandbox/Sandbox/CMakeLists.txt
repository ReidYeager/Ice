
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_executable (Sandbox
  src/source.cpp
)

target_include_directories(Sandbox PRIVATE
  ./src
  ${CMAKE_SOURCE_DIR}/Ice/src/
  ${CMAKE_SOURCE_DIR}/libraries/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_directories(Sandbox PRIVATE
  ./src
  ${CMAKE_SOURCE_DIR}/Ice/src/
  ${CMAKE_SOURCE_DIR}/libraries/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Sandbox
  Vulkan::Vulkan
)

target_link_libraries(Sandbox Ice)
