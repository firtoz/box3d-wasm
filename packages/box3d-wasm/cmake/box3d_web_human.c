#include "box3d_web_shared.h"

#include "human.h"

#include <stddef.h>

B3W_EXPORT int b3wCreateHuman(
	int worldHandle,
	float px,
	float py,
	float pz,
	float frictionTorque,
	float hertz,
	float dampingRatio,
	int groupIndex,
	int colorize)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL) return 0;
	int freeBodySlots = 0;
	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		freeBodySlots += g_bodies[i].active ? 0 : 1;
	}
	if (freeBodySlots < bone_count) return 0;

	bool hasHumanSlot = false;
	for (int i = 0; i < B3W_MAX_HUMANS; ++i)
	{
		if (!g_humans[i].active)
		{
			hasHumanSlot = true;
			break;
		}
	}
	if (!hasHumanSlot) return 0;

	Human human = { 0 };
	CreateHuman(&human, world->worldId, (b3Pos){ px, py, pz }, frictionTorque, hertz, dampingRatio, groupIndex, NULL,
		colorize != 0);
	for (int i = 0; i < bone_count; ++i)
	{
		b3wAllocBodySlot(worldHandle, human.bones[i].bodyId);
	}

	return b3wAllocHumanSlot(worldHandle, human);
}

B3W_EXPORT int b3wGetHumanBoneBody(int humanHandle, int boneIndex)
{
	b3wHumanSlot* slot = b3wGetHuman(humanHandle);
	if (slot == NULL || boneIndex < 0 || boneIndex >= bone_count) return 0;
	b3BodyId bodyId = slot->human.bones[boneIndex].bodyId;
	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		if (g_bodies[i].active && B3_ID_EQUALS(g_bodies[i].bodyId, bodyId)) return i + 1;
	}
	return 0;
}

B3W_EXPORT int b3wGetHumanBoneCount(void)
{
	return bone_count;
}

B3W_EXPORT void b3wHumanSetVelocity(int humanHandle, float x, float y, float z)
{
	b3wHumanSlot* slot = b3wGetHuman(humanHandle);
	if (slot == NULL) return;
	Human_SetVelocity(&slot->human, (b3Vec3){ x, y, z });
}

B3W_EXPORT void b3wHumanSetBullet(int humanHandle, int flag)
{
	b3wHumanSlot* slot = b3wGetHuman(humanHandle);
	if (slot == NULL) return;
	Human_SetBullet(&slot->human, flag != 0);
}

B3W_EXPORT void b3wHumanSetJointFrictionTorque(int humanHandle, float torque)
{
	b3wHumanSlot* slot = b3wGetHuman(humanHandle);
	if (slot == NULL) return;
	Human_SetJointFrictionTorque(&slot->human, torque);
}

B3W_EXPORT void b3wHumanSetJointSpringHertz(int humanHandle, float hertz)
{
	b3wHumanSlot* slot = b3wGetHuman(humanHandle);
	if (slot == NULL) return;
	Human_SetJointSpringHertz(&slot->human, hertz);
}

B3W_EXPORT void b3wHumanSetJointDampingRatio(int humanHandle, float dampingRatio)
{
	b3wHumanSlot* slot = b3wGetHuman(humanHandle);
	if (slot == NULL) return;
	Human_SetJointDampingRatio(&slot->human, dampingRatio);
}
