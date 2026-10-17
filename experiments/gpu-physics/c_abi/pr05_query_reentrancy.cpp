#include "box3d/box3d.h"
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
#endif
#include "query_reentrancy.cpp"
