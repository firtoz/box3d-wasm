#include "box3d_web_shared.h"

#include <stdint.h>
#include <stdlib.h>

B3W_EXPORT uint64_t b3wCreateBox(int worldHandle, float px, float py, float pz, float hx, float hy, float hz, int isStatic, float density)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.position = (b3Vec3){ px, py, pz };
	if (!isStatic) bodyDef.type = b3_dynamicBody;
	b3BodyId bodyId = b3CreateBody(world->worldId, &bodyDef);
	b3BoxHull hull = b3MakeBoxHull(hx, hy, hz);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density > 0.0f ? density : 1.0f;
	b3ShapeId shapeId = b3CreateHullShape(bodyId, &shapeDef, &hull.base);
	(void)shapeId;
	return b3StoreBodyId(bodyId);
}

B3W_EXPORT uint64_t b3wCreateSphere(int worldHandle, float px, float py, float pz, float radius, float vx, float vy, float vz, float density)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = (b3Vec3){ px, py, pz };
	bodyDef.linearVelocity = (b3Vec3){ vx, vy, vz };
	b3BodyId bodyId = b3CreateBody(world->worldId, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density > 0.0f ? density : 1.0f;
	b3Sphere sphere = { .center = { 0.0f, 0.0f, 0.0f }, .radius = radius };
	b3ShapeId shapeId = b3CreateSphereShape(bodyId, &shapeDef, &sphere);
	(void)shapeId;
	return b3StoreBodyId(bodyId);
}

B3W_EXPORT uint64_t b3wCreateHullShape(uint64_t bodyPacked, float density, float friction, float restitution, float rollingResistance, int updateBodyMass, float tx, float ty, float tz,
					   float qx, float qy, float qz, float qw, float hx, float hy, float hz, int isSensor)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	shapeDef.updateBodyMass = updateBodyMass != 0;
	shapeDef.isSensor = isSensor != 0;
	b3BoxHull hull = b3MakeBoxHull(hx, hy, hz);
	b3Transform transform = { { tx, ty, tz }, { { qx, qy, qz }, qw } };
	(void)transform;
	b3ShapeId shapeId = b3CreateHullShape(bodyId, &shapeDef, &hull.base);
	return b3StoreShapeId(shapeId);
}

B3W_EXPORT uint64_t b3wCreateTransformedHullShape(uint64_t bodyPacked, float density, float friction, float restitution, float rollingResistance,
								  float tx, float ty, float tz, float qx, float qy, float qz, float qw,
								  float hx, float hy, float hz, float sx, float sy, float sz)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	b3BoxHull hull = b3MakeBoxHull(hx, hy, hz);
	b3Transform transform = { { tx, ty, tz }, { { qx, qy, qz }, qw } };
	b3Vec3 scale = { sx, sy, sz };
	b3ShapeId shapeId = b3CreateTransformedHullShape(bodyId, &shapeDef, &hull.base, transform, scale);
	return b3StoreShapeId(shapeId);
}

/// Match upstream `b3MakeOffsetBoxHull` + `b3CreateHullShape` (offset baked into hull verts).
B3W_EXPORT uint64_t b3wCreateOffsetHullShape(uint64_t bodyPacked, float density, float friction, float restitution, float rollingResistance,
							 int updateBodyMass, float hx, float hy, float hz, float ox, float oy, float oz)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	shapeDef.updateBodyMass = updateBodyMass != 0;
	b3BoxHull hull = b3MakeOffsetBoxHull(hx, hy, hz, (b3Vec3){ ox, oy, oz });
	b3ShapeId shapeId = b3CreateHullShape(bodyId, &shapeDef, &hull.base);
	return b3StoreShapeId(shapeId);
}

B3W_EXPORT uint64_t b3wCreateSphereShape(uint64_t bodyPacked, float density, float friction, float restitution, float rollingResistance, float px, float py, float pz,
					    float radius, int invokeContactCreation)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	shapeDef.invokeContactCreation = invokeContactCreation != 0;
	b3Sphere sphere = { { px, py, pz }, radius };
	b3ShapeId shapeId = b3CreateSphereShape(bodyId, &shapeDef, &sphere);
	return b3StoreShapeId(shapeId);
}

B3W_EXPORT uint64_t b3wCreateCapsuleShape(uint64_t bodyPacked, float density, float friction, float restitution, float rollingResistance,
						  float ax, float ay, float az, float bx, float by, float bz, float radius, int isSensor)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	shapeDef.isSensor = isSensor != 0;
	b3Capsule capsule = { { ax, ay, az }, { bx, by, bz }, radius };
	b3ShapeId shapeId = b3CreateCapsuleShape(bodyId, &shapeDef, &capsule);
	return b3StoreShapeId(shapeId);
}

B3W_EXPORT uint64_t b3wCreateShapeFromHull(uint64_t bodyPacked, int hullHandle, float density, float friction, float restitution, float rollingResistance, int updateBodyMass, float explosionScale, int isSensor)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	b3wHullSlot* hull = b3wGetHull(hullHandle);
	if (!b3Body_IsValid(bodyId) || hull == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	shapeDef.updateBodyMass = updateBodyMass != 0;
	shapeDef.explosionScale = explosionScale;
	shapeDef.isSensor = isSensor != 0;
	b3ShapeId shapeId = b3CreateHullShape(bodyId, &shapeDef, hull->hull);
	return b3StoreShapeId(shapeId);
}

B3W_EXPORT uint64_t b3wCreateTransformedShapeFromHull(uint64_t bodyPacked, int hullHandle, float density, float friction, float restitution, float rollingResistance,
	int updateBodyMass, float tx, float ty, float tz, float qx, float qy, float qz, float qw, float sx, float sy, float sz)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	b3wHullSlot* hull = b3wGetHull(hullHandle);
	if (!b3Body_IsValid(bodyId) || hull == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	shapeDef.updateBodyMass = updateBodyMass != 0;
	b3Transform transform = { { tx, ty, tz }, { { qx, qy, qz }, qw } };
	b3Vec3 scale = { sx, sy, sz };
	b3ShapeId shapeId = b3CreateTransformedHullShape(bodyId, &shapeDef, hull->hull, transform, scale);
	return b3StoreShapeId(shapeId);
}

B3W_EXPORT void b3wShapeSetDensity(uint64_t shapePacked, float density, int updateBodyMass)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_SetDensity(shapeId, density, updateBodyMass != 0);
}

B3W_EXPORT void b3wShapeSetFriction(uint64_t shapePacked, float friction)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_SetFriction(shapeId, friction);
}

B3W_EXPORT void b3wShapeSetRestitution(uint64_t shapePacked, float restitution)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_SetRestitution(shapeId, restitution);
}

B3W_EXPORT void b3wShapeSetSurfaceMaterial(uint64_t shapePacked, float friction, float restitution, float rollingResistance,
	float tvx, float tvy, float tvz)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3SurfaceMaterial material = { 0 };
	material.friction = friction;
	material.restitution = restitution;
	material.rollingResistance = rollingResistance;
	material.tangentVelocity = (b3Vec3){ tvx, tvy, tvz };
	b3Shape_SetSurfaceMaterial(shapeId, material);
}

B3W_EXPORT void b3wShapeSetFilter(uint64_t shapePacked, int categoryBits, int maskBits, int groupIndex, int invokeContacts)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Filter filter = b3DefaultFilter();
	filter.categoryBits = (uint64_t)categoryBits;
	filter.maskBits = (uint64_t)maskBits;
	filter.groupIndex = groupIndex;
	b3Shape_SetFilter(shapeId, filter, invokeContacts != 0);
}

B3W_EXPORT uint64_t b3wGetShapeBodyHandle(uint64_t shapePacked)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return 0;
	b3BodyId bodyId = b3Shape_GetBody(shapeId);
	if (b3Body_IsValid(bodyId) == false) return 0;
	return b3StoreBodyId(bodyId);
}

B3W_EXPORT int b3wGetBodyShapeCount(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	return b3Body_GetShapeCount(bodyId);
}

B3W_EXPORT int b3wGetBodyShapes(uint64_t bodyPacked, uint64_t* outShapePackedIds, int capacity)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId) || outShapePackedIds == NULL || capacity <= 0) return 0;

	int count = b3Body_GetShapeCount(bodyId);
	if (count <= 0) return 0;
	if (count > capacity) count = capacity;

	b3ShapeId* shapeIds = malloc((size_t)count * sizeof(b3ShapeId));
	if (shapeIds == NULL) return 0;

	int written = b3Body_GetShapes(bodyId, shapeIds, count);
	for (int i = 0; i < written; ++i)
	{
		outShapePackedIds[i] = b3StoreShapeId(shapeIds[i]);
	}
	free(shapeIds);

	return written;
}

B3W_EXPORT void b3wDestroyShape(uint64_t shapePacked, int updateBodyMass)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3DestroyShape(shapeId, updateBodyMass != 0);
}

B3W_EXPORT void b3wShapeEnableSensorEvents(uint64_t shapePacked, int flag)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_EnableSensorEvents(shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeEnableContactEvents(uint64_t shapePacked, int flag)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_EnableContactEvents(shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeEnablePreSolveEvents(uint64_t shapePacked, int flag)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_EnablePreSolveEvents(shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeEnableHitEvents(uint64_t shapePacked, int flag)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_EnableHitEvents(shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeSetSphere(uint64_t shapePacked, float px, float py, float pz, float radius)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Sphere sphere = { { px, py, pz }, radius };
	b3Shape_SetSphere(shapeId, &sphere);
}

B3W_EXPORT void b3wShapeSetCapsule(uint64_t shapePacked, float ax, float ay, float az, float bx, float by, float bz, float radius)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Capsule capsule = { { ax, ay, az }, { bx, by, bz }, radius };
	b3Shape_SetCapsule(shapeId, &capsule);
}

B3W_EXPORT void b3wShapeApplyWind(uint64_t shapePacked, float windX, float windY, float windZ, float drag, float lift, float maxSpeed, int wake)
{
	b3ShapeId shapeId = b3LoadShapeId(shapePacked);
	if (!b3Shape_IsValid(shapeId)) return;
	b3Shape_ApplyWind(shapeId, (b3Vec3){ windX, windY, windZ }, drag, lift, maxSpeed, wake != 0);
}
