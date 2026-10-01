// Explicit unavailable native operations.
// Supported engine implementations belong in shim.c.
// Do not add duplicate fallback definitions for implemented APIs.
#include "box3d/box3d.h"
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "native_api_status.h"

static _Thread_local const char* g_unavailable_operation;

void gpu_native_api_error(const char* operation, int error_code)
{
	g_unavailable_operation = operation;
	errno = error_code;
}

void gpu_native_api_unavailable(const char* operation)
{
	gpu_native_api_error(operation, ENOTSUP);
}

const char* gpu_b3_native_api_last_error(void)
{
	return g_unavailable_operation;
}

void gpu_b3_native_api_clear_error(void)
{
	g_unavailable_operation = NULL;
	errno = 0;
}

bool gpu_b3_native_api_is_unavailable(const char* operation)
{
	if (!operation) return false;
#define GPU_UNAVAILABLE_API(name) if (strcmp(operation, #name) == 0) return true;
#include "native_unavailable.inc"
#undef GPU_UNAVAILABLE_API
	return false;
}

B3_API void b3World_SetWorkerCount(b3WorldId worldId, int count)
{
	gpu_native_api_unavailable(__func__);
}

B3_API int b3World_GetWorkerCount(b3WorldId worldId)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API void b3World_RebuildStaticTree(b3WorldId worldId)
{
	gpu_native_api_unavailable(__func__);
}

B3_API const uint8_t* b3Recording_GetData(const b3Recording* recording)
{
	gpu_native_api_unavailable(__func__);
	return NULL;
}

B3_API int b3Recording_GetSize(const b3Recording* recording)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API b3Recording* b3LoadRecordingFromFile(const char* path)
{
	gpu_native_api_unavailable(__func__);
	return NULL;
}

B3_API bool b3ValidateReplay(const void* data, int size, int workerCount)
{
	gpu_native_api_unavailable(__func__);
	return false;
}

B3_API b3RecPlayer* b3CreatePlayer(const void* data, int size, int workerCount)
{
	gpu_native_api_unavailable(__func__);
	return NULL;
}

B3_API void b3DestroyPlayer(b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
}

B3_API bool b3RecPlayer_StepFrame(b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return false;
}

B3_API void b3RecPlayer_SubStepFrame(b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
}

B3_API void b3RecPlayer_Restart(b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
}

B3_API void b3RecPlayer_SeekFrame(b3RecPlayer* player, int targetFrame)
{
	gpu_native_api_unavailable(__func__);
}

B3_API b3WorldId b3RecPlayer_GetWorldId(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return (b3WorldId){0};
}

B3_API int b3RecPlayer_GetFrame(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API int b3RecPlayer_GetFrameCount(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API bool b3RecPlayer_IsAtEnd(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return false;
}

B3_API bool b3RecPlayer_IsAtPreStep(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return false;
}

B3_API bool b3RecPlayer_HasDiverged(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return false;
}

B3_API b3RecPlayerInfo b3RecPlayer_GetInfo(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return (b3RecPlayerInfo){0};
}

B3_API int b3RecPlayer_GetDivergeFrame(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API void b3RecPlayer_SetWorkerCount(b3RecPlayer* player, int count)
{
	gpu_native_api_unavailable(__func__);
}

B3_API void b3RecPlayer_SetKeyframePolicy(b3RecPlayer* player, size_t budgetBytes, int minIntervalFrames)
{
	gpu_native_api_unavailable(__func__);
}

B3_API size_t b3RecPlayer_GetKeyframeBudget(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API int b3RecPlayer_GetKeyframeMinInterval(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API int b3RecPlayer_GetKeyframeInterval(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API size_t b3RecPlayer_GetKeyframeBytes(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API int b3RecPlayer_GetBodyCount(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API b3BodyId b3RecPlayer_GetBodyId(const b3RecPlayer* player, int index)
{
	gpu_native_api_unavailable(__func__);
	return (b3BodyId){0};
}

B3_API void b3RecPlayer_SetDebugShapeCallbacks(b3RecPlayer* player, b3CreateDebugShapeCallback* createDebugShape, b3DestroyDebugShapeCallback* destroyDebugShape, void* context)
{
	gpu_native_api_unavailable(__func__);
}

B3_API void b3RecPlayer_DrawFrameQueries(b3RecPlayer* player, b3DebugDraw* draw, int queryIndex, int selectedIndex)
{
	gpu_native_api_unavailable(__func__);
}

B3_API int b3RecPlayer_GetFrameQueryCount(const b3RecPlayer* player)
{
	gpu_native_api_unavailable(__func__);
	return 0;
}

B3_API b3RecQueryInfo b3RecPlayer_GetFrameQuery(const b3RecPlayer* player, int index)
{
	gpu_native_api_unavailable(__func__);
	return (b3RecQueryInfo){0};
}

B3_API b3RecQueryHit b3RecPlayer_GetFrameQueryHit(const b3RecPlayer* player, int queryIndex, int hitIndex)
{
	gpu_native_api_unavailable(__func__);
	return (b3RecQueryHit){0};
}
