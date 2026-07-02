cmake_minimum_required(VERSION 3.22)

project(box3d_wasm_bridge C)

if(EMSCRIPTEN)
	add_compile_options(-matomics -mbulk-memory)
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
	box3d_web_shape.c
	box3d_web_hull.c
	box3d_web_compound.c
	box3d_web_joint.c
	box3d_web_human.c
	"${BOX3D_SOURCE_DIR}/shared/human.c"
)
target_include_directories(box3d_web PRIVATE "${BOX3D_SOURCE_DIR}/include" "${BOX3D_SOURCE_DIR}/shared")
target_link_libraries(box3d_web PRIVATE box3d)

set_target_properties(box3d_web PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED YES
	C_EXTENSIONS YES
	OUTPUT_NAME box3d-web
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../demo/public/wasm"
)

	target_link_options(box3d_web PRIVATE
		"SHELL:-sMODULARIZE=1"
		"SHELL:-sEXPORT_ES6=1"
		"SHELL:-sENVIRONMENT=web"
		"SHELL:-sALLOW_MEMORY_GROWTH=1"
		"SHELL:-sNO_EXIT_RUNTIME=1"
		"SHELL:-sEXPORTED_RUNTIME_METHODS=['cwrap','HEAPF32']"
		"SHELL:-sEXPORTED_FUNCTIONS=['_main','_malloc','_free']"
		"SHELL:-sEXPORT_KEEPALIVE=1"
		"SHELL:-sUSE_PTHREADS=1"
		"SHELL:-sPTHREAD_POOL_SIZE=4"
	)
