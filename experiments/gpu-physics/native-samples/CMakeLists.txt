cmake_minimum_required(VERSION 3.24)
project(box3d_native_samples VERSION 0.1.0 LANGUAGES C CXX)
if(APPLE)
  enable_language(OBJC)
endif()

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
option(GPU_PORTABLE_API "Select C API definitions without GNU linker interposition (requires external-c-shim)" OFF)

set(FETCHCONTENT_BASE_DIR "${BOX3D_DIR}/.fetchcontent-cache" CACHE PATH "")
include(FetchContent)

# CPU Box3D (math, hulls, defaults). GPU builds still link this; samples_api/stubs win world symbols.
add_subdirectory("${BOX3D_DIR}/src" box3d_src)
add_subdirectory("${BOX3D_DIR}/shared" box3d_shared)

# Diagnostic creation identities come from the shared Human creation order.
# Generate a local copy so the upstream submodule remains untouched.
file(READ "${BOX3D_DIR}/shared/human.c" HUMAN_SOURCE)
string(REGEX MATCHALL "human->isSpawned = true" HUMAN_HOOK_MATCHES "${HUMAN_SOURCE}")
list(LENGTH HUMAN_HOOK_MATCHES HUMAN_HOOK_COUNT)
if(NOT HUMAN_HOOK_COUNT EQUAL 1)
  message(FATAL_ERROR "Human lifetime hook requires exactly one spawn marker")
endif()
string(REPLACE "human->isSpawned = true;"
  "human->isSpawned = true;\n\textern void gpu_bench_human_created(const Human*);\n\tgpu_bench_human_created(human);"
  HUMAN_SOURCE "${HUMAN_SOURCE}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/diagnostic-human.c" "${HUMAN_SOURCE}")
get_target_property(SHARED_SOURCES shared SOURCES)
list(REMOVE_ITEM SHARED_SOURCES human.c)
set_property(TARGET shared PROPERTY SOURCES ${SHARED_SOURCES} "${CMAKE_CURRENT_BINARY_DIR}/diagnostic-human.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BOX3D_DIR}/shared/human.c")


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
if(WIN32)
  target_compile_definitions(sokol INTERFACE SOKOL_D3D11)
  target_link_libraries(sokol INTERFACE d3d11 dxgi dxguid user32 gdi32 shell32 ole32 winmm)
elseif(APPLE)
  target_compile_definitions(sokol INTERFACE SOKOL_METAL)
  target_link_libraries(sokol INTERFACE "-framework Foundation" "-framework Cocoa"
    "-framework QuartzCore" "-framework Metal" "-framework MetalKit")
  set_source_files_properties("${BOX3D_DIR}/samples/host/sokol_app_impl.c"
    PROPERTIES LANGUAGE OBJC COMPILE_OPTIONS "-fobjc-arc")
elseif(UNIX)
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

find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
set(CAPACITY_GFX_DIR "${CMAKE_CURRENT_BINARY_DIR}/capacity-gfx")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${GPU_PHYSICS_DIR}/scripts/inject-sokol-capacity.py"
  "${BOX3D_DIR}/samples/gfx" "${CAPACITY_GFX_DIR}" COMMAND_ERROR_IS_FATAL ANY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${GPU_PHYSICS_DIR}/scripts/inject-sokol-capacity.py")
file(GLOB LOCAL_GFX_INPUTS CONFIGURE_DEPENDS "${BOX3D_DIR}/samples/gfx/*.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${LOCAL_GFX_INPUTS})
set(LOCAL_GFX_DIR "${CAPACITY_GFX_DIR}")
if(BOTH_SAMPLES)
  set(LOCAL_GFX_DIR "${CMAKE_CURRENT_BINARY_DIR}/split-gfx")
  execute_process(COMMAND "${Python3_EXECUTABLE}" "${GPU_PHYSICS_DIR}/scripts/inject-sokol-split.py"
    "${CAPACITY_GFX_DIR}" "${LOCAL_GFX_DIR}" COMMAND_ERROR_IS_FATAL ANY)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${GPU_PHYSICS_DIR}/scripts/inject-sokol-split.py")
  target_compile_definitions(gfx PRIVATE BOTH_SAMPLES)
endif()
get_target_property(GFX_SOURCES gfx SOURCES)
set(LOCAL_GFX_SOURCES)
foreach(SOURCE IN LISTS GFX_SOURCES)
  if(SOURCE MATCHES "\\.c$" AND NOT SOURCE STREQUAL "sokol_impl.c")
    list(APPEND LOCAL_GFX_SOURCES "${LOCAL_GFX_DIR}/${SOURCE}")
  else()
    list(APPEND LOCAL_GFX_SOURCES "${BOX3D_DIR}/samples/gfx/${SOURCE}")
  endif()
endforeach()
set_property(TARGET gfx PROPERTY SOURCES ${LOCAL_GFX_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/sokol_capacity.c")
if(BOTH_SAMPLES)
  target_sources(gfx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sokol_split_stream.c")
endif()
target_include_directories(gfx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

set(MAIN_CPP "${CMAKE_CURRENT_BINARY_DIR}/main_bench.cpp")
set(SAMPLE_CPP "${CMAKE_CURRENT_BINARY_DIR}/sample_bench.cpp")
set(CONTINUOUS_CPP "${CMAKE_CURRENT_BINARY_DIR}/sample_continuous_bench.cpp")
set(CHARACTER_CPP "${CMAKE_CURRENT_BINARY_DIR}/sample_character_loading.cpp")
set(JOINT_CPP "${CMAKE_CURRENT_BINARY_DIR}/sample_joint_bench.cpp")
set(INJECT_EXTRA)
if(GPU_SAMPLES OR BOTH_SAMPLES)
  list(APPEND INJECT_EXTRA --gpu-sidebar)
endif()
add_custom_command(
  OUTPUT "${MAIN_CPP}" "${SAMPLE_CPP}" "${CONTINUOUS_CPP}" "${JOINT_CPP}" "${CHARACTER_CPP}"
  COMMAND "${Python3_EXECUTABLE}" "${GPU_PHYSICS_DIR}/scripts/inject-sokol-bench.py"
    --src "${BOX3D_DIR}/samples/main.cpp"
    --dst "${MAIN_CPP}"
    --sample-src "${BOX3D_DIR}/samples/sample.cpp"
    --sample-dst "${SAMPLE_CPP}"
    --continuous-src "${BOX3D_DIR}/samples/sample_continuous.cpp"
    --continuous-dst "${CONTINUOUS_CPP}"
    --character-src "${BOX3D_DIR}/samples/sample_character.cpp"
    --character-dst "${CHARACTER_CPP}"
    --joint-src "${BOX3D_DIR}/samples/sample_joint.cpp"
    --joint-dst "${JOINT_CPP}"
    ${INJECT_EXTRA}
  DEPENDS
    "${GPU_PHYSICS_DIR}/scripts/sokol_split_injection.py"
    "${BOX3D_DIR}/samples/main.cpp"
    "${BOX3D_DIR}/samples/sample.cpp"
    "${BOX3D_DIR}/samples/sample_continuous.cpp"
    "${BOX3D_DIR}/samples/sample_character.cpp"
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
  "${CMAKE_CURRENT_SOURCE_DIR}/sample_shape_replacement.cpp"
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
  "${CHARACTER_CPP}"
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
  if(NOT GPU_PHYSICS_LIB)
    if(MSVC)
      set(GPU_PHYSICS_LIB "${GPU_PHYSICS_DIR}/target/release/gpu_physics.lib")
    else()
      set(GPU_PHYSICS_LIB "${GPU_PHYSICS_DIR}/target/release/libgpu_physics.a")
    endif()
  endif()
  set(LINK_SOURCES "${GPU_PHYSICS_DIR}/c_abi")
  set(SHIM_SOURCE)
  set(CPU_ORACLE_TARGET box3d)
  set(PASSTHROUGH_EXTRA)
  find_program(SAMPLES_NM NAMES llvm-nm nm REQUIRED)
  if(GPU_PORTABLE_API)
    # LLVM tools handle ELF, Mach-O and COFF. GNU tools suffice on Linux.
    set(LINK_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/api-sources")
    set(LINK_EXTRA)
    if(BOTH_SAMPLES)
      list(APPEND LINK_EXTRA --both)
      # Preserve the complete original CPU engine before selecting GPU fallbacks.
      get_target_property(CPU_SOURCES box3d SOURCES)
      list(TRANSFORM CPU_SOURCES PREPEND "${BOX3D_DIR}/src/")
      # SOURCES also contains absolute public header paths.
      list(FILTER CPU_SOURCES INCLUDE REGEX "\\.c$")
      add_library(box3d_cpu_original STATIC ${CPU_SOURCES})
      target_include_directories(box3d_cpu_original PRIVATE "${BOX3D_DIR}/src" PUBLIC "${BOX3D_DIR}/include")
      get_target_property(CPU_DEFINITIONS box3d COMPILE_DEFINITIONS)
      if(CPU_DEFINITIONS)
        target_compile_definitions(box3d_cpu_original PRIVATE ${CPU_DEFINITIONS})
      endif()
    endif()
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${GPU_PHYSICS_DIR}/scripts/prepare-samples-link.py"
      --root "${GPU_PHYSICS_DIR}" --box3d "${BOX3D_DIR}" --rust-lib "${GPU_PHYSICS_LIB}"
      --out "${LINK_SOURCES}" --nm "${SAMPLES_NM}" ${LINK_EXTRA} COMMAND_ERROR_IS_FATAL ANY)
    file(GLOB BASE_C_SOURCES CONFIGURE_DEPENDS "${BOX3D_DIR}/src/*.c")
    set(BASE_FILTERED)
    foreach(SOURCE IN LISTS BASE_C_SOURCES)
      get_filename_component(NAME "${SOURCE}" NAME)
      list(APPEND BASE_FILTERED "${LINK_SOURCES}/${NAME}")
    endforeach()
    set_property(TARGET box3d PROPERTY SOURCES ${BASE_FILTERED})
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
      "${GPU_PHYSICS_LIB}" "${GPU_PHYSICS_DIR}/scripts/prepare-samples-link.py"
      "${GPU_PHYSICS_DIR}/scripts/gen-both-cpu.py"
      "${GPU_PHYSICS_DIR}/c_abi/shim.c" "${GPU_PHYSICS_DIR}/c_abi/samples_api.c"
      "${GPU_PHYSICS_DIR}/c_abi/samples_stubs.c" "${GPU_PHYSICS_DIR}/c_abi/both_dual.c"
      "${GPU_PHYSICS_DIR}/c_abi/native_unavailable.inc" "${GPU_PHYSICS_DIR}/c_abi/native_api_status.h"
      "${GPU_PHYSICS_DIR}/c_abi/native_shared.inc" "${GPU_PHYSICS_DIR}/c_abi/native_diagnostics.h" ${BASE_C_SOURCES})
    set(SHIM_SOURCE "${LINK_SOURCES}/shim.c")
    set(CPU_ORACLE_TARGET box3d_cpu_original)
    list(APPEND PASSTHROUGH_EXTRA --exclude-archive "${GPU_PHYSICS_LIB}")
  endif()
  target_sources(${SAMPLE_TARGET} PRIVATE
    "${GPU_PHYSICS_DIR}/native-samples/sample_stats.cpp")
  target_include_directories(${SAMPLE_TARGET} PRIVATE "${GPU_PHYSICS_DIR}/c_abi")
  add_library(gpu_samples_api STATIC
    "${LINK_SOURCES}/samples_api.c"
    "${LINK_SOURCES}/samples_stubs.c"
    ${SHIM_SOURCE}
    "${GPU_PHYSICS_DIR}/c_abi/gpu_gl_poses.c")
  target_include_directories(gpu_samples_api PRIVATE "${BOX3D_DIR}/include" "${GPU_PHYSICS_DIR}/c_abi")
  target_compile_options(gpu_samples_api PRIVATE "$<IF:$<C_COMPILER_ID:MSVC>,/w,-w>")
  set_target_properties(gpu_samples_api PROPERTIES C_STANDARD 17)
  if(BOTH_SAMPLES)
    target_compile_definitions(gpu_samples_api PRIVATE BOTH_SAMPLES=1)
  endif()

  if(NOT GPU_PHYSICS_LIB)
    set(GPU_PHYSICS_LIB "${GPU_PHYSICS_DIR}/target/release/libgpu_physics.a")
  endif()
  add_library(gpu_physics STATIC IMPORTED)
  set_target_properties(gpu_physics PROPERTIES IMPORTED_LOCATION "${GPU_PHYSICS_LIB}")

  if(NOT GPU_PORTABLE_API)
    target_link_options(${SAMPLE_TARGET} PRIVATE "-Wl,--allow-multiple-definition" "-Wl,--wrap=b3CreateCompound")
  endif()
  find_package(Threads REQUIRED)
  target_link_libraries(${SAMPLE_TARGET} PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
  if(WIN32)
    target_link_libraries(${SAMPLE_TARGET} PRIVATE ws2_32 userenv bcrypt ntdll advapi32 dbghelp oleaut32 uuid d3d12)
  elseif(APPLE)
    target_link_libraries(${SAMPLE_TARGET} PRIVATE "-framework Security" "-framework SystemConfiguration" "-framework CoreFoundation" "-framework IOKit")
  endif()

  if(BOTH_SAMPLES)
    set(BOTH_PYTHON "${Python3_EXECUTABLE}")
    find_program(BOTH_OBJCOPY NAMES llvm-objcopy objcopy REQUIRED)
    set(BOTH_CPU_LIB "${CMAKE_CURRENT_BINARY_DIR}/libbox3d_cpu.a")
    set(BOTH_PASSTHROUGH "${CMAKE_CURRENT_BINARY_DIR}/both_passthrough.c")
    add_custom_command(
      OUTPUT "${BOTH_CPU_LIB}" "${BOTH_PASSTHROUGH}"
      COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${CPU_ORACLE_TARGET}>" "${CMAKE_CURRENT_BINARY_DIR}/libbox3d_unprefixed.a"
      COMMAND "${BOTH_PYTHON}"
        "${GPU_PHYSICS_DIR}/scripts/gen-both-cpu.py"
        --archive "${CMAKE_CURRENT_BINARY_DIR}/libbox3d_unprefixed.a"
        --prefixed "${BOTH_CPU_LIB}"
        --passthrough "${BOTH_PASSTHROUGH}"
        --include-dir "${BOX3D_DIR}/include"
        --objcopy "${BOTH_OBJCOPY}" --nm "${SAMPLES_NM}" ${PASSTHROUGH_EXTRA}
      DEPENDS ${CPU_ORACLE_TARGET} "${GPU_PHYSICS_DIR}/scripts/gen-both-cpu.py" "${GPU_PHYSICS_DIR}/c_abi/both_dual.c"
        "${GPU_PHYSICS_DIR}/c_abi/native_unavailable.inc" "${GPU_PHYSICS_DIR}/c_abi/native_shared.inc"
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
    target_compile_options(gpu_both_api PRIVATE "$<IF:$<C_COMPILER_ID:MSVC>,/w,-w>")
    set_target_properties(gpu_both_api PROPERTIES C_STANDARD 17)
    add_library(box3d_cpu STATIC IMPORTED)
    set_target_properties(box3d_cpu PROPERTIES IMPORTED_LOCATION "${BOTH_CPU_LIB}")
    add_dependencies(${SAMPLE_TARGET} gen_box3d_cpu)

    target_link_libraries(${SAMPLE_TARGET} PRIVATE
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_both_api>"
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_samples_api>"
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_physics>"
      box3d_cpu)
    target_compile_definitions(${SAMPLE_TARGET} PRIVATE GPU_PHYSICS_SAMPLES=1 BOTH_SAMPLES=1)
  else()
    target_link_libraries(${SAMPLE_TARGET} PRIVATE
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_physics>"
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,gpu_samples_api>"
      box3d)
    target_compile_definitions(${SAMPLE_TARGET} PRIVATE GPU_PHYSICS_SAMPLES=1)
  endif()
endif()

set_target_properties(${SAMPLE_TARGET} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")
