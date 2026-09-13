// Generated stubs for Box3D APIs the GPU engine does not implement yet.
// Linked after gpu_physics + samples_api so those definitions win.
#include "box3d/box3d.h"
#include <stddef.h>

B3_API void b3World_SetRestitutionThreshold(b3WorldId worldId, float value)
{
	(void)sizeof(char);
}

B3_API float b3World_GetRestitutionThreshold(b3WorldId worldId)
{
	(void)sizeof(char);
	return 0;
}

B3_API void b3World_SetGravity(b3WorldId worldId, b3Vec3 gravity)
{
	(void)sizeof(char);
}

B3_API void b3World_SetContactTuning(b3WorldId worldId, float hertz, float dampingRatio, float contactSpeed)
{
	(void)sizeof(char);
}

B3_API void b3World_SetMaximumLinearSpeed(b3WorldId worldId, float maximumLinearSpeed)
{
	(void)sizeof(char);
}

B3_API float b3World_GetMaximumLinearSpeed(b3WorldId worldId)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3World_GetAwakeBodyCount(b3WorldId worldId)
{
	(void)sizeof(char);
	return 0;
}

B3_API void b3World_SetUserData(b3WorldId worldId, void* userData)
{
	(void)sizeof(char);
}

B3_API void* b3World_GetUserData(b3WorldId worldId)
{
	(void)sizeof(char);
	return NULL;
}

B3_API void b3World_SetWorkerCount(b3WorldId worldId, int count)
{
	(void)sizeof(char);
}

B3_API int b3World_GetWorkerCount(b3WorldId worldId)
{
	(void)sizeof(char);
	return 0;
}

B3_API void b3World_DumpShapeBounds(b3WorldId worldId, b3BodyType type)
{
	(void)sizeof(char);
}

B3_API void b3World_RebuildStaticTree(b3WorldId worldId)
{
	(void)sizeof(char);
}

B3_API void b3World_EnableSpeculative(b3WorldId worldId, bool flag)
{
	(void)sizeof(char);
}

B3_API const uint8_t* b3Recording_GetData(const b3Recording* recording)
{
	(void)sizeof(char);
	return NULL;
}

B3_API int b3Recording_GetSize(const b3Recording* recording)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3Recording* b3LoadRecordingFromFile(const char* path)
{
	(void)sizeof(char);
	return NULL;
}

B3_API bool b3ValidateReplay(const void* data, int size, int workerCount)
{
	(void)sizeof(char);
	return false;
}

B3_API b3RecPlayer* b3CreatePlayer(const void* data, int size, int workerCount)
{
	(void)sizeof(char);
	return NULL;
}

B3_API void b3DestroyPlayer(b3RecPlayer* player)
{
	(void)sizeof(char);
}

B3_API bool b3RecPlayer_StepFrame(b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API void b3RecPlayer_SubStepFrame(b3RecPlayer* player)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_Restart(b3RecPlayer* player)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_SeekFrame(b3RecPlayer* player, int targetFrame)
{
	(void)sizeof(char);
}

B3_API b3WorldId b3RecPlayer_GetWorldId(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return (b3WorldId){0};
}

B3_API int b3RecPlayer_GetFrame(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetFrameCount(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API bool b3RecPlayer_IsAtEnd(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API bool b3RecPlayer_IsAtPreStep(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API bool b3RecPlayer_HasDiverged(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return false;
}

B3_API b3RecPlayerInfo b3RecPlayer_GetInfo(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return (b3RecPlayerInfo){0};
}

B3_API int b3RecPlayer_GetDivergeFrame(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API void b3RecPlayer_SetWorkerCount(b3RecPlayer* player, int count)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_SetKeyframePolicy(b3RecPlayer* player, size_t budgetBytes, int minIntervalFrames)
{
	(void)sizeof(char);
}

B3_API size_t b3RecPlayer_GetKeyframeBudget(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetKeyframeMinInterval(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetKeyframeInterval(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API size_t b3RecPlayer_GetKeyframeBytes(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3RecPlayer_GetBodyCount(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3BodyId b3RecPlayer_GetBodyId(const b3RecPlayer* player, int index)
{
	(void)sizeof(char);
	return (b3BodyId){0};
}

B3_API void b3RecPlayer_SetDebugShapeCallbacks(b3RecPlayer* player, b3CreateDebugShapeCallback* createDebugShape, b3DestroyDebugShapeCallback* destroyDebugShape, void* context)
{
	(void)sizeof(char);
}

B3_API void b3RecPlayer_DrawFrameQueries(b3RecPlayer* player, b3DebugDraw* draw, int queryIndex, int selectedIndex)
{
	(void)sizeof(char);
}

B3_API int b3RecPlayer_GetFrameQueryCount(const b3RecPlayer* player)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3RecQueryInfo b3RecPlayer_GetFrameQuery(const b3RecPlayer* player, int index)
{
	(void)sizeof(char);
	return (b3RecQueryInfo){0};
}

B3_API b3RecQueryHit b3RecPlayer_GetFrameQueryHit(const b3RecPlayer* player, int queryIndex, int hitIndex)
{
	(void)sizeof(char);
	return (b3RecQueryHit){0};
}

B3_API void b3Body_SetName(b3BodyId bodyId, const char* name)
{
	(void)sizeof(char);
}

B3_API const char* b3Body_GetName(b3BodyId bodyId)
{
	(void)sizeof(char);
	return NULL;
}

B3_API b3Pos b3Body_GetWorldPoint(b3BodyId bodyId, b3Vec3 localPoint)
{
	(void)sizeof(char);
	return (b3Pos){0};
}

B3_API b3Vec3 b3Body_GetLocalVector(b3BodyId bodyId, b3Vec3 worldVector)
{
	(void)sizeof(char);
	return (b3Vec3){0};
}

B3_API b3Vec3 b3Body_GetWorldVector(b3BodyId bodyId, b3Vec3 localVector)
{
	(void)sizeof(char);
	return (b3Vec3){0};
}

B3_API b3Vec3 b3Body_GetLocalPointVelocity(b3BodyId bodyId, b3Vec3 localPoint)
{
	(void)sizeof(char);
	return (b3Vec3){0};
}

B3_API b3Vec3 b3Body_GetWorldPointVelocity(b3BodyId bodyId, b3Pos worldPoint)
{
	(void)sizeof(char);
	return (b3Vec3){0};
}

B3_API void b3Body_ApplyForce(b3BodyId bodyId, b3Vec3 force, b3Pos point, bool wake)
{
	(void)sizeof(char);
}

B3_API void b3Body_ApplyForceToCenter(b3BodyId bodyId, b3Vec3 force, bool wake)
{
	(void)sizeof(char);
}

B3_API void b3Body_ApplyTorque(b3BodyId bodyId, b3Vec3 torque, bool wake)
{
	(void)sizeof(char);
}

B3_API float b3Body_GetMass(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3Matrix3 b3Body_GetLocalRotationalInertia(b3BodyId bodyId)
{
	(void)sizeof(char);
	return (b3Matrix3){0};
}

B3_API float b3Body_GetInverseMass(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API b3Matrix3 b3Body_GetWorldInverseRotationalInertia(b3BodyId bodyId)
{
	(void)sizeof(char);
	return (b3Matrix3){0};
}

B3_API b3Vec3 b3Body_GetLocalCenter(b3BodyId bodyId)
{
	(void)sizeof(char);
	return (b3Vec3){0};
}

B3_API float b3Body_GetLinearDamping(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API float b3Body_GetAngularDamping(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API float b3Body_GetGravityScale(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API bool b3Body_IsAwake(b3BodyId bodyId)
{
	(void)sizeof(char);
	return false;
}

B3_API void b3Body_EnableSleep(b3BodyId bodyId, bool enableSleep)
{
	(void)sizeof(char);
}

B3_API bool b3Body_IsSleepEnabled(b3BodyId bodyId)
{
	(void)sizeof(char);
	return false;
}

B3_API void b3Body_SetSleepThreshold(b3BodyId bodyId, float sleepThreshold)
{
	(void)sizeof(char);
}

B3_API float b3Body_GetSleepThreshold(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API bool b3Body_IsEnabled(b3BodyId bodyId)
{
	(void)sizeof(char);
	return false;
}

B3_API void b3Body_Disable(b3BodyId bodyId)
{
	(void)sizeof(char);
}

B3_API void b3Body_Enable(b3BodyId bodyId)
{
	(void)sizeof(char);
}

B3_API void b3Body_SetMotionLocks(b3BodyId bodyId, b3MotionLocks locks)
{
	(void)sizeof(char);
}

B3_API b3MotionLocks b3Body_GetMotionLocks(b3BodyId bodyId)
{
	(void)sizeof(char);
	return (b3MotionLocks){0};
}

B3_API void b3Body_EnableHitEvents(b3BodyId bodyId, bool flag)
{
	(void)sizeof(char);
}

B3_API b3WorldId b3Body_GetWorld(b3BodyId bodyId)
{
	(void)sizeof(char);
	return (b3WorldId){0};
}

B3_API int b3Body_GetShapeCount(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3Body_GetShapes(b3BodyId bodyId, b3ShapeId* shapeArray, int capacity)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3Body_GetJointCount(b3BodyId bodyId)
{
	(void)sizeof(char);
	return 0;
}

B3_API int b3Body_GetJoints(b3BodyId bodyId, b3JointId* jointArray, int capacity)
{
	(void)sizeof(char);
	return 0;
}



B3_API b3WorldId b3Shape_GetWorld(b3ShapeId shapeId)
{
	(void)sizeof(char);
	return (b3WorldId){0};
}

B3_API void b3Shape_SetName(b3ShapeId shapeId, const char* name)
{
	(void)sizeof(char);
}

B3_API const char* b3Shape_GetName(b3ShapeId shapeId)
{
	(void)sizeof(char);
	return NULL;
}

B3_API void b3Shape_SetDensity(b3ShapeId shapeId, float density, bool updateBodyMass)
{
	(void)sizeof(char);
}

B3_API void b3Shape_SetFriction(b3ShapeId shapeId, float friction)
{
	(void)sizeof(char);
}

B3_API void b3Shape_SetRestitution(b3ShapeId shapeId, float restitution)
{
	(void)sizeof(char);
}

B3_API void b3Shape_SetSphere(b3ShapeId shapeId, const b3Sphere* sphere)
{
	(void)sizeof(char);
}

B3_API void b3Shape_SetCapsule(b3ShapeId shapeId, const b3Capsule* capsule)
{
	(void)sizeof(char);
}

B3_API void b3Shape_SetHull(b3ShapeId shapeId, const b3HullData* hull)
{
	(void)sizeof(char);
}



B3_API b3MassData b3Shape_ComputeMassData(b3ShapeId shapeId)
{
	(void)sizeof(char);
	return (b3MassData){0};
}

B3_API void b3Shape_ApplyWind(b3ShapeId shapeId, b3Vec3 wind, float drag, float lift, float maxSpeed, bool wake)
{
	(void)sizeof(char);
}

B3_API b3JointType b3Joint_GetType(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3JointType){0};
}

B3_API b3BodyId b3Joint_GetBodyA(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3BodyId){0};
}

B3_API b3BodyId b3Joint_GetBodyB(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3BodyId){0};
}

B3_API b3WorldId b3Joint_GetWorld(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3WorldId){0};
}

B3_API void b3Joint_SetLocalFrameA(b3JointId jointId, b3Transform localFrame)
{
	(void)sizeof(char);
}

B3_API b3Transform b3Joint_GetLocalFrameA(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3Transform){0};
}

B3_API void b3Joint_SetLocalFrameB(b3JointId jointId, b3Transform localFrame)
{
	(void)sizeof(char);
}

B3_API b3Transform b3Joint_GetLocalFrameB(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3Transform){0};
}

B3_API void b3Joint_SetCollideConnected(b3JointId jointId, bool shouldCollide)
{
	(void)sizeof(char);
}

B3_API bool b3Joint_GetCollideConnected(b3JointId jointId)
{
	(void)sizeof(char);
	return false;
}

B3_API void b3Joint_WakeBodies(b3JointId jointId)
{
	(void)sizeof(char);
}

B3_API b3Vec3 b3Joint_GetConstraintForce(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3Vec3){0};
}

B3_API b3Vec3 b3Joint_GetConstraintTorque(b3JointId jointId)
{
	(void)sizeof(char);
	return (b3Vec3){0};
}

B3_API float b3Joint_GetLinearSeparation(b3JointId jointId)
{
	(void)sizeof(char);
	return 0;
}

B3_API float b3Joint_GetAngularSeparation(b3JointId jointId)
{
	(void)sizeof(char);
	return 0;
}
