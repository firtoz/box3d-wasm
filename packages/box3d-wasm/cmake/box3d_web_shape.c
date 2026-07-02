#include "box3d_web_shared.h"

static b3ShapeId* resolve_shape(int shapeHandle)
{
	b3wShapeSlot* slot = b3wGetShape(shapeHandle);
	return slot ? &slot->shapeId : NULL;
}

B3W_EXPORT int b3wCreateBox(int worldHandle, float px, float py, float pz, float hx, float hy, float hz, int isStatic, float density)
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
	return b3wAllocBodySlot(worldHandle, bodyId);
}

B3W_EXPORT int b3wCreateSphere(int worldHandle, float px, float py, float pz, float radius, float vx, float vy, float vz, float density)
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
	return b3wAllocBodySlot(worldHandle, bodyId);
}

B3W_EXPORT int b3wCreateHullShape(int bodyHandle, float density, float friction, float restitution, float rollingResistance, float tx, float ty, float tz,
					   float qx, float qy, float qz, float qw, float hx, float hy, float hz)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	b3BoxHull hull = b3MakeBoxHull(hx, hy, hz);
	b3Transform transform = { { tx, ty, tz }, { { qx, qy, qz }, qw } };
	(void)transform;
	b3ShapeId shapeId = b3CreateHullShape(slot->bodyId, &shapeDef, &hull.base);
	return b3wAllocShapeSlot(shapeId);
}

B3W_EXPORT int b3wCreateTransformedHullShape(int bodyHandle, float density, float friction, float restitution, float rollingResistance,
								  float tx, float ty, float tz, float qx, float qy, float qz, float qw,
								  float hx, float hy, float hz, float sx, float sy, float sz)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	b3BoxHull hull = b3MakeBoxHull(hx, hy, hz);
	b3Transform transform = { { tx, ty, tz }, { { qx, qy, qz }, qw } };
	b3Vec3 scale = { sx, sy, sz };
	b3ShapeId shapeId = b3CreateTransformedHullShape(slot->bodyId, &shapeDef, &hull.base, transform, scale);
	return b3wAllocShapeSlot(shapeId);
}

B3W_EXPORT int b3wCreateSphereShape(int bodyHandle, float density, float friction, float restitution, float rollingResistance, float px, float py, float pz,
					    float radius)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	b3Sphere sphere = { { px, py, pz }, radius };
	b3ShapeId shapeId = b3CreateSphereShape(slot->bodyId, &shapeDef, &sphere);
	return b3wAllocShapeSlot(shapeId);
}

B3W_EXPORT int b3wCreateCapsuleShape(int bodyHandle, float density, float friction, float restitution, float rollingResistance,
						  float ax, float ay, float az, float bx, float by, float bz, float radius)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	b3Capsule capsule = { { ax, ay, az }, { bx, by, bz }, radius };
	b3ShapeId shapeId = b3CreateCapsuleShape(slot->bodyId, &shapeDef, &capsule);
	return b3wAllocShapeSlot(shapeId);
}

B3W_EXPORT int b3wCreateShapeFromHull(int bodyHandle, int hullHandle, float density, float friction, float restitution, float rollingResistance)
{
	b3wBodySlot* body = b3wGetBody(bodyHandle);
	b3wHullSlot* hull = b3wGetHull(hullHandle);
	if (body == NULL || hull == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	b3ShapeId shapeId = b3CreateHullShape(body->bodyId, &shapeDef, hull->hull);
	return b3wAllocShapeSlot(shapeId);
}

B3W_EXPORT void b3wShapeSetDensity(int shapeHandle, float density, int updateBodyMass)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_SetDensity(*shapeId, density, updateBodyMass != 0);
}

B3W_EXPORT void b3wShapeSetFriction(int shapeHandle, float friction)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_SetFriction(*shapeId, friction);
}

B3W_EXPORT void b3wShapeSetRestitution(int shapeHandle, float restitution)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_SetRestitution(*shapeId, restitution);
}

B3W_EXPORT void b3wShapeSetSurfaceMaterial(int shapeHandle, float friction, float restitution, float rollingResistance)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3SurfaceMaterial material = { 0 };
	material.friction = friction;
	material.restitution = restitution;
	material.rollingResistance = rollingResistance;
	b3Shape_SetSurfaceMaterial(*shapeId, material);
}

B3W_EXPORT void b3wShapeSetFilter(int shapeHandle, int categoryBits, int maskBits, int groupIndex, int invokeContacts)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Filter filter = b3DefaultFilter();
	filter.categoryBits = (uint64_t)categoryBits;
	filter.maskBits = (uint64_t)maskBits;
	filter.groupIndex = groupIndex;
	b3Shape_SetFilter(*shapeId, filter, invokeContacts != 0);
}

B3W_EXPORT int b3wGetShapeBodyHandle(int shapeHandle)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return 0;
	b3BodyId bodyId = b3Shape_GetBody(*shapeId);
	if (b3Body_IsValid(bodyId) == false) return 0;
	return bodyId.index1;
}

B3W_EXPORT void b3wShapeEnableSensorEvents(int shapeHandle, int flag)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_EnableSensorEvents(*shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeEnableContactEvents(int shapeHandle, int flag)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_EnableContactEvents(*shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeEnablePreSolveEvents(int shapeHandle, int flag)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_EnablePreSolveEvents(*shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeEnableHitEvents(int shapeHandle, int flag)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_EnableHitEvents(*shapeId, flag != 0);
}

B3W_EXPORT void b3wShapeSetSphere(int shapeHandle, float px, float py, float pz, float radius)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Sphere sphere = { { px, py, pz }, radius };
	b3Shape_SetSphere(*shapeId, &sphere);
}

B3W_EXPORT void b3wShapeSetCapsule(int shapeHandle, float ax, float ay, float az, float bx, float by, float bz, float radius)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Capsule capsule = { { ax, ay, az }, { bx, by, bz }, radius };
	b3Shape_SetCapsule(*shapeId, &capsule);
}

B3W_EXPORT void b3wShapeApplyWind(int shapeHandle, float windX, float windY, float windZ, float drag, float lift, float maxSpeed, int wake)
{
	b3ShapeId* shapeId = resolve_shape(shapeHandle);
	if (shapeId == NULL) return;
	b3Shape_ApplyWind(*shapeId, (b3Vec3){ windX, windY, windZ }, drag, lift, maxSpeed, wake != 0);
}
