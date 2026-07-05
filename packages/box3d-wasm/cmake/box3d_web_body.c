#include "box3d_web_shared.h"
#include "body.h"
#include "shape.h"
#include "physics_world.h"

static b3HexColor GetBodyDebugColor( b3BodyId bodyId )
{
	b3World* world = b3GetWorld( bodyId.world0 );
	if ( world == NULL )
	{
		return b3_colorBlack;
	}

	b3Body* body = b3GetBodyFullId( world, bodyId );
	b3BodySim* bodySim = b3GetBodySim( world, body );

	// Check for customColor on any shape. If set, return it as-is so
	// user-assigned per-bone ragdoll colors survive the pipeline.
	{
		int shapeId = body->headShapeId;
		while ( shapeId != B3_NULL_INDEX )
		{
			b3Shape* shape = b3Array_Get( world->shapes, shapeId );
			const b3SurfaceMaterial* sm = b3GetShapeMaterials( shape );
			if ( sm[0].customColor != 0 )
			{
				// May already carry a packed material preset, pass through unchanged
				return (b3HexColor)sm[0].customColor;
			}
			shapeId = shape->nextShapeId;
		}
	}

	b3HexColor rgb;
	b3DebugMaterial material = b3_debugMaterialDefault;

	if ( body->type == b3_dynamicBody && body->mass == 0.0f )
	{
		rgb = b3_colorRed;
	}
	else if ( body->setIndex == b3_disabledSet )
	{
		rgb = b3_colorSlateGray;
	}
	else if ( body->flags & b3_hadTimeOfImpact )
	{
		rgb = b3_colorLime;
	}
	else if ( ( bodySim->flags & b3_isBullet ) && body->setIndex == b3_awakeSet )
	{
		rgb = b3_colorTurquoise;
	}
	else if ( body->flags & b3_isSpeedCapped )
	{
		rgb = b3_colorYellow;
	}
	else if ( bodySim->flags & b3_isFast )
	{
		rgb = b3_colorOrange;
		material = b3_debugMaterialGlossy;
	}
	else if ( body->type == b3_staticBody )
	{
		rgb = b3_colorDarkGray;
		material = b3_debugMaterialMatte;
	}
	else if ( body->type == b3_kinematicBody )
	{
		if ( body->setIndex == b3_awakeSet )
		{
			rgb = b3_colorSteelBlue;
			material = b3_debugMaterialMetallic;
		}
		else
		{
			rgb = b3_colorLightSteelBlue;
			material = b3_debugMaterialMatte;
		}
	}
	else if ( body->setIndex == b3_awakeSet )
	{
		rgb = b3_colorTan;
		material = b3_debugMaterialSoft;
	}
	else
	{
		rgb = b3_colorLightSlateGray;
		material = b3_debugMaterialDead;
	}

	return (b3HexColor)b3MakeDebugColor( rgb, material );
}

B3W_EXPORT int b3wCreateBody(int worldHandle, int bodyType, float px, float py, float pz, int enableSleep, int awake)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = (b3BodyType)bodyType;
	bodyDef.position = (b3Vec3){ px, py, pz };
	bodyDef.enableSleep = enableSleep != 0;
	bodyDef.isAwake = awake != 0;
	b3BodyId bodyId = b3CreateBody(world->worldId, &bodyDef);
	return b3wAllocBodySlot(worldHandle, bodyId);
}

B3W_EXPORT void b3wDestroyBody(int bodyHandle)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3DestroyBody(slot->bodyId);
	slot->active = false;
}

B3W_EXPORT void b3wSetBodyTransform(int bodyHandle, float px, float py, float pz, float qx, float qy, float qz, float qw)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetTransform(slot->bodyId, (b3Pos){ px, py, pz }, (b3Quat){ { qx, qy, qz }, qw });
}

B3W_EXPORT void b3wSetBodyLinearVelocity(int bodyHandle, float x, float y, float z)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetLinearVelocity(slot->bodyId, (b3Vec3){ x, y, z });
}

B3W_EXPORT void b3wSetBodyAngularVelocity(int bodyHandle, float x, float y, float z)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetAngularVelocity(slot->bodyId, (b3Vec3){ x, y, z });
}

B3W_EXPORT void b3wApplyLinearImpulse(int bodyHandle, float ix, float iy, float iz, float px, float py, float pz, int wake)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_ApplyLinearImpulse(slot->bodyId, (b3Vec3){ ix, iy, iz }, (b3Pos){ px, py, pz }, wake != 0);
}

B3W_EXPORT void b3wApplyLinearImpulseToCenter(int bodyHandle, float ix, float iy, float iz, int wake)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_ApplyLinearImpulseToCenter(slot->bodyId, (b3Vec3){ ix, iy, iz }, wake != 0);
}

B3W_EXPORT void b3wSetBodyAwake(int bodyHandle, int awake)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetAwake(slot->bodyId, awake != 0);
}

B3W_EXPORT void b3wSetBodyDamping(int bodyHandle, float linearDamping, float angularDamping)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetLinearDamping(slot->bodyId, linearDamping);
	b3Body_SetAngularDamping(slot->bodyId, angularDamping);
}

B3W_EXPORT void b3wSetBodyType(int bodyHandle, int type)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetType(slot->bodyId, (b3BodyType)type);
}

B3W_EXPORT void b3wSetBodyName(int bodyHandle, const char* name)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetName(slot->bodyId, name);
}

B3W_EXPORT void b3wSetBodyGravityScale(int bodyHandle, float scale)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetGravityScale(slot->bodyId, scale);
}

B3W_EXPORT void b3wSetBodySleepThreshold(int bodyHandle, float threshold)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetSleepThreshold(slot->bodyId, threshold);
}

B3W_EXPORT void b3wEnableBodySleep(int bodyHandle, int enableSleep)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_EnableSleep(slot->bodyId, enableSleep != 0);
}

B3W_EXPORT void b3wSetBodyBullet(int bodyHandle, int flag)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_SetBullet(slot->bodyId, flag != 0);
}

B3W_EXPORT void b3wEnableBodyContactRecycling(int bodyHandle, int flag)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_EnableContactRecycling(slot->bodyId, flag != 0);
}

B3W_EXPORT void b3wEnableBodyHitEvents(int bodyHandle, int flag)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_EnableHitEvents(slot->bodyId, flag != 0);
}

B3W_EXPORT void b3wSetBodyMotionLocks(int bodyHandle, int lockBodyX, int lockBodyY, int lockBodyRotationX, int lockBodyRotationY, int lockBodyRotationZ, int lockLinearZ)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3MotionLocks locks = { lockBodyX != 0, lockBodyY != 0, lockBodyRotationX != 0, lockBodyRotationY != 0, lockBodyRotationZ != 0, lockLinearZ != 0 };
	b3Body_SetMotionLocks(slot->bodyId, locks);
}

B3W_EXPORT void b3wSetBodyMassData(int bodyHandle, float mass, float cx, float cy, float cz)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3MassData massData;
	massData.mass = mass;
	massData.center = (b3Vec3){ cx, cy, cz };
	massData.inertia = b3Mat3_identity;
	b3Body_SetMassData(slot->bodyId, massData);
}

B3W_EXPORT void b3wGetBodyMassData(int bodyHandle, float* outMassData)
{
	if (outMassData == NULL) return;
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL)
	{
		outMassData[0] = 0.0f;
		outMassData[1] = 0.0f;
		return;
	}

	b3MassData massData = b3Body_GetMassData(slot->bodyId);
	outMassData[0] = massData.mass;
	outMassData[1] = massData.inertia.cx.x + massData.inertia.cy.y + massData.inertia.cz.z;
}

B3W_EXPORT void b3wApplyBodyMassFromShapes(int bodyHandle)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3Body_ApplyMassFromShapes(slot->bodyId);
}

B3W_EXPORT void b3wSetBodyTargetTransform(int bodyHandle, float px, float py, float pz, float qx, float qy, float qz, float qw, float timeStep, int wake)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return;
	b3WorldTransform target = { { px, py, pz }, { { qx, qy, qz }, qw } };
	b3Body_SetTargetTransform(slot->bodyId, target, timeStep, wake != 0);
}

B3W_EXPORT void b3wGetBodyLocalPoint(int bodyHandle, float worldX, float worldY, float worldZ, float* outPoint)
{
	if (outPoint == NULL) return;
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
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

B3W_EXPORT void b3wGetBodyTransform(int bodyHandle, float* outTransform)
{
	if (outTransform == NULL) return;
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
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

B3W_EXPORT uint32_t b3wGetBodyDebugColor(int bodyHandle)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return 0;
	return (uint32_t)GetBodyDebugColor(slot->bodyId);
}

B3W_EXPORT int b3wBodyIsAwake(int bodyHandle)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return 0;
	return b3Body_IsAwake(slot->bodyId) ? 1 : 0;
}

B3W_EXPORT int b3wGetBodyType(int bodyHandle)
{
	b3wBodySlot* slot = b3wGetBody(bodyHandle);
	if (slot == NULL) return 0;
	return (int)b3Body_GetType(slot->bodyId);
}

B3W_EXPORT void b3wWriteBodyTransforms(int count, const int* bodyHandles, float* outPositions, float* outRotations, char* outAwake, uint32_t* outColors)
{
	for (int i = 0; i < count; ++i)
	{
		int handle = bodyHandles[i];
		b3wBodySlot* slot = b3wGetBody(handle);
		if (slot == NULL)
		{
			outPositions[i * 3 + 0] = 0.0f;
			outPositions[i * 3 + 1] = 0.0f;
			outPositions[i * 3 + 2] = 0.0f;
			outRotations[i * 4 + 0] = 0.0f;
			outRotations[i * 4 + 1] = 0.0f;
			outRotations[i * 4 + 2] = 0.0f;
			outRotations[i * 4 + 3] = 1.0f;
			outAwake[i] = 0;
			outColors[i] = 0;
			continue;
		}
		b3Vec3 position = b3Body_GetPosition(slot->bodyId);
		outPositions[i * 3 + 0] = position.x;
		outPositions[i * 3 + 1] = position.y;
		outPositions[i * 3 + 2] = position.z;
		b3Quat rotation = b3Body_GetRotation(slot->bodyId);
		outRotations[i * 4 + 0] = rotation.v.x;
		outRotations[i * 4 + 1] = rotation.v.y;
		outRotations[i * 4 + 2] = rotation.v.z;
		outRotations[i * 4 + 3] = rotation.s;
		outAwake[i] = b3Body_IsAwake(slot->bodyId) ? 1 : 0;
		outColors[i] = (uint32_t)GetBodyDebugColor( slot->bodyId );
	}
}

B3W_EXPORT void b3wWriteBodyTransformsLight(int count, const int* bodyHandles, float* outPositions, float* outRotations, char* outAwake, uint32_t* outColors)
{
	for (int i = 0; i < count; ++i)
	{
		int handle = bodyHandles[i];
		b3wBodySlot* slot = b3wGetBody(handle);
		if (slot == NULL)
		{
			outPositions[i * 3 + 0] = 0.0f;
			outPositions[i * 3 + 1] = 0.0f;
			outPositions[i * 3 + 2] = 0.0f;
			outRotations[i * 4 + 0] = 0.0f;
			outRotations[i * 4 + 1] = 0.0f;
			outRotations[i * 4 + 2] = 0.0f;
			outRotations[i * 4 + 3] = 1.0f;
			outAwake[i] = 0;
			outColors[i] = 0;
			continue;
		}
		b3Vec3 position = b3Body_GetPosition(slot->bodyId);
		outPositions[i * 3 + 0] = position.x;
		outPositions[i * 3 + 1] = position.y;
		outPositions[i * 3 + 2] = position.z;
		b3Quat rotation = b3Body_GetRotation(slot->bodyId);
		outRotations[i * 4 + 0] = rotation.v.x;
		outRotations[i * 4 + 1] = rotation.v.y;
		outRotations[i * 4 + 2] = rotation.v.z;
		outRotations[i * 4 + 3] = rotation.s;
		outAwake[i] = b3Body_IsAwake(slot->bodyId) ? 1 : 0;
		outColors[i] = outAwake[i] ? 0xd2b48c : 0x778899;
	}
}
