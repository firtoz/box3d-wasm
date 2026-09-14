cmake_minimum_required(VERSION 3.24)
project(box3d_native_samples VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT BOX3D_DIR)
  get_filename_component(BOX3D_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../box3d" ABSOLUTE)
endif()
if(NOT GPU_PHYSICS_DIR)
  get_filename_component(GPU_PHYSICS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
endif()

option(GPU_SAMPLES "Link the samples app against gpu-physics instead of CPU Box3D world APIs" OFF)
option(BOTH_SAMPLES "Step-lock CPU Box3D and GPU physics; compare in synchronized CPU/GPU viewports" OFF)

set(FETCHCONTENT_BASE_DIR "${BOX3D_DIR}/.fetchcontent-cache" CACHE PATH "")
include(FetchContent)

# CPU Box3D (math, hulls, defaults). GPU builds still link this; samples_api/stubs win world symbols.
add_subdirectory("${BOX3D_DIR}/src" box3d_src)
add_subdirectory("${BOX3D_DIR}/shared" box3d_shared)

FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.7
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG v0.17
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(implot)

set(NFD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(nfd
  GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
  GIT_TAG v1.3.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nfd)

add_library(imgui STATIC
  "${imgui_SOURCE_DIR}/imgui.cpp"
  "${imgui_SOURCE_DIR}/imgui_draw.cpp"
  "${imgui_SOURCE_DIR}/imgui_tables.cpp"
  "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
  "${imgui_SOURCE_DIR}/imgui_demo.cpp"
)
target_include_directories(imgui SYSTEM PUBLIC "${imgui_SOURCE_DIR}")
set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(implot STATIC
  "${implot_SOURCE_DIR}/implot.cpp"
  "${implot_SOURCE_DIR}/implot_items.cpp"
  "${implot_SOURCE_DIR}/implot_demo.cpp"
)
target_link_libraries(implot PUBLIC imgui)
target_include_directories(implot SYSTEM PUBLIC "${implot_SOURCE_DIR}")

set(sokol_SOURCE_DIR "${BOX3D_DIR}/extern/sokol")
add_library(sokol INTERFACE)
target_include_directories(sokol SYSTEM INTERFACE "${sokol_SOURCE_DIR}")
if(UNIX AND NOT APPLE)
  target_compile_definitions(sokol INTERFACE SOKOL_GLCORE)
  find_package(OpenGL REQUIRED)
  find_package(Threads REQUIRED)
  find_package(X11 REQUIRED)
  target_link_libraries(sokol INTERFACE
    OpenGL::GL Threads::Threads ${CMAKE_DL_LIBS} m
    X11::X11 X11::Xi X11::Xcursor)
endif()

set(BOX3D_BUILD_SHADERS OFF CACHE BOOL "" FORCE)
add_subdirectory("${BOX3D_DIR}/samples/shaders" box3d_shaders)
add_subdirectory("${BOX3D_DIR}/samples/gfx" box3d_gfx)

find_package(Python3 REQUIRED)
if(BOTH_SAMPLES)
  set(SPLIT_GFX_DIR "${CMAKE_CURRENT_BINARY_DIR}/split-gfx")
  execute_process(COMMAND "${Python3_EXECUTABLE}" "${GPU_PHYSICS_DIR}/scripts/inject-sokol-split.py"
    "${BOX3D_DIR}/samples/gfx" "${SPLIT_GFX_DIR}" COMMAND_ERROR_IS_FATAL ANY)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${GPU_PHYSICS_DIR}/scripts/inject-sokol-split.py")
  file(GLOB SPLIT_GFX_INPUTS CONFIGURE_DEPENDS "${BOX3D_DIR}/samples/gfx/*.c")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${SPLIT_GFX_INPUTS})
  get_target_property(GFX_SOURCES gfx SOURCES)
  set(SPLIT_GFX_SOURCES)
  foreach(SOURCE IN LISTS GFX_SOURCES)
    if(SOURCE MATCHES "\\.c$" AND NOT SOURCE STREQUAL "sokol_impl.c")
      list(APPEND SPLIT_GFX_SOURCES "${SPLIT_GFX_DIR}/${SOURCE}")
    else()
      list(APPEND SPLIT_GFX_SOURCES "${BOX3D_DIR}/samples/gfx/${SOURCE}")
    endif()
  endforeach()
  set_property(TARGET gfx PROPERTY SOURCES ${SPLIT_GFX_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/sokol_split_stream.c")
  target_include_directories(gfx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(MAIN_CPP "${CMAKE_CURRENT_BINARY_DIR}/main_bench.cpp")
set(SAMPLE_CPP "${CMAKE_CURRENT_BINARY_DIR}/sample_bench.cpp")
set(CONTINUOUS_CPP "${CMAKE_CURRENT_BINARY_DIR}/sample_continuous_bench.cpp")
set(JOINT_CPP "${CMAKE_CURRENT_BINARY_DIR}/sample_joint_bench.cpp")
set(INJECT_EXTRA)
if(GPU_SAMPLES OR BOTH_SAMPLES)
  list(APPEND INJECT_EXTRA --gpu-sidebar)
endif()
add_custom_command(
  OUTPUT "${MAIN_CPP}" "${SAMPLE_CPP}" "${CONTINUOUS_CPP}" "${JOINT_CPP}"
  COMMAND "${Python3_EXECUTABLE}" "${GPU_PHYSICS_DIR}/scripts/inject-sokol-bench.py"
    --src "${BOX3D_DIR}/samples/main.cpp"
    --dst "${MAIN_CPP}"
    --sample-src "${BOX3D_DIR}/samples/sample.cpp"
    --sample-dst "${SAMPLE_CPP}"
    --continuous-src "${BOX3D_DIR}/samples/sample_continuous.cpp"
    --continuous-dst "${CONTINUOUS_CPP}"
    --joint-src "${BOX3D_DIR}/samples/sample_joint.cpp"
    --joint-dst "${JOINT_CPP}"
    ${INJECT_EXTRA}
  DEPENDS
    "${GPU_PHYSICS_DIR}/scripts/sokol_split_injection.py"
    "${BOX3D_DIR}/samples/main.cpp"
    "${BOX3D_DIR}/samples/sample.cpp"
    "${BOX3D_DIR}/samples/sample_continuous.cpp"
    "${BOX3D_DIR}/samples/sample_joint.cpp"
    "${GPU_PHYSICS_DIR}/scripts/inject-sokol-bench.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/sokol_bench_hooks.h"
  VERBATIM
)

set(SAMPLE_FILES
  "${MAIN_CPP}"
  "${CMAKE_CURRENT_SOURCE_DIR}/sokol_bench_hooks.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/sokol_loading.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/sample_gpu_bench.cpp"
  "${BOX3D_DIR}/samples/host/camera.cpp"
  "${BOX3D_DIR}/samples/host/camera.h"
  "${BOX3D_DIR}/samples/host/gui.cpp"
  "${BOX3D_DIR}/samples/host/gui.h"
  "${BOX3D_DIR}/samples/host/imgui_impl.cpp"
  "${BOX3D_DIR}/samples/host/sokol_app_impl.c"
  "${SAMPLE_CPP}"
  "${BOX3D_DIR}/samples/sample.h"
  "${BOX3D_DIR}/samples/mesh_loader.cpp"
  "${BOX3D_DIR}/samples/mesh_loader.h"
  "${BOX3D_DIR}/samples/mover.cpp"
  "${BOX3D_DIR}/samples/mover.h"
  "${BOX3D_DIR}/samples/sample_benchmark.cpp"
  "${BOX3D_DIR}/samples/sample_bodies.cpp"
  "${BOX3D_DIR}/samples/sample_character.cpp"
  "${BOX3D_DIR}/samples/sample_collision.cpp"
  "${BOX3D_DIR}/samples/sample_compound.cpp"
  "${CONTINUOUS_CPP}"
  "${BOX3D_DIR}/samples/sample_determinism.cpp"
  "${BOX3D_DIR}/samples/sample_events.cpp"
  "${BOX3D_DIR}/samples/sample_geometry.cpp"
  "${BOX3D_DIR}/samples/sample_issues.cpp"
  "${JOINT_CPP}"
  "${BOX3D_DIR}/samples/sample_manifold.cpp"
  "${BOX3D_DIR}/samples/sample_mesh.cpp"
  "${BOX3D_DIR}/samples/sample_ragdoll.cpp"
  "${BOX3D_DIR}/samples/sample_replay.cpp"
  "${BOX3D_DIR}/samples/sample_robustness.cpp"
  "${BOX3D_DIR}/samples/sample_shapes.cpp"
  "${BOX3D_DIR}/samples/sample_stacking.cpp"
  "${BOX3D_DIR}/samples/sample_tree.cpp"
  "${BOX3D_DIR}/samples/sample_world.cpp"
)

if(BOTH_SAMPLES)
  set(SAMPLE_TARGET samples_both)
elseif(GPU_SAMPLES)
  set(SAMPLE_TARGET samples_gpu)
else()
  set(SAMPLE_TARGET samples_cpu)
endif()

add_executable(${SAMPLE_TARGET} ${SAMPLE_FILES})
target_include_directories(${SAMPLE_TARGET} PRIVATE
  "${BOX3D_DIR}/samples"
  "${BOX3D_DIR}/data"
  "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${SAMPLE_TARGET} PRIVATE gfx shared imgui implot sokol nfd)

if(GPU_SAMPLES OR BOTH_SAMPLES)
  target_sources(${SAMPLE_TARGET} PRIVATE
    "${GPU_PHYSICS_DIR}/native-samples/sample_stats.cpp")
  add_library(gpu_samples_api STATIC
    "${GPU_PHYSICS_DIR}/c_abi/samples_api.c"
    "${GPU_PHYSICS_DIR}/c_abi/samples_stubs.c"
    "${GPU_PHYSICS_DIR}/c_abi/gpu_gl_poses.c")
  target_include_directories(gpu_samples_api PRIVATE "${BOX3D_DIR}/include")
  target_compile_options(gpu_samples_api PRIVATE -w)
  set_target_properties(gpu_samples_api PROPERTIES C_STANDARD 17)
  if(BOTH_SAMPLES)
    target_compile_definitions(gpu_samples_api PRIVATE BOTH_SAMPLES=1)
  endif()

  if(NOT GPU_PHYSICS_LIB)
    set(GPU_PHYSICS_LIB "${GPU_PHYSICS_DIR}/target/release/libgpu_physics.a")
  endif()
  add_library(gpu_physics STATIC IMPORTED)
  set_target_properties(gpu_physics PROPERTIES IMPORTED_LOCATION "${GPU_PHYSICS_LIB}")

  target_link_options(${SAMPLE_TARGET} PRIVATE "-Wl,--allow-multiple-definition" "-Wl,--wrap=b3CreateCompound")

  if(BOTH_SAMPLES)
    find_program(BOTH_PYTHON python3 REQUIRED)
    find_program(BOTH_OBJCOPY objcopy REQUIRED)
    set(BOTH_CPU_LIB "${CMAKE_CURRENT_BINARY_DIR}/libbox3d_cpu.a")
    set(BOTH_PASSTHROUGH "${CMAKE_CURRENT_BINARY_DIR}/both_passthrough.c")
    add_custom_command(
      OUTPUT "${BOTH_CPU_LIB}" "${BOTH_PASSTHROUGH}"
      COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:box3d>" "${CMAKE_CURRENT_BINARY_DIR}/libbox3d_unprefixed.a"
      COMMAND "${BOTH_PYTHON}"
        "${GPU_PHYSICS_DIR}/scripts/gen-both-cpu.py"
        --archive "${CMAKE_CURRENT_BINARY_DIR}/libbox3d_unprefixed.a"
        --prefixed "${BOTH_CPU_LIB}"
        --passthrough "${BOTH_PASSTHROUGH}"
        --include-dir "${BOX3D_DIR}/include"
        --objcopy "${BOTH_OBJCOPY}"
      DEPENDS box3d "${GPU_PHYSICS_DIR}/scripts/gen-both-cpu.py" "${GPU_PHYSICS_DIR}/c_abi/both_dual.c"
      COMMENT "Prefixing Box3D symbols with cpu_ for samples_both"
    )
    add_custom_target(gen_box3d_cpu DEPENDS "${BOTH_CPU_LIB}" "${BOTH_PASSTHROUGH}")
    add_library(gpu_both_api STATIC
      "${GPU_PHYSICS_DIR}/c_abi/both_map.c"
      "${GPU_PHYSICS_DIR}/c_abi/both_dual.c"
      "${BOTH_PASSTHROUGH}")
    add_dependencies(gpu_both_api gen_box3d_cpu)
    target_include_directories(gpu_both_api PRIVATE
      "${BOX3D_DIR}/include"
      "${GPU_PHYSICS_DIR}/c_abi")
    target_compile_options(gpu_both_api PRIVATE -w)
    set_target_properties(gpu_both_api PROPERTIES C_STANDARD 17)
    add_library(box3d_cpu STATIC IMPORTED)
    set_target_properties(box3d_cpu PROPERTIES IMPORTED_LOCATION "${BOTH_CPU_LIB}")
    add_dependencies(${SAMPLE_TARGET} gen_box3d_cpu)

    target_link_libraries(${SAMPLE_TARGET} PRIVATE
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_both_api>"
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_samples_api>"
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_physics>"
      box3d_cpu
      dl pthread m)
    target_compile_definitions(${SAMPLE_TARGET} PRIVATE GPU_PHYSICS_SAMPLES=1 BOTH_SAMPLES=1)
  else()
    target_link_libraries(${SAMPLE_TARGET} PRIVATE
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_physics>"
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_samples_api>"
      box3d
      dl pthread m)
    target_compile_definitions(${SAMPLE_TARGET} PRIVATE GPU_PHYSICS_SAMPLES=1)
  endif()
endif()

set_target_properties(${SAMPLE_TARGET} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
