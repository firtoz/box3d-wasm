cmake_minimum_required(VERSION 3.22)
project(box3d_oracle C CXX)

set(BOX3D_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../box3d")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory("${BOX3D_SRC_DIR}" box3d-build)

add_executable(box3d_oracle oracle.cpp)
target_include_directories(box3d_oracle PRIVATE "${BOX3D_SRC_DIR}/include")
target_link_libraries(box3d_oracle PRIVATE box3d)
target_compile_options(box3d_oracle PRIVATE -ffp-contract=off)
set_target_properties(box3d_oracle PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Private Box3D symbols prevent interposition by the GPU C ABI in the viewer.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_library(box3d_viewer_cpu SHARED viewer_bridge.cpp)
  target_include_directories(box3d_viewer_cpu PRIVATE "${BOX3D_SRC_DIR}/include")
  target_link_libraries(box3d_viewer_cpu PRIVATE box3d)
  target_compile_options(box3d_viewer_cpu PRIVATE -ffp-contract=off)
  target_link_options(box3d_viewer_cpu PRIVATE
    "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/viewer_bridge.map"
    "LINKER:-Bsymbolic")
  set_target_properties(box3d_viewer_cpu PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
  set_property(TARGET box3d_viewer_cpu APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/viewer_bridge.map")
endif()
