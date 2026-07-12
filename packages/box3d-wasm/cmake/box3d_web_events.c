#include "box3d_web_shared.h"

#include <stdint.h>
#include <string.h>

// Open-addressing map: native body id key -> render index.
// Capacity is 2x max bodies so load stays reasonable when fully tracked.
// Empty slots use key == 0 (null body ids are never inserted).
#define B3W_MOVE_MAP_CAPACITY (B3W_MAX_BODIES * 2)
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
	if (key == 0 || g_moveMapCount >= B3W_MAX_BODIES)
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

B3W_EXPORT void b3wConfigureBodyMoveTracking(int count, const int* bodyHandles)
{
	b3wClearBodyMoveTracking();
	if (bodyHandles == NULL || count <= 0)
	{
		return;
	}

	int n = count;
	if (n > B3W_MAX_BODIES)
	{
		n = B3W_MAX_BODIES;
	}

	for (int i = 0; i < n; ++i)
	{
		b3wBodySlot* slot = b3wGetBody(bodyHandles[i]);
		if (slot == NULL)
		{
			continue;
		}
		uint64_t key = b3StoreBodyId(slot->bodyId);
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
