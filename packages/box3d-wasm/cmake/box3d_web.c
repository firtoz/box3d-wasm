#include "box3d/box3d.h"

#include <stdbool.h>
#include <stddef.h>

enum
{
	B3W_MAX_WORLDS = 8,
	B3W_MAX_BODIES = 256,
	B3W_MAX_JOINTS = 256,
};

typedef struct b3wWorldSlot { bool active; b3WorldId worldId; } b3wWorldSlot;
typedef struct b3wBodySlot { bool active; int worldHandle; b3BodyId bodyId; } b3wBodySlot;
typedef struct b3wJointSlot { bool active; int worldHandle; b3JointId jointId; } b3wJointSlot;

static b3wWorldSlot g_worlds[B3W_MAX_WORLDS];
static b3wBodySlot g_bodies[B3W_MAX_BODIES];
static b3wJointSlot g_joints[B3W_MAX_JOINTS];

static int alloc_handle(void* slots, int count, size_t stride)
{
	for (int i = 0; i < count; ++i)
	{
		bool* active = (bool*)((char*)slots + (size_t)i * stride);
		if (!*active)
		{
			return i + 1;
		}
	}
	return 0;
}

static b3wWorldSlot* get_world(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_WORLDS) return NULL;
	b3wWorldSlot* slot = &g_worlds[handle - 1];
	return slot->active ? slot : NULL;
}

static b3wBodySlot* get_body(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_BODIES) return NULL;
	b3wBodySlot* slot = &g_bodies[handle - 1];
	return slot->active ? slot : NULL;
}

static int register_body(int worldHandle, b3BodyId bodyId)
{
	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		if (!g_bodies[i].active)
		{
			g_bodies[i].active = true;
			g_bodies[i].worldHandle = worldHandle;
			g_bodies[i].bodyId = bodyId;
			return i + 1;
		}
	}
	b3DestroyBody(bodyId);
	return 0;
}

static int register_joint(int worldHandle, b3JointId jointId)
{
	for (int i = 0; i < B3W_MAX_JOINTS; ++i)
	{
		if (!g_joints[i].active)
		{
			g_joints[i].active = true;
			g_joints[i].worldHandle = worldHandle;
			g_joints[i].jointId = jointId;
			return i + 1;
		}
	}
	b3DestroyJoint(jointId, true);
	return 0;
}

int b3wCreateWorld(float gravityX, float gravityY, float gravityZ)
{
	b3WorldDef def = b3DefaultWorldDef();
	def.gravity = (b3Vec3){ gravityX, gravityY, gravityZ };
	b3WorldId worldId = b3CreateWorld(&def);
	for (int i = 0; i < B3W_MAX_WORLDS; ++i)
	{
		if (!g_worlds[i].active)
		{
			g_worlds[i].active = true;
			g_worlds[i].worldId = worldId;
			return i + 1;
		}
	}
	b3DestroyWorld(worldId);
	return 0;
}

void b3wDestroyWorld(int worldHandle)
{
	b3wWorldSlot* slot = get_world(worldHandle);
	if (slot == NULL) return;
	b3DestroyWorld(slot->worldId);
	slot->active = false;
}

int b3wCreateBox(int worldHandle, float px, float py, float pz, float hx, float hy, float hz, int isStatic, float density)
{
	b3wWorldSlot* world = get_world(worldHandle);
	if (world == NULL) return 0;
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.position = (b3Vec3){ px, py, pz };
	if (!isStatic) bodyDef.type = b3_dynamicBody;
	b3BodyId bodyId = b3CreateBody(world->worldId, &bodyDef);
	b3BoxHull hull = b3MakeBoxHull(hx, hy, hz);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density > 0.0f ? density : 1.0f;
	b3CreateHullShape(bodyId, &shapeDef, &hull.base);
	return register_body(worldHandle, bodyId);
}

int b3wCreateSphere(int worldHandle, float px, float py, float pz, float radius, float vx, float vy, float vz, float density)
{
	b3wWorldSlot* world = get_world(worldHandle);
	if (world == NULL) return 0;
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = (b3Vec3){ px, py, pz };
	bodyDef.linearVelocity = (b3Vec3){ vx, vy, vz };
	b3BodyId bodyId = b3CreateBody(world->worldId, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density > 0.0f ? density : 1.0f;
	b3Sphere sphere = { .center = { 0.0f, 0.0f, 0.0f }, .radius = radius };
	b3CreateSphereShape(bodyId, &shapeDef, &sphere);
	return register_body(worldHandle, bodyId);
}

void b3wDestroyBody(int bodyHandle)
{
	b3wBodySlot* slot = get_body(bodyHandle);
	if (slot == NULL) return;
	b3DestroyBody(slot->bodyId);
	slot->active = false;
}

int b3wCreateBody(int worldHandle, int bodyType, float px, float py, float pz, int enableSleep, int awake)
{
	b3wWorldSlot* world = get_world(worldHandle);
	if (world == NULL) return 0;
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = (b3BodyType)bodyType;
	bodyDef.position = (b3Vec3){ px, py, pz };
	bodyDef.enableSleep = enableSleep != 0;
	bodyDef.isAwake = awake != 0;
	b3BodyId bodyId = b3CreateBody(world->worldId, &bodyDef);
	return register_body(worldHandle, bodyId);
}

void b3wSetBodyTransform(int bodyHandle, float px, float py, float pz, float qx, float qy, float qz, float qw)
{
	b3wBodySlot* slot = get_body(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetTransform(slot->bodyId, (b3Pos){ px, py, pz }, (b3Quat){ { qx, qy, qz }, qw });
}

void b3wSetBodyAwake(int bodyHandle, int awake)
{
	b3wBodySlot* slot = get_body(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetAwake(slot->bodyId, awake != 0);
}

void b3wSetBodyDamping(int bodyHandle, float linearDamping, float angularDamping)
{
	b3wBodySlot* slot = get_body(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetLinearDamping(slot->bodyId, linearDamping);
	b3Body_SetAngularDamping(slot->bodyId, angularDamping);
}

void b3wGetBodyLocalPoint(int bodyHandle, float worldX, float worldY, float worldZ, float* outPoint)
{
	if (outPoint == NULL) return;
	b3wBodySlot* slot = get_body(bodyHandle);
	if (slot == NULL)
	{
		outPoint[0] = 0.0f;
		outPoint[1] = 0.0f;
		outPoint[2] = 0.0f;
		return;
	}
	b3Vec3 localPoint = b3Body_GetLocalPoint(slot->bodyId, (b3Pos){ worldX, worldY, worldZ });
	outPoint[0] = localPoint.x;
	outPoint[1] = localPoint.y;
	outPoint[2] = localPoint.z;
}

int b3wCreateMotorJoint(
	int worldHandle,
	int bodyAHandle,
	int bodyBHandle,
	float localAx,
	float localAy,
	float localAz,
	float localBx,
	float localBy,
	float localBz,
	float linearVx,
	float linearVy,
	float linearVz,
	float maxVelocityForce,
	float angularVx,
	float angularVy,
	float angularVz,
	float maxVelocityTorque,
	float linearHertz,
	float linearDampingRatio,
	float maxSpringForce,
	float angularHertz,
	float angularDampingRatio,
	float maxSpringTorque)
{
	b3wWorldSlot* world = get_world(worldHandle);
	b3wBodySlot* bodyA = get_body(bodyAHandle);
	b3wBodySlot* bodyB = get_body(bodyBHandle);
	if (world == NULL || bodyA == NULL || bodyB == NULL) return 0;
	b3MotorJointDef jointDef = b3DefaultMotorJointDef();
	jointDef.base.bodyIdA = bodyA->bodyId;
	jointDef.base.bodyIdB = bodyB->bodyId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, b3Quat_identity };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, b3Quat_identity };
	jointDef.linearVelocity = (b3Vec3){ linearVx, linearVy, linearVz };
	jointDef.angularVelocity = (b3Vec3){ angularVx, angularVy, angularVz };
	jointDef.maxVelocityForce = maxVelocityForce;
	jointDef.maxVelocityTorque = maxVelocityTorque;
	jointDef.linearHertz = linearHertz;
	jointDef.linearDampingRatio = linearDampingRatio;
	jointDef.maxSpringForce = maxSpringForce;
	jointDef.angularHertz = angularHertz;
	jointDef.angularDampingRatio = angularDampingRatio;
	jointDef.maxSpringTorque = maxSpringTorque;
	b3JointId jointId = b3CreateMotorJoint(world->worldId, &jointDef);
	return register_joint(bodyA->worldHandle, jointId);
}

void b3wDestroyJoint(int jointHandle)
{
	if (jointHandle <= 0 || jointHandle > B3W_MAX_JOINTS) return;
	b3wJointSlot* slot = &g_joints[jointHandle - 1];
	if (!slot->active) return;
	b3DestroyJoint(slot->jointId, true);
	slot->active = false;
}

void b3wStep(int worldHandle, float timeStep, int subStepCount)
{
	b3wWorldSlot* slot = get_world(worldHandle);
	if (slot == NULL) return;
	b3World_Step(slot->worldId, timeStep, subStepCount);
}

void b3wGetBodyTransform(int bodyHandle, float* outTransform)
{
	if (outTransform == NULL) return;
	b3wBodySlot* slot = get_body(bodyHandle);
	if (slot == NULL)
	{
		for (int i = 0; i < 7; ++i) outTransform[i] = 0.0f;
		return;
	}
	b3Vec3 position = b3Body_GetPosition(slot->bodyId);
	b3Quat rotation = b3Body_GetRotation(slot->bodyId);
	outTransform[0] = position.x;
	outTransform[1] = position.y;
	outTransform[2] = position.z;
	outTransform[3] = rotation.v.x;
	outTransform[4] = rotation.v.y;
	outTransform[5] = rotation.v.z;
	outTransform[6] = rotation.s;
}

int main(void) { return 0; }
