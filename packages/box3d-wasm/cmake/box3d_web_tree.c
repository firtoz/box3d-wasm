#include "box3d_web_shared.h"

#ifndef B3W_MAX_TREES
#define B3W_MAX_TREES 16
#endif

typedef struct b3wTreeSlot
{
	bool active;
	int nextFree;
	b3DynamicTree tree;
} b3wTreeSlot;

static b3wTreeSlot g_trees[B3W_MAX_TREES];
static int g_treeFreeHead = B3W_SLOT_FREE_NONE;
static int g_treeActiveCount = 0;
static bool g_treesReady = false;

static void b3wInitTrees(void)
{
	if (g_treesReady) return;
	for (int i = 0; i < B3W_MAX_TREES; ++i)
	{
		g_trees[i].active = false;
		g_trees[i].nextFree = (i + 1 < B3W_MAX_TREES) ? (i + 1) : B3W_SLOT_FREE_NONE;
	}
	g_treeFreeHead = 0;
	g_treeActiveCount = 0;
	g_treesReady = true;
}

static b3wTreeSlot* b3wGetTree(int handle)
{
	b3wInitTrees();
	if (handle <= 0 || handle > B3W_MAX_TREES) return NULL;
	b3wTreeSlot* slot = &g_trees[handle - 1];
	return slot->active ? slot : NULL;
}

B3W_EXPORT void b3wGetTreeSlotCounts(int* outUsed, int* outMax)
{
	b3wInitTrees();
	if (outUsed != NULL) *outUsed = g_treeActiveCount;
	if (outMax != NULL) *outMax = B3W_MAX_TREES;
}

B3W_EXPORT int b3wCreateDynamicTree(int proxyCapacity)
{
	b3wInitTrees();
	if (g_treeFreeHead == B3W_SLOT_FREE_NONE) return 0;
	int index = g_treeFreeHead;
	g_treeFreeHead = g_trees[index].nextFree;
	g_trees[index].active = true;
	g_trees[index].nextFree = B3W_SLOT_FREE_NONE;
	g_trees[index].tree = b3DynamicTree_Create(proxyCapacity > 0 ? proxyCapacity : 16);
	g_treeActiveCount += 1;
	return index + 1;
}

B3W_EXPORT void b3wDestroyDynamicTree(int handle)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return;
	b3DynamicTree_Destroy(&slot->tree);
	slot->active = false;
	slot->nextFree = g_treeFreeHead;
	g_treeFreeHead = handle - 1;
	g_treeActiveCount -= 1;
}

B3W_EXPORT int b3wTreeCreateProxy(int handle, float minX, float minY, float minZ, float maxX, float maxY, float maxZ,
	int categoryBits, int userData)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return -1;
	b3AABB aabb;
	aabb.lowerBound = (b3Vec3){ minX, minY, minZ };
	aabb.upperBound = (b3Vec3){ maxX, maxY, maxZ };
	return b3DynamicTree_CreateProxy(&slot->tree, aabb, (uint64_t)(uint32_t)categoryBits, (uint64_t)(uint32_t)userData);
}

B3W_EXPORT void b3wTreeDestroyProxy(int handle, int proxyId)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return;
	b3DynamicTree_DestroyProxy(&slot->tree, proxyId);
}

B3W_EXPORT void b3wTreeMoveProxy(int handle, int proxyId, float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return;
	b3AABB aabb;
	aabb.lowerBound = (b3Vec3){ minX, minY, minZ };
	aabb.upperBound = (b3Vec3){ maxX, maxY, maxZ };
	b3DynamicTree_MoveProxy(&slot->tree, proxyId, aabb);
}

typedef struct b3wTreeQueryContext
{
	int* out;
	int capacity;
	int count;
} b3wTreeQueryContext;

static bool b3wTreeQueryCallback(int proxyId, uint64_t userData, void* context)
{
	(void)userData;
	b3wTreeQueryContext* ctx = (b3wTreeQueryContext*)context;
	if (ctx->count >= ctx->capacity) return false;
	ctx->out[ctx->count++] = proxyId;
	return ctx->count < ctx->capacity;
}

B3W_EXPORT int b3wTreeQueryAABB(int handle, float minX, float minY, float minZ, float maxX, float maxY, float maxZ,
	int* outProxyIds, int maxCount, int* outNodeVisits, int* outLeafVisits)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL || outProxyIds == NULL || maxCount <= 0) return 0;
	b3AABB aabb;
	aabb.lowerBound = (b3Vec3){ minX, minY, minZ };
	aabb.upperBound = (b3Vec3){ maxX, maxY, maxZ };
	b3wTreeQueryContext ctx = { outProxyIds, maxCount, 0 };
	b3TreeStats stats = b3DynamicTree_Query(&slot->tree, aabb, ~0ull, false, b3wTreeQueryCallback, &ctx);
	if (outNodeVisits != NULL) *outNodeVisits = stats.nodeVisits;
	if (outLeafVisits != NULL) *outLeafVisits = stats.leafVisits;
	return ctx.count;
}

typedef struct b3wTreeRayContext
{
	int proxyId;
	float fraction;
} b3wTreeRayContext;

static float b3wTreeRayCallback(const b3RayCastInput* input, int proxyId, uint64_t userData, void* context)
{
	(void)input;
	(void)userData;
	b3wTreeRayContext* ctx = (b3wTreeRayContext*)context;
	ctx->proxyId = proxyId;
	ctx->fraction = 0.0f;
	return 0.0f;
}

B3W_EXPORT int b3wTreeRayCast(int handle, float ox, float oy, float oz, float tx, float ty, float tz, float maxFraction,
	int* outProxyId, float* outFraction)
{
	if (outProxyId != NULL) *outProxyId = -1;
	if (outFraction != NULL) *outFraction = 1.0f;
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return 0;
	b3RayCastInput input = { { ox, oy, oz }, { tx, ty, tz }, maxFraction };
	b3wTreeRayContext ctx = { -1, maxFraction };
	b3DynamicTree_RayCast(&slot->tree, &input, ~0ull, false, b3wTreeRayCallback, &ctx);
	if (ctx.proxyId < 0) return 0;
	if (outProxyId != NULL) *outProxyId = ctx.proxyId;
	if (outFraction != NULL) *outFraction = ctx.fraction;
	return 1;
}

B3W_EXPORT int b3wTreeGetHeight(int handle)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return 0;
	return b3DynamicTree_GetHeight(&slot->tree);
}

B3W_EXPORT float b3wTreeGetAreaRatio(int handle)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return 0.0f;
	return b3DynamicTree_GetAreaRatio(&slot->tree);
}

B3W_EXPORT int b3wTreeGetProxyCount(int handle)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return 0;
	return b3DynamicTree_GetProxyCount(&slot->tree);
}

B3W_EXPORT int b3wTreeRebuild(int handle, int fullBuild)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return 0;
	return b3DynamicTree_Rebuild(&slot->tree, fullBuild != 0);
}

B3W_EXPORT int b3wTreeGetByteCount(int handle)
{
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL) return 0;
	return b3DynamicTree_GetByteCount(&slot->tree);
}

B3W_EXPORT void b3wTreeGetRootBounds(int handle, float* out)
{
	if (out == NULL) return;
	b3wTreeSlot* slot = b3wGetTree(handle);
	if (slot == NULL)
	{
		out[0] = out[1] = out[2] = out[3] = out[4] = out[5] = 0;
		return;
	}
	b3AABB aabb = b3DynamicTree_GetRootBounds(&slot->tree);
	out[0] = aabb.lowerBound.x;
	out[1] = aabb.lowerBound.y;
	out[2] = aabb.lowerBound.z;
	out[3] = aabb.upperBound.x;
	out[4] = aabb.upperBound.y;
	out[5] = aabb.upperBound.z;
}
