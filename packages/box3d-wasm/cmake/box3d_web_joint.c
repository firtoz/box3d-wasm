#include "box3d_web_shared.h"

B3W_EXPORT uint64_t b3wCreateMotorJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
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
	int collideConnected,
	float linearHertz,
	float linearDampingRatio,
	float maxSpringForce,
	float angularHertz,
	float angularDampingRatio,
	float maxSpringTorque)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3MotorJointDef jointDef = b3DefaultMotorJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, b3Quat_identity };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, b3Quat_identity };
	jointDef.base.collideConnected = collideConnected != 0;
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
	return b3StoreJointId(jointId);
}

B3W_EXPORT uint64_t b3wCreateFilterJoint(int worldHandle, uint64_t bodyAPacked, uint64_t bodyBPacked)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3FilterJointDef jointDef = b3DefaultFilterJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	b3JointId jointId = b3CreateFilterJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT uint64_t b3wCreateRevoluteJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
	float localAx,
	float localAy,
	float localAz,
	float localAqx,
	float localAqy,
	float localAqz,
	float localAqw,
	float localBx,
	float localBy,
	float localBz,
	float localBqx,
	float localBqy,
	float localBqz,
	float localBqw,
	float constraintHertz,
	float constraintDampingRatio,
	float targetAngle,
	int enableSpring,
	float hertz,
	float dampingRatio,
	int enableLimit,
	float lowerAngle,
	float upperAngle,
	int enableMotor,
	float maxMotorTorque,
	float motorSpeed,
	float forceThreshold,
	float torqueThreshold,
	int collideConnected)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3RevoluteJointDef jointDef = b3DefaultRevoluteJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.base.constraintHertz = constraintHertz;
	jointDef.base.constraintDampingRatio = constraintDampingRatio;
	jointDef.base.forceThreshold = forceThreshold;
	jointDef.base.torqueThreshold = torqueThreshold;
	jointDef.base.collideConnected = collideConnected != 0;
	jointDef.targetAngle = targetAngle;
	jointDef.enableSpring = enableSpring != 0;
	jointDef.hertz = hertz;
	jointDef.dampingRatio = dampingRatio;
	jointDef.enableLimit = enableLimit != 0;
	jointDef.lowerAngle = lowerAngle;
	jointDef.upperAngle = upperAngle;
	jointDef.enableMotor = enableMotor != 0;
	jointDef.maxMotorTorque = maxMotorTorque;
	jointDef.motorSpeed = motorSpeed;
	b3JointId jointId = b3CreateRevoluteJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT uint64_t b3wCreateSphericalJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
	float localAx,
	float localAy,
	float localAz,
	float localAqx,
	float localAqy,
	float localAqz,
	float localAqw,
	float localBx,
	float localBy,
	float localBz,
	float localBqx,
	float localBqy,
	float localBqz,
	float localBqw,
	int enableSpring,
	float hertz,
	float dampingRatio,
	float targetQx,
	float targetQy,
	float targetQz,
	float targetQw,
	int enableConeLimit,
	float coneAngle,
	int enableTwistLimit,
	float lowerTwistAngle,
	float upperTwistAngle,
	int enableMotor,
	float maxMotorTorque,
	float motorVx,
	float motorVy,
	float motorVz)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3SphericalJointDef jointDef = b3DefaultSphericalJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.enableSpring = enableSpring != 0;
	jointDef.hertz = hertz;
	jointDef.dampingRatio = dampingRatio;
	jointDef.targetRotation = (b3Quat){ { targetQx, targetQy, targetQz }, targetQw };
	jointDef.enableConeLimit = enableConeLimit != 0;
	jointDef.coneAngle = coneAngle;
	jointDef.enableTwistLimit = enableTwistLimit != 0;
	jointDef.lowerTwistAngle = lowerTwistAngle;
	jointDef.upperTwistAngle = upperTwistAngle;
	jointDef.enableMotor = enableMotor != 0;
	jointDef.maxMotorTorque = maxMotorTorque;
	jointDef.motorVelocity = (b3Vec3){ motorVx, motorVy, motorVz };
	b3JointId jointId = b3CreateSphericalJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT uint64_t b3wCreatePrismaticJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
	float localAx,
	float localAy,
	float localAz,
	float localAqx,
	float localAqy,
	float localAqz,
	float localAqw,
	float localBx,
	float localBy,
	float localBz,
	float localBqx,
	float localBqy,
	float localBqz,
	float localBqw,
	float constraintHertz,
	float constraintDampingRatio,
	int enableSpring,
	float hertz,
	float dampingRatio,
	float targetTranslation,
	int enableLimit,
	float lowerTranslation,
	float upperTranslation,
	int enableMotor,
	float maxMotorForce,
	float motorSpeed,
	float forceThreshold,
	float torqueThreshold,
	int collideConnected)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3PrismaticJointDef jointDef = b3DefaultPrismaticJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.base.constraintHertz = constraintHertz;
	jointDef.base.constraintDampingRatio = constraintDampingRatio;
	jointDef.base.forceThreshold = forceThreshold;
	jointDef.base.torqueThreshold = torqueThreshold;
	jointDef.base.collideConnected = collideConnected != 0;
	jointDef.enableSpring = enableSpring != 0;
	jointDef.hertz = hertz;
	jointDef.dampingRatio = dampingRatio;
	jointDef.targetTranslation = targetTranslation;
	jointDef.enableLimit = enableLimit != 0;
	jointDef.lowerTranslation = lowerTranslation;
	jointDef.upperTranslation = upperTranslation;
	jointDef.enableMotor = enableMotor != 0;
	jointDef.maxMotorForce = maxMotorForce;
	jointDef.motorSpeed = motorSpeed;
	b3JointId jointId = b3CreatePrismaticJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT uint64_t b3wCreateWeldJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
	float localAx,
	float localAy,
	float localAz,
	float localAqx,
	float localAqy,
	float localAqz,
	float localAqw,
	float localBx,
	float localBy,
	float localBz,
	float localBqx,
	float localBqy,
	float localBqz,
	float localBqw,
	float linearHertz,
	float angularHertz,
	float linearDampingRatio,
	float angularDampingRatio,
	float forceThreshold,
	float torqueThreshold,
	int collideConnected)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3WeldJointDef jointDef = b3DefaultWeldJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.base.forceThreshold = forceThreshold;
	jointDef.base.torqueThreshold = torqueThreshold;
	jointDef.base.collideConnected = collideConnected != 0;
	jointDef.linearHertz = linearHertz;
	jointDef.angularHertz = angularHertz;
	jointDef.linearDampingRatio = linearDampingRatio;
	jointDef.angularDampingRatio = angularDampingRatio;
	b3JointId jointId = b3CreateWeldJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT uint64_t b3wCreateDistanceJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
	float localAx,
	float localAy,
	float localAz,
	float localAqx,
	float localAqy,
	float localAqz,
	float localAqw,
	float localBx,
	float localBy,
	float localBz,
	float localBqx,
	float localBqy,
	float localBqz,
	float localBqw,
	float length,
	float forceThreshold,
	float torqueThreshold,
	int collideConnected)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3DistanceJointDef jointDef = b3DefaultDistanceJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.base.forceThreshold = forceThreshold;
	jointDef.base.torqueThreshold = torqueThreshold;
	jointDef.base.collideConnected = collideConnected != 0;
	jointDef.length = length;
	b3JointId jointId = b3CreateDistanceJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT uint64_t b3wCreateParallelJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
	float localAx,
	float localAy,
	float localAz,
	float localAqx,
	float localAqy,
	float localAqz,
	float localAqw,
	float localBx,
	float localBy,
	float localBz,
	float localBqx,
	float localBqy,
	float localBqz,
	float localBqw,
	float hertz,
	float dampingRatio,
	float maxTorque,
	float forceThreshold,
	float torqueThreshold,
	int collideConnected)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3ParallelJointDef jointDef = b3DefaultParallelJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.base.forceThreshold = forceThreshold;
	jointDef.base.torqueThreshold = torqueThreshold;
	jointDef.base.collideConnected = collideConnected != 0;
	jointDef.hertz = hertz;
	jointDef.dampingRatio = dampingRatio;
	jointDef.maxTorque = maxTorque;
	b3JointId jointId = b3CreateParallelJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT void b3wDestroyJoint(uint64_t jointPacked)
{
	b3JointId jointId = b3LoadJointId(jointPacked);
	if (!b3Joint_IsValid(jointId)) return;
	b3DestroyJoint(jointId, true);
}

B3W_EXPORT void b3wGetJointConstraintForce(uint64_t jointPacked, float* outForce)
{
	if (outForce == NULL) return;
	b3JointId jointId = b3LoadJointId(jointPacked);
	if (!b3Joint_IsValid(jointId)) return;
	b3Vec3 force = b3Joint_GetConstraintForce(jointId);
	outForce[0] = force.x;
	outForce[1] = force.y;
	outForce[2] = force.z;
}

B3W_EXPORT void b3wGetJointConstraintTorque(uint64_t jointPacked, float* outTorque)
{
	if (outTorque == NULL) return;
	b3JointId jointId = b3LoadJointId(jointPacked);
	if (!b3Joint_IsValid(jointId)) return;
	b3Vec3 torque = b3Joint_GetConstraintTorque(jointId);
	outTorque[0] = torque.x;
	outTorque[1] = torque.y;
	outTorque[2] = torque.z;
}

B3W_EXPORT float b3wGetJointLinearSeparation(uint64_t jointPacked)
{
	b3JointId jointId = b3LoadJointId(jointPacked);
	if (!b3Joint_IsValid(jointId)) return 0.0f;
	return b3Joint_GetLinearSeparation(jointId);
}

B3W_EXPORT uint64_t b3wCreateWheelJoint(
	int worldHandle,
	uint64_t bodyAPacked,
	uint64_t bodyBPacked,
	float localAx, float localAy, float localAz,
	float localAqx, float localAqy, float localAqz, float localAqw,
	float localBx, float localBy, float localBz,
	float localBqx, float localBqy, float localBqz, float localBqw,
	int enableSuspensionSpring,
	float suspensionHertz,
	float suspensionDampingRatio,
	int enableSuspensionLimit,
	float lowerSuspensionLimit,
	float upperSuspensionLimit,
	int enableSpinMotor,
	float maxSpinTorque,
	float spinSpeed,
	int enableSteering,
	float steeringHertz,
	float steeringDampingRatio,
	float targetSteeringAngle,
	float maxSteeringTorque,
	int enableSteeringLimit,
	float lowerSteeringLimit,
	float upperSteeringLimit,
	int collideConnected)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3BodyId bodyAId = b3LoadBodyId(bodyAPacked);
	b3BodyId bodyBId = b3LoadBodyId(bodyBPacked);
	if (world == NULL || !b3Body_IsValid(bodyAId) || !b3Body_IsValid(bodyBId)) return 0;
	b3WheelJointDef jointDef = b3DefaultWheelJointDef();
	jointDef.base.bodyIdA = bodyAId;
	jointDef.base.bodyIdB = bodyBId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.base.collideConnected = collideConnected != 0;
	jointDef.enableSuspensionSpring = enableSuspensionSpring != 0;
	jointDef.suspensionHertz = suspensionHertz;
	jointDef.suspensionDampingRatio = suspensionDampingRatio;
	jointDef.enableSuspensionLimit = enableSuspensionLimit != 0;
	jointDef.lowerSuspensionLimit = lowerSuspensionLimit;
	jointDef.upperSuspensionLimit = upperSuspensionLimit;
	jointDef.enableSpinMotor = enableSpinMotor != 0;
	jointDef.maxSpinTorque = maxSpinTorque;
	jointDef.spinSpeed = spinSpeed;
	jointDef.enableSteering = enableSteering != 0;
	jointDef.steeringHertz = steeringHertz;
	jointDef.steeringDampingRatio = steeringDampingRatio;
	jointDef.targetSteeringAngle = targetSteeringAngle;
	jointDef.maxSteeringTorque = maxSteeringTorque;
	jointDef.enableSteeringLimit = enableSteeringLimit != 0;
	jointDef.lowerSteeringLimit = lowerSteeringLimit;
	jointDef.upperSteeringLimit = upperSteeringLimit;
	b3JointId jointId = b3CreateWheelJoint(world->worldId, &jointDef);
	return b3StoreJointId(jointId);
}

B3W_EXPORT void b3wRevoluteJointSetTargetAngle(uint64_t jointPacked, float targetRadians)
{
	b3JointId jointId = b3LoadJointId(jointPacked);
	if (!b3Joint_IsValid(jointId)) return;
	b3RevoluteJoint_SetTargetAngle(jointId, targetRadians);
}

B3W_EXPORT void b3wPrismaticJointSetMotorSpeed(uint64_t jointPacked, float motorSpeed)
{
	b3JointId jointId = b3LoadJointId(jointPacked);
	if (!b3Joint_IsValid(jointId)) return;
	b3PrismaticJoint_SetMotorSpeed(jointId, motorSpeed);
}

B3W_EXPORT float b3wPrismaticJointGetTranslation(uint64_t jointPacked)
{
	b3JointId jointId = b3LoadJointId(jointPacked);
	if (!b3Joint_IsValid(jointId)) return 0.0f;
	return b3PrismaticJoint_GetTranslation(jointId);
}
