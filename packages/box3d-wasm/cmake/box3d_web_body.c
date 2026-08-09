#include "box3d_web_shared.h"
#include "body.h"
#include "shape.h"
#include "physics_world.h"

b3HexColor b3wGetBodyDebugColorForId( b3BodyId bodyId )
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

B3W_EXPORT uint64_t b3wCreateBody(int worldHandle, int bodyType, float px, float py, float pz, int enableSleep, int awake)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = (b3BodyType)bodyType;
	bodyDef.position = (b3Vec3){ px, py, pz };
	bodyDef.enableSleep = enableSleep != 0;
	bodyDef.isAwake = awake != 0;
	b3BodyId bodyId = b3CreateBody(world->worldId, &bodyDef);
	return b3StoreBodyId(bodyId);
}

B3W_EXPORT void b3wDestroyBody(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3DestroyBody(bodyId);
}

B3W_EXPORT void b3wSetBodyTransform(uint64_t bodyPacked, float px, float py, float pz, float qx, float qy, float qz, float qw)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetTransform(bodyId, (b3Pos){ px, py, pz }, (b3Quat){ { qx, qy, qz }, qw });
}

B3W_EXPORT void b3wSetBodyLinearVelocity(uint64_t bodyPacked, float x, float y, float z)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetLinearVelocity(bodyId, (b3Vec3){ x, y, z });
}

B3W_EXPORT void b3wSetBodyAngularVelocity(uint64_t bodyPacked, float x, float y, float z)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetAngularVelocity(bodyId, (b3Vec3){ x, y, z });
}

B3W_EXPORT void b3wGetBodyLinearVelocity(uint64_t bodyPacked, float* outVelocity)
{
	if (outVelocity == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outVelocity[0] = 0.0f; outVelocity[1] = 0.0f; outVelocity[2] = 0.0f;
		return;
	}
	b3Vec3 v = b3Body_GetLinearVelocity(bodyId);
	outVelocity[0] = v.x; outVelocity[1] = v.y; outVelocity[2] = v.z;
}

B3W_EXPORT void b3wGetBodyAngularVelocity(uint64_t bodyPacked, float* outVelocity)
{
	if (outVelocity == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outVelocity[0] = 0.0f; outVelocity[1] = 0.0f; outVelocity[2] = 0.0f;
		return;
	}
	b3Vec3 v = b3Body_GetAngularVelocity(bodyId);
	outVelocity[0] = v.x; outVelocity[1] = v.y; outVelocity[2] = v.z;
}

B3W_EXPORT void b3wApplyLinearImpulse(uint64_t bodyPacked, float ix, float iy, float iz, float px, float py, float pz, int wake)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_ApplyLinearImpulse(bodyId, (b3Vec3){ ix, iy, iz }, (b3Pos){ px, py, pz }, wake != 0);
}

B3W_EXPORT void b3wApplyLinearImpulseToCenter(uint64_t bodyPacked, float ix, float iy, float iz, int wake)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_ApplyLinearImpulseToCenter(bodyId, (b3Vec3){ ix, iy, iz }, wake != 0);
}

B3W_EXPORT void b3wSetBodyAwake(uint64_t bodyPacked, int awake)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetAwake(bodyId, awake != 0);
}

B3W_EXPORT void b3wSetBodyDamping(uint64_t bodyPacked, float linearDamping, float angularDamping)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetLinearDamping(bodyId, linearDamping);
	b3Body_SetAngularDamping(bodyId, angularDamping);
}

B3W_EXPORT void b3wSetBodyType(uint64_t bodyPacked, int type)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetType(bodyId, (b3BodyType)type);
}

B3W_EXPORT void b3wBodyEnable(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_Enable(bodyId);
}

B3W_EXPORT void b3wBodyDisable(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_Disable(bodyId);
}

B3W_EXPORT int b3wBodyIsEnabled(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	return b3Body_IsEnabled(bodyId) ? 1 : 0;
}

B3W_EXPORT float b3wGetBodyMass(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0.0f;
	return b3Body_GetMass(bodyId);
}

B3W_EXPORT void b3wGetBodyLocalRotationalInertia(uint64_t bodyPacked, float* outInertia)
{
	if (outInertia == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		for (int i = 0; i < 9; ++i) outInertia[i] = 0.0f;
		return;
	}
	b3Matrix3 inertia = b3Body_GetLocalRotationalInertia(bodyId);
	outInertia[0] = inertia.cx.x; outInertia[1] = inertia.cx.y; outInertia[2] = inertia.cx.z;
	outInertia[3] = inertia.cy.x; outInertia[4] = inertia.cy.y; outInertia[5] = inertia.cy.z;
	outInertia[6] = inertia.cz.x; outInertia[7] = inertia.cz.y; outInertia[8] = inertia.cz.z;
}

B3W_EXPORT void b3wGetBodyWorldCenter(uint64_t bodyPacked, float* outPoint)
{
	if (outPoint == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outPoint[0] = 0.0f; outPoint[1] = 0.0f; outPoint[2] = 0.0f;
		return;
	}
	b3Pos center = b3Body_GetWorldCenter(bodyId);
	outPoint[0] = center.x; outPoint[1] = center.y; outPoint[2] = center.z;
}

B3W_EXPORT void b3wGetBodyWorldPoint(uint64_t bodyPacked, float lx, float ly, float lz, float* outPoint)
{
	if (outPoint == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outPoint[0] = 0.0f; outPoint[1] = 0.0f; outPoint[2] = 0.0f;
		return;
	}
	b3Pos worldPoint = b3Body_GetWorldPoint(bodyId, (b3Vec3){ lx, ly, lz });
	outPoint[0] = worldPoint.x; outPoint[1] = worldPoint.y; outPoint[2] = worldPoint.z;
}

B3W_EXPORT void b3wGetBodyLocalPointVelocity(uint64_t bodyPacked, float lx, float ly, float lz, float* outVelocity)
{
	if (outVelocity == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outVelocity[0] = 0.0f; outVelocity[1] = 0.0f; outVelocity[2] = 0.0f;
		return;
	}
	b3Vec3 v = b3Body_GetLocalPointVelocity(bodyId, (b3Vec3){ lx, ly, lz });
	outVelocity[0] = v.x; outVelocity[1] = v.y; outVelocity[2] = v.z;
}

B3W_EXPORT void b3wGetBodyWorldPointVelocity(uint64_t bodyPacked, float wx, float wy, float wz, float* outVelocity)
{
	if (outVelocity == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outVelocity[0] = 0.0f; outVelocity[1] = 0.0f; outVelocity[2] = 0.0f;
		return;
	}
	b3Vec3 v = b3Body_GetWorldPointVelocity(bodyId, (b3Pos){ wx, wy, wz });
	outVelocity[0] = v.x; outVelocity[1] = v.y; outVelocity[2] = v.z;
}

B3W_EXPORT void b3wSetBodyName(uint64_t bodyPacked, const char* name)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetName(bodyId, name);
}

B3W_EXPORT void b3wSetBodyGravityScale(uint64_t bodyPacked, float scale)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetGravityScale(bodyId, scale);
}

B3W_EXPORT void b3wSetBodySleepThreshold(uint64_t bodyPacked, float threshold)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetSleepThreshold(bodyId, threshold);
}

B3W_EXPORT void b3wEnableBodySleep(uint64_t bodyPacked, int enableSleep)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_EnableSleep(bodyId, enableSleep != 0);
}

B3W_EXPORT void b3wSetBodyBullet(uint64_t bodyPacked, int flag)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_SetBullet(bodyId, flag != 0);
}

B3W_EXPORT void b3wAllowBodyFastRotation(uint64_t bodyPacked, int flag)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_AllowFastRotation(bodyId, flag != 0);
}

B3W_EXPORT int b3wIsBodyFastRotationAllowed(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	return b3Body_IsFastRotationAllowed(bodyId) ? 1 : 0;
}

B3W_EXPORT void b3wEnableBodyContactRecycling(uint64_t bodyPacked, int flag)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_EnableContactRecycling(bodyId, flag != 0);
}

B3W_EXPORT void b3wEnableBodyHitEvents(uint64_t bodyPacked, int flag)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_EnableHitEvents(bodyId, flag != 0);
}

B3W_EXPORT void b3wSetBodyMotionLocks(uint64_t bodyPacked, int lockLinearX, int lockLinearY, int lockLinearZ, int lockAngularX, int lockAngularY, int lockAngularZ)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3MotionLocks locks = { lockLinearX != 0, lockLinearY != 0, lockLinearZ != 0, lockAngularX != 0, lockAngularY != 0, lockAngularZ != 0 };
	b3Body_SetMotionLocks(bodyId, locks);
}

B3W_EXPORT void b3wSetBodyMassData(uint64_t bodyPacked, float mass, float cx, float cy, float cz, float* inertia)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3MassData massData;
	massData.mass = mass;
	massData.center = (b3Vec3){ cx, cy, cz };
	if (inertia != NULL)
	{
		massData.inertia = (b3Matrix3){
			{ inertia[0], inertia[1], inertia[2] },
			{ inertia[3], inertia[4], inertia[5] },
			{ inertia[6], inertia[7], inertia[8] } };
	}
	else
	{
		massData.inertia = b3Mat3_identity;
	}
	b3Body_SetMassData(bodyId, massData);
}

B3W_EXPORT void b3wGetBodyMassData(uint64_t bodyPacked, float* outMassData)
{
	if (outMassData == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outMassData[0] = 0.0f;
		outMassData[1] = 0.0f;
		return;
	}

	b3MassData massData = b3Body_GetMassData(bodyId);
	outMassData[0] = massData.mass;
	outMassData[1] = massData.inertia.cx.x + massData.inertia.cy.y + massData.inertia.cz.z;
}

B3W_EXPORT void b3wApplyBodyMassFromShapes(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3Body_ApplyMassFromShapes(bodyId);
}

B3W_EXPORT void b3wSetBodyTargetTransform(uint64_t bodyPacked, float px, float py, float pz, float qx, float qy, float qz, float qw, float timeStep, int wake)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return;
	b3WorldTransform target = { { px, py, pz }, { { qx, qy, qz }, qw } };
	b3Body_SetTargetTransform(bodyId, target, timeStep, wake != 0);
}

B3W_EXPORT void b3wGetBodyLocalPoint(uint64_t bodyPacked, float worldX, float worldY, float worldZ, float* outPoint)
{
	if (outPoint == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outPoint[0] = 0.0f;
		outPoint[1] = 0.0f;
		outPoint[2] = 0.0f;
		return;
	}
	b3Vec3 localPoint = b3Body_GetLocalPoint(bodyId, (b3Pos){ worldX, worldY, worldZ });
	outPoint[0] = localPoint.x;
	outPoint[1] = localPoint.y;
	outPoint[2] = localPoint.z;
}

B3W_EXPORT void b3wGetBodyTransform(uint64_t bodyPacked, float* outTransform)
{
	if (outTransform == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		for (int i = 0; i < 7; ++i) outTransform[i] = 0.0f;
		return;
	}
	b3Vec3 position = b3Body_GetPosition(bodyId);
	b3Quat rotation = b3Body_GetRotation(bodyId);
	outTransform[0] = position.x;
	outTransform[1] = position.y;
	outTransform[2] = position.z;
	outTransform[3] = rotation.v.x;
	outTransform[4] = rotation.v.y;
	outTransform[5] = rotation.v.z;
	outTransform[6] = rotation.s;
}

B3W_EXPORT uint32_t b3wGetBodyDebugColor(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	return (uint32_t)b3wGetBodyDebugColorForId(bodyId);
}

B3W_EXPORT int b3wBodyIsAwake(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	return b3Body_IsAwake(bodyId) ? 1 : 0;
}

B3W_EXPORT int b3wGetBodyType(uint64_t bodyPacked)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	return (int)b3Body_GetType(bodyId);
}

B3W_EXPORT void b3wWriteBodyTransforms(int count, const uint64_t* bodyPackedIds, float* outPositions, float* outRotations, char* outAwake, uint32_t* outColors)
{
	for (int i = 0; i < count; ++i)
	{
		b3BodyId bodyId = b3LoadBodyId(bodyPackedIds[i]);
		if (!b3Body_IsValid(bodyId))
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
		b3WorldTransform xf = b3Body_GetTransform(bodyId);
		outPositions[i * 3 + 0] = xf.p.x;
		outPositions[i * 3 + 1] = xf.p.y;
		outPositions[i * 3 + 2] = xf.p.z;
		outRotations[i * 4 + 0] = xf.q.v.x;
		outRotations[i * 4 + 1] = xf.q.v.y;
		outRotations[i * 4 + 2] = xf.q.v.z;
		outRotations[i * 4 + 3] = xf.q.s;
		outAwake[i] = b3Body_IsAwake(bodyId) ? 1 : 0;
		outColors[i] = (uint32_t)b3wGetBodyDebugColorForId( bodyId );
	}
}

B3W_EXPORT void b3wWriteBodyTransformsLight(int count, const uint64_t* bodyPackedIds, float* outPositions, float* outRotations, char* outAwake, uint32_t* outColors)
{
	for (int i = 0; i < count; ++i)
	{
		b3BodyId bodyId = b3LoadBodyId(bodyPackedIds[i]);
		if (!b3Body_IsValid(bodyId))
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
		b3WorldTransform xf = b3Body_GetTransform(bodyId);
		outPositions[i * 3 + 0] = xf.p.x;
		outPositions[i * 3 + 1] = xf.p.y;
		outPositions[i * 3 + 2] = xf.p.z;
		outRotations[i * 4 + 0] = xf.q.v.x;
		outRotations[i * 4 + 1] = xf.q.v.y;
		outRotations[i * 4 + 2] = xf.q.v.z;
		outRotations[i * 4 + 3] = xf.q.s;
		outAwake[i] = b3Body_IsAwake(bodyId) ? 1 : 0;
		outColors[i] = outAwake[i] ? 0xd2b48c : 0x778899;
	}
}

B3W_EXPORT int b3wBodyIsValid(uint64_t bodyPacked)
{
	return b3Body_IsValid(b3LoadBodyId(bodyPacked)) ? 1 : 0;
}

B3W_EXPORT int b3wBodyCastRay(uint64_t bodyPacked, float originX, float originY, float originZ, float translationX,
	float translationY, float translationZ, int categoryBits, int maskBits, float maxFraction,
	float bodyPx, float bodyPy, float bodyPz, float bodyQx, float bodyQy, float bodyQz, float bodyQw,
	int* outHit, float* outPoint, float* outNormal, float* outFraction)
{
	if (outHit != NULL) *outHit = 0;
	if (outFraction != NULL) *outFraction = 1.0f;
	if (outPoint != NULL) { outPoint[0] = outPoint[1] = outPoint[2] = 0.0f; }
	if (outNormal != NULL) { outNormal[0] = outNormal[1] = outNormal[2] = 0.0f; }
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId)) return 0;
	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.categoryBits = (uint64_t)categoryBits;
	filter.maskBits = (uint64_t)maskBits;
	b3WorldTransform bodyTransform = { { bodyPx, bodyPy, bodyPz }, { { bodyQx, bodyQy, bodyQz }, bodyQw } };
	b3BodyCastResult result = b3Body_CastRay(bodyId, (b3Pos){ originX, originY, originZ },
		(b3Vec3){ translationX, translationY, translationZ }, filter, maxFraction, bodyTransform);
	if (result.hit == false) return 0;
	if (outHit != NULL) *outHit = 1;
	if (outFraction != NULL) *outFraction = result.fraction;
	if (outPoint != NULL)
	{
		outPoint[0] = result.point.x;
		outPoint[1] = result.point.y;
		outPoint[2] = result.point.z;
	}
	if (outNormal != NULL)
	{
		outNormal[0] = result.normal.x;
		outNormal[1] = result.normal.y;
		outNormal[2] = result.normal.z;
	}
	return 1;
}
