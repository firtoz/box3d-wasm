#include "box3d_web_shared.h"

B3W_EXPORT int b3wCreateMotorJoint(
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
	int collideConnected,
	float linearHertz,
	float linearDampingRatio,
	float maxSpringForce,
	float angularHertz,
	float angularDampingRatio,
	float maxSpringTorque)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3wBodySlot* bodyA = b3wGetBody(bodyAHandle);
	b3wBodySlot* bodyB = b3wGetBody(bodyBHandle);
	if (world == NULL || bodyA == NULL || bodyB == NULL) return 0;
	b3MotorJointDef jointDef = b3DefaultMotorJointDef();
	jointDef.base.bodyIdA = bodyA->bodyId;
	jointDef.base.bodyIdB = bodyB->bodyId;
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
	return b3wAllocJointSlot(bodyA->worldHandle, jointId);
}

B3W_EXPORT int b3wCreateFilterJoint(int worldHandle, int bodyAHandle, int bodyBHandle)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3wBodySlot* bodyA = b3wGetBody(bodyAHandle);
	b3wBodySlot* bodyB = b3wGetBody(bodyBHandle);
	if (world == NULL || bodyA == NULL || bodyB == NULL) return 0;
	b3FilterJointDef jointDef = b3DefaultFilterJointDef();
	jointDef.base.bodyIdA = bodyA->bodyId;
	jointDef.base.bodyIdB = bodyB->bodyId;
	b3JointId jointId = b3CreateFilterJoint(world->worldId, &jointDef);
	return b3wAllocJointSlot(bodyA->worldHandle, jointId);
}

B3W_EXPORT int b3wCreateRevoluteJoint(
	int worldHandle,
	int bodyAHandle,
	int bodyBHandle,
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
	float motorSpeed)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3wBodySlot* bodyA = b3wGetBody(bodyAHandle);
	b3wBodySlot* bodyB = b3wGetBody(bodyBHandle);
	if (world == NULL || bodyA == NULL || bodyB == NULL) return 0;
	b3RevoluteJointDef jointDef = b3DefaultRevoluteJointDef();
	jointDef.base.bodyIdA = bodyA->bodyId;
	jointDef.base.bodyIdB = bodyB->bodyId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.base.constraintHertz = constraintHertz;
	jointDef.base.constraintDampingRatio = constraintDampingRatio;
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
	return b3wAllocJointSlot(bodyA->worldHandle, jointId);
}

B3W_EXPORT int b3wCreateSphericalJoint(
	int worldHandle,
	int bodyAHandle,
	int bodyBHandle,
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
	b3wBodySlot* bodyA = b3wGetBody(bodyAHandle);
	b3wBodySlot* bodyB = b3wGetBody(bodyBHandle);
	if (world == NULL || bodyA == NULL || bodyB == NULL) return 0;
	b3SphericalJointDef jointDef = b3DefaultSphericalJointDef();
	jointDef.base.bodyIdA = bodyA->bodyId;
	jointDef.base.bodyIdB = bodyB->bodyId;
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
	return b3wAllocJointSlot(bodyA->worldHandle, jointId);
}

B3W_EXPORT int b3wCreatePrismaticJoint(
	int worldHandle,
	int bodyAHandle,
	int bodyBHandle,
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
	float targetTranslation,
	int enableLimit,
	float lowerTranslation,
	float upperTranslation,
	int enableMotor,
	float maxMotorForce,
	float motorSpeed)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3wBodySlot* bodyA = b3wGetBody(bodyAHandle);
	b3wBodySlot* bodyB = b3wGetBody(bodyBHandle);
	if (world == NULL || bodyA == NULL || bodyB == NULL) return 0;
	b3PrismaticJointDef jointDef = b3DefaultPrismaticJointDef();
	jointDef.base.bodyIdA = bodyA->bodyId;
	jointDef.base.bodyIdB = bodyB->bodyId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
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
	return b3wAllocJointSlot(bodyA->worldHandle, jointId);
}

B3W_EXPORT int b3wCreateWeldJoint(
	int worldHandle,
	int bodyAHandle,
	int bodyBHandle,
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
	float angularDampingRatio)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	b3wBodySlot* bodyA = b3wGetBody(bodyAHandle);
	b3wBodySlot* bodyB = b3wGetBody(bodyBHandle);
	if (world == NULL || bodyA == NULL || bodyB == NULL) return 0;
	b3WeldJointDef jointDef = b3DefaultWeldJointDef();
	jointDef.base.bodyIdA = bodyA->bodyId;
	jointDef.base.bodyIdB = bodyB->bodyId;
	jointDef.base.localFrameA = (b3Transform){ { localAx, localAy, localAz }, { { localAqx, localAqy, localAqz }, localAqw } };
	jointDef.base.localFrameB = (b3Transform){ { localBx, localBy, localBz }, { { localBqx, localBqy, localBqz }, localBqw } };
	jointDef.linearHertz = linearHertz;
	jointDef.angularHertz = angularHertz;
	jointDef.linearDampingRatio = linearDampingRatio;
	jointDef.angularDampingRatio = angularDampingRatio;
	b3JointId jointId = b3CreateWeldJoint(world->worldId, &jointDef);
	return b3wAllocJointSlot(bodyA->worldHandle, jointId);
}

B3W_EXPORT void b3wDestroyJoint(int jointHandle)
{
	if (jointHandle <= 0 || jointHandle > B3W_MAX_JOINTS) return;
	b3wJointSlot* slot = &g_joints[jointHandle - 1];
	if (!slot->active) return;
	b3DestroyJoint(slot->jointId, true);
	slot->active = false;
}

B3W_EXPORT void b3wGetJointConstraintForce(int jointHandle, float* outForce)
{
	if (outForce == NULL || jointHandle <= 0 || jointHandle > B3W_MAX_JOINTS) return;
	b3wJointSlot* slot = &g_joints[jointHandle - 1];
	if (!slot->active) return;
	b3Vec3 force = b3Joint_GetConstraintForce(slot->jointId);
	outForce[0] = force.x;
	outForce[1] = force.y;
	outForce[2] = force.z;
}

B3W_EXPORT void b3wGetJointConstraintTorque(int jointHandle, float* outTorque)
{
	if (outTorque == NULL || jointHandle <= 0 || jointHandle > B3W_MAX_JOINTS) return;
	b3wJointSlot* slot = &g_joints[jointHandle - 1];
	if (!slot->active) return;
	b3Vec3 torque = b3Joint_GetConstraintTorque(slot->jointId);
	outTorque[0] = torque.x;
	outTorque[1] = torque.y;
	outTorque[2] = torque.z;
}

B3W_EXPORT float b3wGetJointLinearSeparation(int jointHandle)
{
	if (jointHandle <= 0 || jointHandle > B3W_MAX_JOINTS) return 0.0f;
	b3wJointSlot* slot = &g_joints[jointHandle - 1];
	if (!slot->active) return 0.0f;
	return b3Joint_GetLinearSeparation(slot->jointId);
}

B3W_EXPORT void b3wRevoluteJointSetTargetAngle(int jointHandle, float targetRadians)
{
	if (jointHandle <= 0 || jointHandle > B3W_MAX_JOINTS) return;
	b3wJointSlot* slot = &g_joints[jointHandle - 1];
	if (!slot->active) return;
	b3RevoluteJoint_SetTargetAngle(slot->jointId, targetRadians);
}
