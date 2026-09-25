#pragma once
#include "box3d/id.h"
bool gpu_loading_poll(b3WorldId world);
bool gpu_loading_active();
void gpu_loading_draw();
void gpu_loading_shutdown();
#include <functional>
void gpu_loading_create_scene(std::function<b3WorldId()> create);
void gpu_loading_presented();
void gpu_loading_begin_startup();
void gpu_loading_lock_mouse(bool lock);
void gpu_loading_request_close();
bool gpu_loading_close_pending();
