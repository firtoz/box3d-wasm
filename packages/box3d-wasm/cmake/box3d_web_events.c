#include "box3d_web_shared.h"

#include <stdint.h>
#include <string.h>

// Open-addressing map: packed native body id key -> render index.
// Capacity is 2x max tracked bodies so load stays reasonable when fully tracked.
// Empty slots use key == 0 (null body ids are never inserted).
#ifndef B3W_MOVE_MAP_MAX_BODIES
#define B3W_MOVE_MAP_MAX_BODIES 65536
#endif
#define B3W_MOVE_MAP_CAPACITY (B3W_MOVE_MAP_MAX_BODIES * 2)
#define B3W_MOVE_MAP_EMPTY (-1)

static uint64_t g_moveKeys[B3W_MOVE_MAP_CAPACITY];
static int g_moveIndices[B3W_MOVE_MAP_CAPACITY];
static int g_moveMapCount = 0;

static uint32_t b3wMoveMapHash(uint64_t key)
{
	key ^= key >> 33;
	key *= 0xff51afd7ed558ccdULL;
	key ^= key >> 33;
	return (uint32_t)key;
}

static void b3wMoveMapInsert(uint64_t key, int renderIndex)
{
	if (key == 0 || g_moveMapCount >= B3W_MOVE_MAP_MAX_BODIES)
	{
		return;
	}

	uint32_t slot = b3wMoveMapHash(key) % (uint32_t)B3W_MOVE_MAP_CAPACITY;
	for (int probe = 0; probe < B3W_MOVE_MAP_CAPACITY; ++probe)
	{
		if (g_moveKeys[slot] == 0)
		{
			g_moveKeys[slot] = key;
			g_moveIndices[slot] = renderIndex;
			++g_moveMapCount;
			return;
		}
		if (g_moveKeys[slot] == key)
		{
			g_moveIndices[slot] = renderIndex;
			return;
		}
		slot = (slot + 1) % (uint32_t)B3W_MOVE_MAP_CAPACITY;
	}
}

static int b3wMoveMapLookup(uint64_t key)
{
	if (key == 0 || g_moveMapCount == 0)
	{
		return B3W_MOVE_MAP_EMPTY;
	}

	uint32_t slot = b3wMoveMapHash(key) % (uint32_t)B3W_MOVE_MAP_CAPACITY;
	for (int probe = 0; probe < B3W_MOVE_MAP_CAPACITY; ++probe)
	{
		if (g_moveKeys[slot] == 0)
		{
			return B3W_MOVE_MAP_EMPTY;
		}
		if (g_moveKeys[slot] == key)
		{
			return g_moveIndices[slot];
		}
		slot = (slot + 1) % (uint32_t)B3W_MOVE_MAP_CAPACITY;
	}
	return B3W_MOVE_MAP_EMPTY;
}

B3W_EXPORT void b3wClearBodyMoveTracking(void)
{
	memset(g_moveKeys, 0, sizeof(g_moveKeys));
	memset(g_moveIndices, 0, sizeof(g_moveIndices));
	g_moveMapCount = 0;
}

B3W_EXPORT void b3wConfigureBodyMoveTracking(int count, const uint64_t* bodyPackedIds)
{
	b3wClearBodyMoveTracking();
	if (bodyPackedIds == NULL || count <= 0)
	{
		return;
	}

	int n = count;
	if (n > B3W_MOVE_MAP_MAX_BODIES)
	{
		n = B3W_MOVE_MAP_MAX_BODIES;
	}

	for (int i = 0; i < n; ++i)
	{
		uint64_t key = bodyPackedIds[i];
		if (key == 0)
		{
			continue;
		}
		// Keys ARE the packed body ids (no slot lookup).
		b3wMoveMapInsert(key, i);
	}
}

B3W_EXPORT int b3wGetBodyMoveEventCount(int worldHandle)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL)
	{
		return -1;
	}
	b3BodyEvents events = b3World_GetBodyEvents(world->worldId);
	return events.moveCount;
}

B3W_EXPORT int b3wScatterBodyMoveEvents(
	int worldHandle,
	float* outPositions,
	float* outRotations,
	char* outAwake,
	uint32_t* outColors,
	int useLightColors)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL || outPositions == NULL || outRotations == NULL || outAwake == NULL || outColors == NULL)
	{
		return -1;
	}

	b3BodyEvents events = b3World_GetBodyEvents(world->worldId);
	for (int e = 0; e < events.moveCount; ++e)
	{
		const b3BodyMoveEvent* ev = events.moveEvents + e;
		int i = b3wMoveMapLookup(b3StoreBodyId(ev->bodyId));
		if (i < 0)
		{
			continue;
		}

		outPositions[i * 3 + 0] = ev->transform.p.x;
		outPositions[i * 3 + 1] = ev->transform.p.y;
		outPositions[i * 3 + 2] = ev->transform.p.z;
		outRotations[i * 4 + 0] = ev->transform.q.v.x;
		outRotations[i * 4 + 1] = ev->transform.q.v.y;
		outRotations[i * 4 + 2] = ev->transform.q.v.z;
		outRotations[i * 4 + 3] = ev->transform.q.s;

		char awake = ev->fellAsleep ? 0 : 1;
		outAwake[i] = awake;

		if (useLightColors)
		{
			outColors[i] = awake ? 0xd2b48c : 0x778899;
		}
		else
		{
			outColors[i] = (uint32_t)b3wGetBodyDebugColorForId(ev->bodyId);
		}
	}

	return events.moveCount;
}

B3W_EXPORT int b3wGetContactBeginEventCount(int worldHandle)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return -1;
	b3ContactEvents events = b3World_GetContactEvents(world->worldId);
	return events.beginCount;
}

B3W_EXPORT int b3wGetContactBeginEvent(int worldHandle, int index, uint64_t* outShapeAPacked, uint64_t* outShapeBPacked)
{
	if (outShapeAPacked != NULL) *outShapeAPacked = 0;
	if (outShapeBPacked != NULL) *outShapeBPacked = 0;
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3ContactEvents events = b3World_GetContactEvents(world->worldId);
	if (index < 0 || index >= events.beginCount) return 0;
	const b3ContactBeginTouchEvent* event = events.beginEvents + index;
	if (outShapeAPacked != NULL)
	{
		*outShapeAPacked = b3StoreShapeId(event->shapeIdA);
	}
	if (outShapeBPacked != NULL)
	{
		*outShapeBPacked = b3StoreShapeId(event->shapeIdB);
	}
	return 1;
}

B3W_EXPORT int b3wGetSensorBeginEventCount(int worldHandle)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return -1;
	b3SensorEvents events = b3World_GetSensorEvents(world->worldId);
	return events.beginCount;
}

B3W_EXPORT int b3wGetSensorBeginEvent(int worldHandle, int index, uint64_t* outSensorShapePacked, uint64_t* outVisitorShapePacked)
{
	if (outSensorShapePacked != NULL) *outSensorShapePacked = 0;
	if (outVisitorShapePacked != NULL) *outVisitorShapePacked = 0;
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3SensorEvents events = b3World_GetSensorEvents(world->worldId);
	if (index < 0 || index >= events.beginCount) return 0;
	const b3SensorBeginTouchEvent* event = events.beginEvents + index;
	if (outSensorShapePacked != NULL)
	{
		*outSensorShapePacked = b3StoreShapeId(event->sensorShapeId);
	}
	if (outVisitorShapePacked != NULL)
	{
		*outVisitorShapePacked = b3StoreShapeId(event->visitorShapeId);
	}
	return 1;
}

B3W_EXPORT int b3wGetSensorEndEventCount(int worldHandle)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return -1;
	b3SensorEvents events = b3World_GetSensorEvents(world->worldId);
	return events.endCount;
}

B3W_EXPORT int b3wGetSensorEndEvent(int worldHandle, int index, uint64_t* outSensorShapePacked, uint64_t* outVisitorShapePacked)
{
	if (outSensorShapePacked != NULL) *outSensorShapePacked = 0;
	if (outVisitorShapePacked != NULL) *outVisitorShapePacked = 0;
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3SensorEvents events = b3World_GetSensorEvents(world->worldId);
	if (index < 0 || index >= events.endCount) return 0;
	const b3SensorEndTouchEvent* event = events.endEvents + index;
	if (outSensorShapePacked != NULL)
	{
		*outSensorShapePacked = b3StoreShapeId(event->sensorShapeId);
	}
	if (outVisitorShapePacked != NULL)
	{
		*outVisitorShapePacked = b3StoreShapeId(event->visitorShapeId);
	}
	return 1;
}

static bool b3wSensorFilter(b3ShapeId idA, b3ShapeId idB, void* context)
{
	b3wWorldSlot* slot = (b3wWorldSlot*)context;
	intptr_t data = 0;
	int found = 0;
	if (b3Shape_IsSensor(idA))
	{
		data = (intptr_t)b3Shape_GetUserData(idA);
		found = 1;
	}
	else if (b3Shape_IsSensor(idB))
	{
		data = (intptr_t)b3Shape_GetUserData(idB);
		found = 1;
	}
	if (found == 0)
	{
		return true;
	}
	int active = (int)((data >> 30) & 1);
	int row = (int)(data & 0x3FFFFFFF);
	return active != 0 || row != slot->sensorFilterRow;
}

B3W_EXPORT void b3wSetCustomSensorFilter(int worldHandle, int filterRow, int enabled)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	slot->sensorFilterRow = filterRow;
	slot->sensorFilterEnabled = enabled != 0;
	if (enabled != 0)
	{
		b3World_SetCustomFilterCallback(slot->worldId, b3wSensorFilter, slot);
	}
	else
	{
		b3World_SetCustomFilterCallback(slot->worldId, NULL, NULL);
	}
}

B3W_EXPORT int b3wShapeIsSensor(uint64_t shapePacked)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return 0;
	return b3Shape_IsSensor(shapeId) ? 1 : 0;
}

B3W_EXPORT int b3wGetJointEventCount(int worldHandle)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return -1;
	b3JointEvents events = b3World_GetJointEvents(world->worldId);
	return events.count;
}

B3W_EXPORT uint64_t b3wGetJointEventHandle(int worldHandle, int index)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3JointEvents events = b3World_GetJointEvents(world->worldId);
	if (index < 0 || index >= events.count) return 0;
	b3JointId jointId = events.jointEvents[index].jointId;
	if (b3Joint_IsValid(jointId) == false) return 0;
	return b3StoreJointId(jointId);
}

#define B3W_EVENT_HEADER 3

B3W_EXPORT int b3wFillContactEvents(int worldHandle, uint64_t* out, int capacity)
{
	if (out == NULL || capacity < B3W_EVENT_HEADER) return 0;
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL)
	{
		out[0] = out[1] = out[2] = 0;
		return B3W_EVENT_HEADER;
	}
	b3ContactEvents events = b3World_GetContactEvents(world->worldId);
	int maxPairs = (capacity - B3W_EVENT_HEADER) / 2;
	int begin = events.beginCount;
	int end = events.endCount;
	int hit = events.hitCount;
	if (begin + end + hit > maxPairs)
	{
		int remain = maxPairs;
		if (begin > remain) begin = remain;
		remain -= begin;
		if (end > remain) end = remain;
		remain -= end;
		if (hit > remain) hit = remain;
	}
	out[0] = (uint64_t)begin;
	out[1] = (uint64_t)end;
	out[2] = (uint64_t)hit;
	int written = B3W_EVENT_HEADER;
	for (int i = 0; i < begin; ++i)
	{
		out[written++] = b3StoreShapeId(events.beginEvents[i].shapeIdA);
		out[written++] = b3StoreShapeId(events.beginEvents[i].shapeIdB);
	}
	for (int i = 0; i < end; ++i)
	{
		out[written++] = b3StoreShapeId(events.endEvents[i].shapeIdA);
		out[written++] = b3StoreShapeId(events.endEvents[i].shapeIdB);
	}
	for (int i = 0; i < hit; ++i)
	{
		out[written++] = b3StoreShapeId(events.hitEvents[i].shapeIdA);
		out[written++] = b3StoreShapeId(events.hitEvents[i].shapeIdB);
	}
	return written;
}

B3W_EXPORT int b3wFillSensorEvents(int worldHandle, uint64_t* out, int capacity)
{
	if (out == NULL || capacity < B3W_EVENT_HEADER) return 0;
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL)
	{
		out[0] = out[1] = out[2] = 0;
		return B3W_EVENT_HEADER;
	}
	b3SensorEvents events = b3World_GetSensorEvents(world->worldId);
	int maxPairs = (capacity - B3W_EVENT_HEADER) / 2;
	int begin = events.beginCount;
	int end = events.endCount;
	if (begin + end > maxPairs)
	{
		int remain = maxPairs;
		if (begin > remain) begin = remain;
		remain -= begin;
		if (end > remain) end = remain;
	}
	out[0] = (uint64_t)begin;
	out[1] = (uint64_t)end;
	out[2] = 0;
	int written = B3W_EVENT_HEADER;
	for (int i = 0; i < begin; ++i)
	{
		out[written++] = b3StoreShapeId(events.beginEvents[i].sensorShapeId);
		out[written++] = b3StoreShapeId(events.beginEvents[i].visitorShapeId);
	}
	for (int i = 0; i < end; ++i)
	{
		out[written++] = b3StoreShapeId(events.endEvents[i].sensorShapeId);
		out[written++] = b3StoreShapeId(events.endEvents[i].visitorShapeId);
	}
	return written;
}

B3W_EXPORT int b3wFillBodyContactData(uint64_t bodyPacked, uint64_t* out, int capacity)
{
	if (out == NULL || capacity < 1) return 0;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		out[0] = 0;
		return 1;
	}
	int maxPairs = (capacity - 1) / 2;
	if (maxPairs < 1)
	{
		out[0] = 0;
		return 1;
	}
	b3ContactData stack[32];
	int n = maxPairs < 32 ? maxPairs : 32;
	int count = b3Body_GetContactData(bodyId, stack, n);
	out[0] = (uint64_t)count;
	int written = 1;
	for (int i = 0; i < count && written + 2 <= capacity; ++i)
	{
		out[written++] = b3StoreShapeId(stack[i].shapeIdA);
		out[written++] = b3StoreShapeId(stack[i].shapeIdB);
	}
	return written;
}

#define B3W_CONTACT_POINT_FLOATS 8
#define B3W_CONTACT_MAX_POINTS 32

B3W_EXPORT int b3wFillBodyContactManifolds(uint64_t bodyPacked, float* out, int capacityFloats)
{
	if (out == NULL || capacityFloats < 2) return 0;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		out[0] = 0;
		out[1] = 0;
		return 2;
	}
	b3ContactData stack[16];
	int contactCount = b3Body_GetContactData(bodyId, stack, 16);
	b3Pos center = b3Body_GetWorldCenter(bodyId);
	int maxPoints = (capacityFloats - 2) / B3W_CONTACT_POINT_FLOATS;
	if (maxPoints > B3W_CONTACT_MAX_POINTS) maxPoints = B3W_CONTACT_MAX_POINTS;
	int points = 0;
	int written = 2;
	for (int c = 0; c < contactCount && points < maxPoints; ++c)
	{
		const b3Manifold* manifolds = stack[c].manifolds;
		int manifoldCount = stack[c].manifoldCount;
		if (manifolds == NULL) continue;
		for (int m = 0; m < manifoldCount && points < maxPoints; ++m)
		{
			const b3Manifold* man = manifolds + m;
			for (int p = 0; p < man->pointCount && points < maxPoints; ++p)
			{
				const b3ManifoldPoint* pt = man->points + p;
				out[written++] = center.x + pt->anchorA.x;
				out[written++] = center.y + pt->anchorA.y;
				out[written++] = center.z + pt->anchorA.z;
				out[written++] = man->normal.x;
				out[written++] = man->normal.y;
				out[written++] = man->normal.z;
				out[written++] = pt->separation;
				out[written++] = pt->totalNormalImpulse;
				points += 1;
			}
		}
	}
	out[0] = (float)contactCount;
	out[1] = (float)points;
	return written;
}
