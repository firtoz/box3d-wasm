#pragma once

#include <stdio.h>
#include "box3d/id.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dump all body transforms at the current frame to a JSON fragment.
// Called per checkpoint. Returns 1 if all bodies are asleep.
int dump_bodies(FILE* out, b3WorldId worldId, int frame, int emitComma);
int all_bodies_asleep(b3WorldId worldId);

#ifdef __cplusplus
}
#endif
