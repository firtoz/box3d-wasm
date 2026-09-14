#pragma once
#include "box3d/box3d.h"
#ifdef __cplusplus
extern "C" {
#endif
bool both_split_enabled(void);
void both_set_split(bool enabled);
void both_draw_second(void);
void both_begin_frame(void);
void both_pointer_down(b3WorldId world, b3Pos origin, b3Vec3 translation, bool grab, float force_scale);
void both_pointer_move(b3Pos origin, b3Vec3 translation);
void both_pointer_up(void);
void both_pointer_impulse(b3WorldId world, b3Pos origin, b3Vec3 translation, b3Vec3 impulse);
typedef struct BothPointerState {
    b3BodyId selected, mouse;
    b3JointId joint;
    float fraction;
    b3Vec3 local_anchor;
} BothPointerState;
BothPointerState both_pointer_state(int engine);
#ifdef __cplusplus
}
#endif
