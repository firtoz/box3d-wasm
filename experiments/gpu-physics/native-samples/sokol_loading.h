#pragma once
#include "box3d/id.h"
bool gpu_loading_poll(b3WorldId world);
bool gpu_loading_active();
void gpu_loading_draw();
void gpu_loading_shutdown();
