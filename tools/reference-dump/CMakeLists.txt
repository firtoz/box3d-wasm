cmake_minimum_required(VERSION 3.22)
project(reference-dump C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Determine precision. Default to single (the Box3D default).
option(BOX3D_DOUBLE_PRECISION "Enable double precision for large worlds" OFF)

set(BOX3D_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../box3d")

# Build the Box3D physics library from source (no samples/tests/docs)
add_subdirectory("${BOX3D_SRC_DIR}" box3d-build)

# --- imgui stub (header-only, no imgui library needed) ---------------------
# We use a stub imgui.h in stubs/ instead of fetching the real imgui.

# --- Shared library (human, utils, benchmarks, determinism, stability, overflow_color) --
set(SHARED_SRC_DIR "${BOX3D_SRC_DIR}/shared")
set(SHARED_SOURCES
  "${SHARED_SRC_DIR}/benchmarks.c"
  "${SHARED_SRC_DIR}/determinism.c"
  "${SHARED_SRC_DIR}/human.c"
  "${SHARED_SRC_DIR}/overflow_color.c"
  "${SHARED_SRC_DIR}/stability.c"
  "${SHARED_SRC_DIR}/utils.c"
)
add_library(shared STATIC ${SHARED_SOURCES})
target_include_directories(shared PUBLIC "${SHARED_SRC_DIR}")
target_link_libraries(shared PRIVATE box3d)
set_target_properties(shared PROPERTIES
  C_STANDARD 17
  POSITION_INDEPENDENT_CODE ON
)

# --- Reference-dump executable --------------------------------------------
set(SAMPLE_SRC_DIR "${BOX3D_SRC_DIR}/samples")
set(STUBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stubs")

# Sample source files from upstream (excluding main.cpp, sample.cpp, host/, gfx/,
# mesh_loader.cpp, jsmn.h, and other non-sample files)
set(SAMPLE_FILES
  # sample_benchmark.cpp is included from dump-interaction-overrides.cpp so DumpDestruction can subclass it.
  "${SAMPLE_SRC_DIR}/sample_character.cpp"
  # sample_collision.cpp is included from dump-interaction-overrides.cpp so DumpRayCurtain can subclass it.
  "${SAMPLE_SRC_DIR}/sample_compound.cpp"
  "${SAMPLE_SRC_DIR}/sample_continuous.cpp"
  "${SAMPLE_SRC_DIR}/sample_determinism.cpp"
  "${SAMPLE_SRC_DIR}/sample_events.cpp"
  "${SAMPLE_SRC_DIR}/sample_geometry.cpp"
  # sample_issues.cpp is included from dump-interaction-overrides.cpp so DumpCrash can subclass it.
  "${SAMPLE_SRC_DIR}/sample_manifold.cpp"
  "${SAMPLE_SRC_DIR}/sample_mesh.cpp"
  "${SAMPLE_SRC_DIR}/sample_ragdoll.cpp"
  "${SAMPLE_SRC_DIR}/sample_replay.cpp"
  "${SAMPLE_SRC_DIR}/sample_robustness.cpp"
  "${SAMPLE_SRC_DIR}/sample_shapes.cpp"
  "${SAMPLE_SRC_DIR}/sample_stacking.cpp"
  "${SAMPLE_SRC_DIR}/sample_tree.cpp"
  "${SAMPLE_SRC_DIR}/sample_world.cpp"
  # Extra sample kept after upstream removed it (matches demo card-house-thick).
  "${CMAKE_CURRENT_SOURCE_DIR}/sample_card_house_thick.cpp"
)

set(REFERENCE_DUMP_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/dump-core.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/dump-core.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/dump-interaction-overrides.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/dump-interaction-overrides.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/reference-dump.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/sample-stubs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/stubs.cpp"
  # Upstream sample files compiled as-is (no modifications)
  ${SAMPLE_FILES}
)

add_executable(reference-dump ${REFERENCE_DUMP_SOURCES})

target_include_directories(reference-dump PRIVATE
  "${STUBS_DIR}"                         # stubs/gfx/draw.h, imgui.h, etc.
  "${SAMPLE_SRC_DIR}"                    # sample.h, mesh_loader.h, earcut.h, gfx/ headers
  "${BOX3D_SRC_DIR}/include"             # box3d/box3d.h, box3d/types.h
  "${BOX3D_SRC_DIR}/src"                 # body.h, physics_world.h, solver_set.h
  "${SHARED_SRC_DIR}"                    # human.h, utils.h
  "${BOX3D_SRC_DIR}/data"               # for data/ includes (sample_issues uses dumps/)
  "${CMAKE_CURRENT_SOURCE_DIR}"          # dump-core.h
)

target_link_libraries(reference-dump PRIVATE
  box3d
  shared
  m                                    # math library
)

# Use ffast-math for tests, -ffp-contract=off for determinism
target_compile_options(reference-dump PRIVATE
  -ffp-contract=off
)

set_target_properties(reference-dump PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Check if the binary is in the build dir for easy running
message(STATUS "reference-dump build: ${PROJECT_BINARY_DIR}/reference-dump")
