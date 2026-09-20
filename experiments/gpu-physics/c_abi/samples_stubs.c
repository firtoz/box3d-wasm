// Remaining unimplemented native APIs. Real implementations belong in shim.c.
// Do not add duplicate fallback definitions for implemented APIs.
#include "box3d/box3d.h"
#include <stddef.h>

B3_API void b3World_SetWorkerCount(b3WorldId worldId, int count)
{
	(void)sizeof(char);
}

B3_API int b3World_GetWorkerCount(b3WorldId worldId)
{
	(void)sizeof(char);
	return 0;
}

B3_API void b3World_DumpShapeBounds(b3WorldId worldId, b3BodyType type)
{
	(void)sizeof(char);
}

B3_API void b3World_RebuildStaticTree(b3WorldId worldId)
{
	(void)sizeof(char);
}

B3_API void b3World_EnableSpeculative(b3WorldId worldId, bool flag)
{
	(void)sizeof(char);
}

B3_API const uint8_t* b3Recording_GetData(const b3Recording* recording)
{
	(void)sizeof(char);
	return NULL;
}

B3_API int b3Recording_GetSize(const b3Recording* recording)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3Recording* b3LoadRecordingFromFile(const char* path)
{
	(void)sizeof(char);
	return NULL;
}

B3_API bool b3ValidateReplay(const void* data, int size, int workerCount)
{
	(void)sizeof(char);
	return false;
}

B3_API b3RecPlayer* b3CreatePlayer(const void* data, int size, int workerCount)
{
	(void)sizeof(char);
	return NULL;
}

B3_API void b3DestroyPlayer(b3RecPlayer* player)
{
	(void)sizeof(char);
}

B3_API bool b3RecPlayer_StepFrame(b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API void b3RecPlayer_SubStepFrame(b3RecPlayer* player)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_Restart(b3RecPlayer* player)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_SeekFrame(b3RecPlayer* player, int targetFrame)
{
	(void)sizeof(char);
}

B3_API b3WorldId b3RecPlayer_GetWorldId(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return (b3WorldId){0};
}

B3_API int b3RecPlayer_GetFrame(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetFrameCount(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API bool b3RecPlayer_IsAtEnd(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API bool b3RecPlayer_IsAtPreStep(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API bool b3RecPlayer_HasDiverged(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API b3RecPlayerInfo b3RecPlayer_GetInfo(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return (b3RecPlayerInfo){0};
}

B3_API int b3RecPlayer_GetDivergeFrame(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API void b3RecPlayer_SetWorkerCount(b3RecPlayer* player, int count)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_SetKeyframePolicy(b3RecPlayer* player, size_t budgetBytes, int minIntervalFrames)
{
	(void)sizeof(char);
}

B3_API size_t b3RecPlayer_GetKeyframeBudget(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetKeyframeMinInterval(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetKeyframeInterval(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API size_t b3RecPlayer_GetKeyframeBytes(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetBodyCount(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3BodyId b3RecPlayer_GetBodyId(const b3RecPlayer* player, int index)
{
	(void)sizeof(char);
	return (b3BodyId){0};
}

B3_API void b3RecPlayer_SetDebugShapeCallbacks(b3RecPlayer* player, b3CreateDebugShapeCallback* createDebugShape, b3DestroyDebugShapeCallback* destroyDebugShape, void* context)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_DrawFrameQueries(b3RecPlayer* player, b3DebugDraw* draw, int queryIndex, int selectedIndex)
{
	(void)sizeof(char);
}

B3_API int b3RecPlayer_GetFrameQueryCount(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3RecQueryInfo b3RecPlayer_GetFrameQuery(const b3RecPlayer* player, int index)
{
	(void)sizeof(char);
	return (b3RecQueryInfo){0};
}

B3_API b3RecQueryHit b3RecPlayer_GetFrameQueryHit(const b3RecPlayer* player, int queryIndex, int hitIndex)
{
	(void)sizeof(char);
	return (b3RecQueryHit){0};
}
