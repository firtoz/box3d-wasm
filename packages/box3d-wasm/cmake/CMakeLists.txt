cmake_minimum_required(VERSION 3.22)

project(box3d_wasm_bridge C)

option(BOX3D_WASM_PROFILE_NAMES "Emit wasm function names/source maps for Chromium profiling" OFF)
option(BOX3D_WASM_ALLOW_MEMORY_GROWTH "Allow the WASM heap to grow at runtime" OFF)

set(B3W_MAX_WORLDS 16 CACHE STRING "Maximum tracked world handles in the WASM bridge")
set(B3W_MAX_BODIES 65536 CACHE STRING "Maximum tracked body handles in the WASM bridge")
set(B3W_MAX_JOINTS 65536 CACHE STRING "Maximum tracked joint handles in the WASM bridge")
set(B3W_MAX_HULLS 16384 CACHE STRING "Maximum tracked hull handles in the WASM bridge")
set(B3W_MAX_SHAPES 65536 CACHE STRING "Maximum tracked shape handles in the WASM bridge")
set(B3W_MAX_MESHES 1024 CACHE STRING "Maximum tracked mesh handles in the WASM bridge")
set(B3W_MAX_COMPOUNDS 1024 CACHE STRING "Maximum tracked compound handles in the WASM bridge")
set(B3W_MAX_HUMANS 512 CACHE STRING "Maximum tracked human handles in the WASM bridge")
set(BOX3D_WASM_INITIAL_MEMORY 268435456 CACHE STRING "Initial WASM heap size in bytes")

if(EMSCRIPTEN)
	add_compile_options(-pthread)
endif()


if(NOT DEFINED BOX3D_SOURCE_DIR)
	set(BOX3D_SOURCE_DIR "$ENV{HOME}/.cache/box3d-wasm/box3d")
endif()

if(NOT EXISTS "${BOX3D_SOURCE_DIR}/CMakeLists.txt")
	message(FATAL_ERROR "BOX3D_SOURCE_DIR does not point at a box3d checkout: ${BOX3D_SOURCE_DIR}")
endif()

add_subdirectory("${BOX3D_SOURCE_DIR}" box3d-build)

add_executable(box3d_web
	box3d_web.c
	box3d_web_slots.c
	box3d_web_world.c
	box3d_web_body.c
	box3d_web_math.c
	box3d_web_shape.c
	box3d_web_mesh.c
	box3d_web_hull.c
	box3d_web_compound.c
	box3d_web_joint.c
	box3d_web_human.c
	"${BOX3D_SOURCE_DIR}/shared/human.c"
	"${BOX3D_SOURCE_DIR}/shared/utils.c"
)
target_include_directories(box3d_web PRIVATE "${BOX3D_SOURCE_DIR}/include" "${BOX3D_SOURCE_DIR}/shared" "${BOX3D_SOURCE_DIR}/src")
target_link_libraries(box3d_web PRIVATE box3d)
target_compile_definitions(box3d_web PRIVATE
	B3W_MAX_WORLDS=${B3W_MAX_WORLDS}
	B3W_MAX_BODIES=${B3W_MAX_BODIES}
	B3W_MAX_JOINTS=${B3W_MAX_JOINTS}
	B3W_MAX_HULLS=${B3W_MAX_HULLS}
	B3W_MAX_SHAPES=${B3W_MAX_SHAPES}
	B3W_MAX_MESHES=${B3W_MAX_MESHES}
	B3W_MAX_COMPOUNDS=${B3W_MAX_COMPOUNDS}
	B3W_MAX_HUMANS=${B3W_MAX_HUMANS}
)

set_target_properties(box3d_web PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED YES
	C_EXTENSIONS YES
	OUTPUT_NAME box3d-web
)

set(_box3d_wasm_link_options
	"SHELL:-sMODULARIZE=1"
	"SHELL:-sEXPORT_ES6=1"
	"SHELL:-sENVIRONMENT=web,worker,node"
	"SHELL:-sINITIAL_MEMORY=${BOX3D_WASM_INITIAL_MEMORY}"
	"SHELL:-sNO_EXIT_RUNTIME=1"
	"SHELL:-sEXPORTED_RUNTIME_METHODS=['cwrap','HEAPF32','HEAPU8','HEAP32']"
	"SHELL:-sEXPORTED_FUNCTIONS=['_main','_malloc','_free']"
	"SHELL:-sASSERTIONS=0"
	"SHELL:-sEXPORT_KEEPALIVE=1"
	"SHELL:-sUSE_PTHREADS=1"
	"SHELL:-sPTHREAD_POOL_SIZE=(globalThis.BOX3D_POOL_SIZE||(globalThis.navigator?globalThis.navigator.hardwareConcurrency:4))"
	"SHELL:-sSTACK_SIZE=2097152"
	"SHELL:-sDEFAULT_PTHREAD_STACK_SIZE=1048576"
	"SHELL:-sFILESYSTEM=0"
)

if(BOX3D_WASM_ALLOW_MEMORY_GROWTH)
	list(APPEND _box3d_wasm_link_options "SHELL:-sALLOW_MEMORY_GROWTH=1")
else()
	list(APPEND _box3d_wasm_link_options "SHELL:-sALLOW_MEMORY_GROWTH=0")
endif()

target_link_options(box3d_web PRIVATE ${_box3d_wasm_link_options})

if(BOX3D_WASM_PROFILE_NAMES)
	target_compile_options(box3d PRIVATE -g)
	target_compile_options(box3d_web PRIVATE -g)
	target_link_options(box3d_web PRIVATE
		-g
		-gsource-map
		--profiling-funcs
	)
endif()
