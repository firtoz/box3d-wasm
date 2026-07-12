#include "box3d_web_shared.h"

B3W_EXPORT int b3wCreateGridMesh(int worldHandle, int xCount, int zCount, float cellWidth, int materialCount, int identifyEdges)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3MeshData* mesh = b3CreateGridMesh(xCount, zCount, cellWidth, materialCount, identifyEdges != 0);
	if (mesh == NULL) return 0;
	return b3wAllocMeshSlot(worldHandle, mesh);
}

B3W_EXPORT int b3wCreateTorusMesh(int worldHandle, int radialResolution, int tubularResolution, float radius, float thickness)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3MeshData* mesh = b3CreateTorusMesh(radialResolution, tubularResolution, radius, thickness);
	if (mesh == NULL) return 0;
	return b3wAllocMeshSlot(worldHandle, mesh);
}

B3W_EXPORT int b3wCreateWaveMesh(int worldHandle, int xCount, int zCount, float cellWidth, float amplitude, float rowFrequency, float columnFrequency)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3MeshData* mesh = b3CreateWaveMesh(xCount, zCount, cellWidth, amplitude, rowFrequency, columnFrequency);
	if (mesh == NULL) return 0;
	return b3wAllocMeshSlot(worldHandle, mesh);
}

B3W_EXPORT int b3wCreateBoxMesh(int worldHandle, float cx, float cy, float cz, float ex, float ey, float ez, int identifyEdges)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3Vec3 center = { cx, cy, cz };
	b3Vec3 extent = { ex, ey, ez };
	b3MeshData* mesh = b3CreateBoxMesh(center, extent, identifyEdges != 0);
	if (mesh == NULL) return 0;
	return b3wAllocMeshSlot(worldHandle, mesh);
}

B3W_EXPORT void b3wDestroyMesh(int meshHandle)
{
	b3wMeshSlot* slot = b3wGetMesh(meshHandle);
	if (slot == NULL) return;
	b3DestroyMesh(slot->mesh);
	slot->active = false;
	slot->worldHandle = 0;
	slot->mesh = NULL;
}

B3W_EXPORT int b3wCreateMeshShape(int bodyHandle, int meshHandle, float density, float friction, float restitution, float rollingResistance,
	float sx, float sy, float sz)
{
	b3wBodySlot* body = b3wGetBody(bodyHandle);
	b3wMeshSlot* mesh = b3wGetMesh(meshHandle);
	if (body == NULL || mesh == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	b3Vec3 scale = { sx, sy, sz };
	b3ShapeId shapeId = b3CreateMeshShape(body->bodyId, &shapeDef, mesh->mesh, scale);
	return b3wAllocShapeSlot(shapeId);
}

B3W_EXPORT void b3wShapeSetMesh(int shapeHandle, int meshHandle, float sx, float sy, float sz)
{
	b3wShapeSlot* shape = b3wGetShape(shapeHandle);
	b3wMeshSlot* mesh = b3wGetMesh(meshHandle);
	if (shape == NULL || mesh == NULL) return;
	b3Vec3 scale = { sx, sy, sz };
	b3Shape_SetMesh(shape->shapeId, mesh->mesh, scale);
}
