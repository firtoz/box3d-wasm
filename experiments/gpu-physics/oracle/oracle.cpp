// Real Box3D C oracle: same cohort scenes as gpu-physics, wall ms/step + rest, BodyGpu frame dumps.
#include "box3d/box3d.h"
#include "../native-samples/falling-cubes.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>

static constexpr uint32_t kKindSphere = 0;
static constexpr uint32_t kKindBox = 1;
static constexpr uint32_t kKindCapsule = 2;
static constexpr uint32_t kFlagStatic = 1;
static constexpr uint32_t kFlagHidden = 2;
static constexpr uint32_t kFlagLockLinZ = 1u << 10;
static constexpr uint32_t kFlagLockAngX = 1u << 11;
static constexpr uint32_t kFlagLockAngY = 1u << 12;
static constexpr uint32_t kFlagLockAngZ = 1u << 13;
static constexpr int kSubSteps = 4;
static constexpr float kDt = 1.0f / 60.0f;
static constexpr uint32_t kCheckpoints[] = {0, 50, 100, 200, 300};
static constexpr float kSphereRadius = 0.18f;

struct BodyGpu
{
	float pos[3];
	float inv_mass;
	float vel[3];
	uint32_t kind;
	float half[3];
	uint32_t flags;
	float rot[4];
	float omega[3];
	float restitution;
	float inv_inertia[3];
	float friction;
	float gravity_scale;
	float linear_damping;
	float angular_damping;
	float rolling;
	float dp[3];
	float pad_dp;
	float dq[4];
	uint32_t island_id;
	float sleep_velocity;
	uint32_t pad_island[2];
};
static_assert(sizeof(BodyGpu) == 160, "BodyGpu must match gpu-physics 160-byte layout");

struct TracePoint
{
	float r_a[4];
	float r_b[4];
};

struct TraceManifold
{
	uint32_t a;
	uint32_t b;
	uint32_t count;
	uint32_t pad;
	float n[3];
	float pn;
	TracePoint pts[4];
	uint32_t point_ids[4];
	float friction[3];
	float twist;
	float rolling[3];
	float pad_impulse;
};
static_assert(sizeof(TraceManifold) == 208, "TraceManifold must match gpu-physics 208-byte layout");

static uint32_t flip_feature_id(uint32_t id)
{
	uint32_t owner1 = (id >> 24) & 1u;
	uint32_t index1 = (id >> 16) & 255u;
	uint32_t owner2 = (id >> 8) & 1u;
	uint32_t index2 = id & 255u;
	return ((1u - owner2) << 24) | (index2 << 16) | ((1u - owner1) << 8) | index1;
}

struct Vis
{
	b3BodyId id;
	uint32_t kind;
	float half[3];
	uint32_t flags;
};

struct SceneState
{
	b3WorldId world{};
	std::vector<Vis> bodies;
};

static const char* kScenes[] = {
	"single-box",
	"box-stack",
	"sphere-stack",
	"capsule-stack",
	"revolute",
	"weld",
	"anchored-mechanisms",
	"joint-chain",
	"stack",
	"pyramid",
	"bounce",
	"mixed",
	"spinner",
	"ramp",
	"spheres",
	"dominoes",
	"high-resistance",
	"mixed-stacks",
	"falling-cubes",
};

static bool is_scene(const char* name)
{
	for (const char* s : kScenes)
	{
		if (std::strcmp(name, s) == 0)
		{
			return true;
		}
	}
	return false;
}

static Vis add_ground(b3WorldId world, float extent)
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.position = b3Pos{0.0f, -1.0f, 0.0f};
	b3BodyId ground = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3BoxHull hull = b3MakeBoxHull(extent, 1.0f, extent);
	b3CreateHullShape(ground, &shapeDef, &hull.base);
	return Vis{ground, kKindBox, {extent, 1.0f, extent}, kFlagStatic | kFlagHidden};
}

static bool g_enable_sleep = true;

static SceneState build_scene(const char* name, uint32_t sphere_count, int workers = 0)
{
	SceneState st;
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.enableSleep = g_enable_sleep;
	worldDef.workerCount = workers;
	st.world = b3CreateWorld(&worldDef);
	if (!b3World_IsValid(st.world))
	{
		std::fprintf(stderr, "b3CreateWorld failed\n");
		std::exit(1);
	}

	auto push_box = [&](b3BodyId id, float hx, float hy, float hz, uint32_t flags) {
		st.bodies.push_back(Vis{id, kKindBox, {hx, hy, hz}, flags});
	};
	auto push_sphere = [&](b3BodyId id, float r, uint32_t flags) {
		st.bodies.push_back(Vis{id, kKindSphere, {r, r, r}, flags});
	};
	auto push_capsule = [&](b3BodyId id, float r, float half_len, uint32_t flags) {
		st.bodies.push_back(Vis{id, kKindCapsule, {r, half_len, r}, flags});
	};

	if (std::strcmp(name, "single-box") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 20.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 0.5f, 0.0f};
		b3BodyId id = b3CreateBody(st.world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull cube = b3MakeCubeHull(0.5f);
		b3CreateHullShape(id, &shapeDef, &cube.base);
		push_box(id, 0.5f, 0.5f, 0.5f, 0);
	}
	else if (std::strcmp(name, "box-stack") == 0 || std::strcmp(name, "stack") == 0)
	{
		st.bodies.push_back(add_ground(st.world, std::strcmp(name, "stack") == 0 ? 12.0f : 40.0f));
		float a = 0.5f;
		int count = std::strcmp(name, "stack") == 0 ? 6 : 40;
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3BoxHull cube = b3MakeBoxHull(a, a, a);
		for (int i = 0; i < count; ++i)
		{
			if (std::strcmp(name, "stack") == 0)
			{
				bodyDef.position = b3Pos{0.0f, 0.5f + (float)i * 1.05f, 0.0f};
			}
			else
			{
				bodyDef.position = b3Pos{0.0f, 1.5f * a + 2.5f * a * (float)i, 0.0f};
			}
			b3BodyId id = b3CreateBody(st.world, &bodyDef);
			b3ShapeDef shapeDef = b3DefaultShapeDef();
			if (std::strcmp(name, "box-stack") == 0)
			{
				shapeDef.baseMaterial.rollingResistance = 0.1f;
			}
			b3CreateHullShape(id, &shapeDef, &cube.base);
			push_box(id, a, a, a, 0);
		}
	}
	else if (std::strcmp(name, "sphere-stack") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 15.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		float r = 0.5f;
		b3Sphere sphere = {b3Vec3_zero, r};
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.rollingResistance = 0.1f;
		float y = 1.5f * r;
		for (int i = 0; i < 30; ++i)
		{
			bodyDef.position.y = y;
			b3BodyId id = b3CreateBody(st.world, &bodyDef);
			b3CreateSphereShape(id, &shapeDef, &sphere);
			push_sphere(id, r, 0);
			y += 3.0f * r;
		}
	}
	else if (std::strcmp(name, "capsule-stack") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 40.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.motionLocks.linearZ = true;
		bodyDef.motionLocks.angularX = true;
		bodyDef.motionLocks.angularY = true;
		bodyDef.motionLocks.angularZ = true;
		float r = 0.5f;
		b3Capsule capsule = {{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, r};
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		uint32_t flags = kFlagLockLinZ | kFlagLockAngX | kFlagLockAngY | kFlagLockAngZ;
		float y = 1.5f * r;
		for (int i = 0; i < 20; ++i)
		{
			bodyDef.position.y = y;
			b3BodyId id = b3CreateBody(st.world, &bodyDef);
			b3CreateCapsuleShape(id, &shapeDef, &capsule);
			push_capsule(id, r, 1.0f, flags);
			y += 2.0f * r;
		}
	}
	else if (std::strcmp(name, "high-resistance") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 50.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.rotation = b3MakeQuatFromAxisAngle(b3Vec3_axisZ, B3_DEG_TO_RAD * 30.0f);
		b3Capsule capsule = {{0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.5f};
		for (int index = 0; index < 10; ++index)
		{
			bodyDef.position = {-22.0f + 5.0f * (float)index, 1.5f, 0.0f};
			b3BodyId id = b3CreateBody(st.world, &bodyDef);
			b3ShapeDef shapeDef = b3DefaultShapeDef();
			shapeDef.baseMaterial.rollingResistance = 0.2f * (float)index;
			b3CreateCapsuleShape(id, &shapeDef, &capsule);
			push_capsule(id, 0.5f, 1.0f, 0);
		}
	}
	else if (std::strcmp(name, "falling-cubes") == 0)
    {
        uint32_t count = std::max(1u, sphere_count);
        st.bodies.push_back(add_ground(st.world, fallingGround(count)));
        b3BodyDef body = b3DefaultBodyDef(); body.type = b3_dynamicBody;
        b3ShapeDef shape = b3DefaultShapeDef();
        b3BoxHull cube = b3MakeCubeHull(0.5f);
        for (uint32_t i = 0; i < count; ++i) {
            body.position = fallingPosition(count, i);
            b3BodyId id = b3CreateBody(st.world, &body);
            b3CreateHullShape(id, &shape, &cube.base);
            push_box(id, 0.5f, 0.5f, 0.5f, 0);
        }
    }
	else if (std::strcmp(name, "mixed-stacks") == 0)
	{
		int count = (int)std::max(2u, sphere_count);
		int per_layer = (count + 1) / 2;
		// Match the GPU fixture: every scaled row must remain over both grounds.
		float ground_half = std::max(100.0f, 3.0f * (float)((per_layer - 1) / 20) + 3.0f);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		for (int g = 0; g < 2; ++g)
		{
			st.bodies.push_back(add_ground(st.world, ground_half));
		}
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3BoxHull cube = b3MakeCubeHull(0.5f);
		for (int i = 0; i < count; ++i)
		{
			int col = (i % per_layer) % 20;
			int layer = i >= per_layer ? 1 : 0;
			int row = (i % per_layer) / 20;
			bodyDef.position = {3.0f * (float)col, 0.5f + (float)layer, 3.0f * (float)row};
			b3BodyId id = b3CreateBody(st.world, &bodyDef);
			b3CreateHullShape(id, &shapeDef, &cube.base);
			push_box(id, 0.5f, 0.5f, 0.5f, 0);
		}
	}
	else if (std::strcmp(name, "revolute") == 0 || std::strcmp(name, "weld") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 20.0f));
		b3BodyDef groundDef = b3DefaultBodyDef();
		groundDef.position = b3Pos{0.0f, -1.0f, 0.0f};
		b3BodyId groundId = b3CreateBody(st.world, &groundDef);
		st.bodies.push_back(Vis{groundId, kKindSphere, {0.0f, 0.0f, 0.0f}, kFlagStatic});

		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 4.0f, 0.0f};
		if (std::strcmp(name, "weld") == 0)
		{
			bodyDef.gravityScale = 0.0f;
		}
		b3BodyId body = b3CreateBody(st.world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeBoxHull(0.5f, 1.5f, 0.25f);
		b3CreateHullShape(body, &shapeDef, &box.base);
		push_box(body, 0.5f, 1.5f, 0.25f, 0);

		if (std::strcmp(name, "revolute") == 0)
		{
			b3RevoluteJointDef jointDef = b3DefaultRevoluteJointDef();
			jointDef.base.bodyIdA = groundId;
			jointDef.base.bodyIdB = body;
			jointDef.base.localFrameA.p = {0.0f, 6.5f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, 1.5f, 0.0f};
			b3CreateRevoluteJoint(st.world, &jointDef);
		}
		else
		{
			b3WeldJointDef jointDef = b3DefaultWeldJointDef();
			jointDef.base.bodyIdA = groundId;
			jointDef.base.bodyIdB = body;
			jointDef.base.localFrameA.p = {0.0f, 6.5f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, 1.5f, 0.0f};
			jointDef.base.constraintHertz = 240.0f;
			b3CreateWeldJoint(st.world, &jointDef);
		}
	}
	else if (std::strcmp(name, "anchored-mechanisms") == 0)
	{
		Vis groundVis = add_ground(st.world, 80.0f);
		st.bodies.push_back(groundVis);
		uint32_t count = std::max(1u, sphere_count);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull hull = b3MakeBoxHull(0.25f, 1.0f, 0.25f);
		for (uint32_t i = 0; i < count; ++i)
		{
			float x = ((float)i - 0.5f * ((float)count - 1.0f)) * 1.2f;
			b3BodyDef bodyDef = b3DefaultBodyDef();
			bodyDef.type = b3_dynamicBody;
			bodyDef.position = {x, 4.0f, 0.0f};
			b3BodyId body = b3CreateBody(st.world, &bodyDef);
			b3CreateHullShape(body, &shapeDef, &hull.base);
			push_box(body, 0.25f, 1.0f, 0.25f, 0);
			b3RevoluteJointDef jointDef = b3DefaultRevoluteJointDef();
			jointDef.base.bodyIdA = groundVis.id;
			jointDef.base.bodyIdB = body;
			jointDef.base.localFrameA.p = {x, 6.0f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, 1.0f, 0.0f};
			b3CreateRevoluteJoint(st.world, &jointDef);
		}
	}
	else if (std::strcmp(name, "joint-chain") == 0)
	{
		Vis groundVis = add_ground(st.world, 40.0f);
		st.bodies.push_back(groundVis);
		uint32_t count = std::max(2u, sphere_count);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull hull = b3MakeBoxHull(0.25f, 0.4f, 0.25f);
		b3BodyId prev = groundVis.id;
		for (uint32_t i = 0; i < count; ++i)
		{
			b3BodyDef bodyDef = b3DefaultBodyDef();
			bodyDef.type = b3_dynamicBody;
			bodyDef.position = {0.0f, 0.5f + (float)i * 0.9f, 0.0f};
			b3BodyId body = b3CreateBody(st.world, &bodyDef);
			b3CreateHullShape(body, &shapeDef, &hull.base);
			push_box(body, 0.25f, 0.4f, 0.25f, 0);
			b3RevoluteJointDef jointDef = b3DefaultRevoluteJointDef();
			jointDef.base.bodyIdA = prev;
			jointDef.base.bodyIdB = body;
			jointDef.base.localFrameA.p = {0.0f, 0.4f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, -0.4f, 0.0f};
			b3CreateRevoluteJoint(st.world, &jointDef);
			prev = body;
		}
	}
	else if (std::strcmp(name, "pyramid") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 20.0f));
		float a = 0.5f;
		int rows = 10;
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3BoxHull cube = b3MakeCubeHull(a);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.rollingResistance = 0.1f;
		for (int i = 0; i < rows; ++i)
		{
			int count = rows - i;
			for (int j = 0; j < count; ++j)
			{
				float x = (2.0f * (float)j - ((float)count - 1.0f)) * a;
				float y = a + 2.0f * a * (float)i;
				bodyDef.position = b3Pos{x, y, 0.0f};
				b3BodyId id = b3CreateBody(st.world, &bodyDef);
				b3CreateHullShape(id, &shapeDef, &cube.base);
				push_box(id, a, a, a, 0);
			}
		}
	}
	else if (std::strcmp(name, "bounce") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 20.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 5.0f, 0.0f};
		b3BodyId id = b3CreateBody(st.world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.restitution = 1.0f;
		b3Sphere sphere = {b3Vec3_zero, 0.5f};
		b3CreateSphereShape(id, &shapeDef, &sphere);
		push_sphere(id, 0.5f, 0);
	}
	else if (std::strcmp(name, "mixed") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 20.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		bodyDef.position = b3Pos{-2.0f, 3.0f, 0.0f};
		b3BodyId boxId = b3CreateBody(st.world, &bodyDef);
		b3BoxHull cube = b3MakeCubeHull(0.5f);
		b3CreateHullShape(boxId, &shapeDef, &cube.base);
		push_box(boxId, 0.5f, 0.5f, 0.5f, 0);
		bodyDef.position = b3Pos{0.0f, 3.0f, 0.0f};
		b3BodyId sphId = b3CreateBody(st.world, &bodyDef);
		b3Sphere sphere = {b3Vec3_zero, 0.5f};
		b3CreateSphereShape(sphId, &shapeDef, &sphere);
		push_sphere(sphId, 0.5f, 0);
		bodyDef.position = b3Pos{2.0f, 3.0f, 0.0f};
		b3BodyId capId = b3CreateBody(st.world, &bodyDef);
		b3Capsule capsule = {{-0.5f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, 0.35f};
		b3CreateCapsuleShape(capId, &shapeDef, &capsule);
		push_capsule(capId, 0.35f, 0.5f, 0);
	}
	else if (std::strcmp(name, "spinner") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 20.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 4.0f, 0.0f};
		bodyDef.gravityScale = 0.0f;
		bodyDef.angularVelocity = {0.0f, 0.0f, 8.0f};
		b3BodyId id = b3CreateBody(st.world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeBoxHull(1.0f, 0.15f, 0.6f);
		b3CreateHullShape(id, &shapeDef, &box.base);
		push_box(id, 1.0f, 0.15f, 0.6f, 0);
	}
	else if (std::strcmp(name, "ramp") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 20.0f));
		b3BodyDef rampDef = b3DefaultBodyDef();
		rampDef.position = b3Pos{0.0f, 1.5f, 0.0f};
		rampDef.rotation = b3MakeQuatFromAxisAngle({0.0f, 0.0f, 1.0f}, -20.0f * B3_DEG_TO_RAD);
		b3BodyId ramp = b3CreateBody(st.world, &rampDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull plank = b3MakeBoxHull(6.0f, 0.2f, 1.5f);
		b3CreateHullShape(ramp, &shapeDef, &plank.base);
		push_box(ramp, 6.0f, 0.2f, 1.5f, kFlagStatic);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{-4.0f, 4.2f, 0.0f};
		b3BodyId cubeId = b3CreateBody(st.world, &bodyDef);
		b3BoxHull cube = b3MakeCubeHull(0.4f);
		b3CreateHullShape(cubeId, &shapeDef, &cube.base);
		push_box(cubeId, 0.4f, 0.4f, 0.4f, 0);
	}
	else if (std::strcmp(name, "spheres") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 12.0f));
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3Sphere sphere = {b3Vec3_zero, kSphereRadius};
		uint32_t count = sphere_count ? sphere_count : 256;
		uint32_t cols = (uint32_t)std::ceil(std::cbrt((float)count));
		if (cols < 1)
		{
			cols = 1;
		}
		for (uint32_t i = 0; i < count; ++i)
		{
			uint32_t x = i % cols;
			uint32_t yz = i / cols;
			uint32_t z = yz % cols;
			uint32_t y = yz / cols;
			float px = (x - 0.5f * (cols - 1.0f)) * (kSphereRadius * 2.4f);
			float py = 1.2f + y * (kSphereRadius * 2.4f);
			float pz = (z - 0.5f * (cols - 1.0f)) * (kSphereRadius * 2.4f);
			bodyDef.position = b3Pos{px, py, pz};
			b3BodyId id = b3CreateBody(st.world, &bodyDef);
			b3CreateSphereShape(id, &shapeDef, &sphere);
			push_sphere(id, kSphereRadius, 0);
		}
	}
	else if (std::strcmp(name, "dominoes") == 0)
	{
		st.bodies.push_back(add_ground(st.world, 80.0f));
		uint32_t rings = sphere_count ? sphere_count : 30u;
		b3BoxHull boxHull = b3MakeBoxHull(0.2f, 0.8f, 0.05f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		for (uint32_t ring = 0; ring < rings; ++ring)
		{
			float radius = 7.0f + 1.1f * (float)ring;
			for (float alpha = 0.0f; alpha <= 360.0f; alpha += 2.0f)
			{
				float rad = B3_PI / 180.0f * alpha;
				b3CosSin cs = b3ComputeCosSin(rad);
				float cosine = cs.cosine;
				float sine = cs.sine;
				b3Pos position = {radius * cosine, 0.8f, radius * sine};
				position.x -= alpha / 630.0f * cosine;
				position.z -= alpha / 630.0f * sine;
				bodyDef.position = position;
				bodyDef.rotation = b3MakeQuatFromAxisAngle({0.0f, 1.0f, 0.0f}, -rad);
				b3BodyId body = b3CreateBody(st.world, &bodyDef);
				b3CreateHullShape(body, &shapeDef, &boxHull.base);
				push_box(body, 0.2f, 0.8f, 0.05f, 0);
				if (alpha == 0.0f)
				{
					b3Body_ApplyLinearImpulse(
						body, {0.0f, 0.0f, 25.0f}, {position.x, position.y + 0.8f, position.z}, true);
				}
			}
		}
	}
	else
	{
		std::fprintf(stderr, "unknown scene %s\n", name);
		std::exit(2);
	}
	return st;
}

static void destroy_scene(SceneState& st)
{
	if (b3World_IsValid(st.world))
	{
		b3DestroyWorld(st.world);
	}
	st.world = {};
	st.bodies.clear();
}

static void fill_gpu(BodyGpu* g, const Vis& v)
{
	std::memset(g, 0, sizeof(*g));
	b3Pos p = b3Body_GetPosition(v.id);
	g->pos[0] = (float)p.x;
	g->pos[1] = (float)p.y;
	g->pos[2] = (float)p.z;
	float mass = b3Body_GetMass(v.id);
	g->inv_mass = mass > 0.0f ? 1.0f / mass : 0.0f;
	b3Vec3 vel = b3Body_GetLinearVelocity(v.id);
	g->vel[0] = vel.x;
	g->vel[1] = vel.y;
	g->vel[2] = vel.z;
	g->kind = v.kind;
	g->half[0] = v.half[0];
	g->half[1] = v.half[1];
	g->half[2] = v.half[2];
	g->flags = v.flags;
	b3Quat q = b3Body_GetRotation(v.id);
	g->rot[0] = q.v.x;
	g->rot[1] = q.v.y;
	g->rot[2] = q.v.z;
	g->rot[3] = q.s;
	b3Vec3 w = b3Body_GetAngularVelocity(v.id);
	g->omega[0] = w.x;
	g->omega[1] = w.y;
	g->omega[2] = w.z;
	g->dq[3] = 1.0f;
	g->island_id = UINT32_MAX;
}

static int body_index(const SceneState& st, b3BodyId id)
{
	for (size_t i = 0; i < st.bodies.size(); ++i)
	{
		if (st.bodies[i].id.index1 == id.index1 && st.bodies[i].id.world0 == id.world0 &&
			st.bodies[i].id.generation == id.generation)
		{
			return (int)i;
		}
	}
	return -1;
}

static void collect_contacts(const SceneState& st, std::vector<TraceManifold>& out)
{
	out.clear();
	std::unordered_set<uint64_t> seen;
	std::vector<b3ContactData> buf;
	for (size_t bi = 0; bi < st.bodies.size(); ++bi)
	{
		int cap = b3Body_GetContactCapacity(st.bodies[bi].id);
		if (cap <= 0)
		{
			continue;
		}
		buf.resize((size_t)cap);
		int got = b3Body_GetContactData(st.bodies[bi].id, buf.data(), cap);
		for (int k = 0; k < got; ++k)
		{
			const b3ContactData& cd = buf[(size_t)k];
			if (!b3Contact_IsValid(cd.contactId) || cd.manifolds == nullptr || cd.manifoldCount <= 0)
			{
				continue;
			}
			uint64_t key = ((uint64_t)(uint32_t)cd.contactId.index1 << 32) | cd.contactId.generation;
			if (!seen.insert(key).second)
			{
				continue;
			}
			b3BodyId ba = b3Shape_GetBody(cd.shapeIdA);
			b3BodyId bb = b3Shape_GetBody(cd.shapeIdB);
			int ia = body_index(st, ba);
			int ib = body_index(st, bb);
			if (ia < 0 || ib < 0 || ia == ib)
			{
				continue;
			}
			const b3Manifold& man = cd.manifolds[0];
			if (man.pointCount <= 0)
			{
				continue;
			}
			TraceManifold tm{};
			int a = ia;
			int b = ib;
			float nx = man.normal.x;
			float ny = man.normal.y;
			float nz = man.normal.z;
			bool flip = a > b;
			if (flip)
			{
				int t = a;
				a = b;
				b = t;
				nx = -nx;
				ny = -ny;
				nz = -nz;
			}
			tm.a = (uint32_t)a;
			tm.b = (uint32_t)b;
			tm.count = (uint32_t)std::min(man.pointCount, 4);
			tm.n[0] = nx;
			tm.n[1] = ny;
			tm.n[2] = nz;
			float impulseSign = flip ? -1.0f : 1.0f;
			tm.friction[0] = impulseSign * man.frictionImpulse.x;
			tm.friction[1] = impulseSign * man.frictionImpulse.y;
			tm.friction[2] = impulseSign * man.frictionImpulse.z;
			tm.twist = man.twistImpulse;
			tm.rolling[0] = impulseSign * man.rollingImpulse.x;
			tm.rolling[1] = impulseSign * man.rollingImpulse.y;
			tm.rolling[2] = impulseSign * man.rollingImpulse.z;
			for (uint32_t p = 0; p < tm.count; ++p)
			{
				const b3ManifoldPoint& mp = man.points[p];
				tm.point_ids[p] = flip ? flip_feature_id(mp.featureId) : mp.featureId;
				if (flip)
				{
					tm.pts[p].r_a[0] = mp.anchorB.x;
					tm.pts[p].r_a[1] = mp.anchorB.y;
					tm.pts[p].r_a[2] = mp.anchorB.z;
					tm.pts[p].r_b[0] = mp.anchorA.x;
					tm.pts[p].r_b[1] = mp.anchorA.y;
					tm.pts[p].r_b[2] = mp.anchorA.z;
				}
				else
				{
					tm.pts[p].r_a[0] = mp.anchorA.x;
					tm.pts[p].r_a[1] = mp.anchorA.y;
					tm.pts[p].r_a[2] = mp.anchorA.z;
					tm.pts[p].r_b[0] = mp.anchorB.x;
					tm.pts[p].r_b[1] = mp.anchorB.y;
					tm.pts[p].r_b[2] = mp.anchorB.z;
				}
				tm.pts[p].r_a[3] = mp.separation;
				tm.pts[p].r_b[3] = mp.normalImpulse;
			}
			out.push_back(tm);
		}
	}
	std::sort(out.begin(), out.end(), [](const TraceManifold& x, const TraceManifold& y) {
		if (x.a != y.a)
		{
			return x.a < y.a;
		}
		return x.b < y.b;
	});
}

static FILE* open_trace(const char* path, uint32_t frames, uint32_t body_count)
{
	FILE* f = std::fopen(path, "wb");
	if (!f)
	{
		std::fprintf(stderr, "cannot write %s\n", path);
		std::exit(1);
	}
	const char mag[4] = {'B', '3', 'T', 'R'};
	uint32_t version = 4;
	std::fwrite(mag, 1, 4, f);
	std::fwrite(&version, 4, 1, f);
	std::fwrite(&frames, 4, 1, f);
	std::fwrite(&body_count, 4, 1, f);
	return f;
}

static void write_trace_frame(FILE* f, SceneState& st, std::vector<BodyGpu>& buf, uint32_t step)
{
	for (size_t b = 0; b < st.bodies.size(); ++b)
	{
		fill_gpu(&buf[b], st.bodies[b]);
	}
	std::fwrite(buf.data(), sizeof(BodyGpu), buf.size(), f);
	std::vector<TraceManifold> cons;
	collect_contacts(st, cons);
	uint32_t nc = (uint32_t)cons.size();
	std::fwrite(&nc, 4, 1, f);
	if (nc)
	{
		std::fwrite(cons.data(), sizeof(TraceManifold), nc, f);
	}
	if (st.bodies.size() >= 3 && std::getenv("TRACE_VERBOSE") != nullptr)
	{
		b3Counters ctr = b3World_GetCounters(st.world);
		b3Pos p1 = b3Body_GetPosition(st.bodies[1].id);
		b3Pos p2 = b3Body_GetPosition(st.bodies[2].id);
		std::fprintf(stderr,
			"  cpu[%u] worldContacts=%d touching=%u y1=%.5f y2=%.5f gap=%.5f\n", step, ctr.contactCount, nc,
			(float)p1.y, (float)p2.y, (float)p2.y - (float)p1.y - 1.0f);
	}
}

static bool is_stack(const char* name)
{
	return std::strcmp(name, "box-stack") == 0 || std::strcmp(name, "sphere-stack") == 0 ||
		   std::strcmp(name, "capsule-stack") == 0 || std::strcmp(name, "stack") == 0;
}

struct RestSnap
{
	uint32_t step;
	float speed;
	bool has_y;
	float y_err;
	bool has_stack;
	float bottom_err;
	float gap_err;
	bool has_floor;
	float floor_err;
	bool has_joint;
	float joint_err;
	bool has_hold;
	float hold_err;
	bool has_bounce;
	float bounce_y;
	bool has_omega;
	float omega;
	bool has_pyramid;
	uint32_t fallen;
	float xz_err;
	bool has_mixed;
	float min_y;
	bool has_ramp;
	float slide_x;
	float slide_y;
};

static RestSnap measure_rest(const SceneState& st, const char* name, uint32_t step)
{
	RestSnap r{};
	r.step = step;
	std::vector<float> ys;
	ys.reserve(st.bodies.size());
	bool first_dyn = true;
	for (const Vis& v : st.bodies)
	{
		float mass = b3Body_GetMass(v.id);
		if (mass <= 0.0f)
		{
			continue;
		}
		b3Pos p = b3Body_GetPosition(v.id);
		b3Vec3 vel = b3Body_GetLinearVelocity(v.id);
		b3Vec3 w = b3Body_GetAngularVelocity(v.id);
		float spd = std::sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
		r.speed = std::max(r.speed, spd);
		ys.push_back((float)p.y);
		if (std::strcmp(name, "revolute") == 0)
		{
			float dx = (float)p.x;
			float dy = (float)p.y - 5.5f;
			float dz = (float)p.z;
			r.has_joint = true;
			r.joint_err = std::fabs(std::sqrt(dx * dx + dy * dy + dz * dz) - 1.5f);
		}
		if (std::strcmp(name, "weld") == 0)
		{
			float dx = (float)p.x;
			float dy = (float)p.y - 4.0f;
			float dz = (float)p.z;
			r.has_hold = true;
			r.hold_err = std::sqrt(dx * dx + dy * dy + dz * dz);
		}
		if (std::strcmp(name, "bounce") == 0 && first_dyn)
		{
			r.has_bounce = true;
			r.bounce_y = (float)p.y;
		}
		if (std::strcmp(name, "spinner") == 0)
		{
			float om = std::sqrt(w.x * w.x + w.y * w.y + w.z * w.z);
			r.has_omega = true;
			r.omega = std::max(r.omega, om);
		}
		if (std::strcmp(name, "pyramid") == 0)
		{
			r.has_pyramid = true;
			if ((float)p.y < 0.25f)
			{
				r.fallen += 1;
			}
			float xz = std::sqrt((float)p.x * (float)p.x + (float)p.z * (float)p.z);
			r.xz_err = std::max(r.xz_err, xz);
		}
		if (std::strcmp(name, "mixed") == 0)
		{
			r.has_mixed = true;
			r.min_y = first_dyn ? (float)p.y : std::min(r.min_y, (float)p.y);
		}
		if (std::strcmp(name, "ramp") == 0 && first_dyn)
		{
			r.has_ramp = true;
			r.slide_x = (float)p.x;
			r.slide_y = (float)p.y;
		}
		first_dyn = false;
	}
	if (std::strcmp(name, "single-box") == 0 && !ys.empty())
	{
		r.has_y = true;
		r.y_err = std::fabs(ys[0] - 0.5f);
	}
	if (is_stack(name) && ys.size() >= 2)
	{
		std::sort(ys.begin(), ys.end());
		r.has_stack = true;
		r.bottom_err = std::fabs(ys[0] - 0.5f);
		r.gap_err = 0.0f;
		for (size_t i = 0; i + 1 < ys.size(); ++i)
		{
			r.gap_err = std::max(r.gap_err, std::fabs(ys[i + 1] - ys[i] - 1.0f));
		}
	}
	if (std::strcmp(name, "spheres") == 0 && !ys.empty())
	{
		float min_y = ys[0];
		for (float y : ys)
		{
			min_y = std::min(min_y, y);
		}
		r.has_floor = true;
		r.floor_err = std::fabs(min_y - kSphereRadius);
	}
	return r;
}

static FILE* open_dump(const char* path, uint32_t frames, uint32_t body_count)
{
	FILE* f = std::fopen(path, "wb");
	if (!f)
	{
		std::fprintf(stderr, "cannot write %s\n", path);
		std::exit(1);
	}
	const char mag[4] = {'B', '3', 'O', 'R'};
	uint32_t version = 1;
	uint32_t stride = (uint32_t)sizeof(BodyGpu);
	std::fwrite(mag, 1, 4, f);
	std::fwrite(&version, 4, 1, f);
	std::fwrite(&frames, 4, 1, f);
	std::fwrite(&body_count, 4, 1, f);
	std::fwrite(&stride, 4, 1, f);
	return f;
}

static void write_dump_frame(FILE* f, SceneState& st, std::vector<BodyGpu>& buf)
{
	for (size_t b = 0; b < st.bodies.size(); ++b)
	{
		fill_gpu(&buf[b], st.bodies[b]);
	}
	std::fwrite(buf.data(), sizeof(BodyGpu), buf.size(), f);
}

#ifndef GPU_PHYSICS_ORACLE_LIBRARY
int main(int argc, char** argv)
{
	uint32_t frames = 300;
	uint32_t warmup = 60;
	uint32_t bodies_n = 256;
	bool bodies_set = false;
	int workers = 0;
	const char* dump_dir = nullptr;
	const char* trace_dir = nullptr;
	const char* metrics_path = nullptr;
	std::vector<const char*> scenes;
	for (int i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc)
		{
			frames = (uint32_t)std::atoi(argv[++i]);
		}
		else if (std::strcmp(argv[i], "--warmup") == 0 && i + 1 < argc)
		{
			warmup = (uint32_t)std::atoi(argv[++i]);
		}
		else if (std::strcmp(argv[i], "--bodies") == 0 && i + 1 < argc)
		{
			bodies_n = (uint32_t)std::atoi(argv[++i]);
			bodies_set = true;
		}
		else if (std::strcmp(argv[i], "--workers") == 0 && i + 1 < argc)
		{
			char* end = nullptr;
			long value = std::strtol(argv[++i], &end, 10);
			if (end == argv[i] || *end || value < 0 || value > 64)
			{
				std::fprintf(stderr, "workers must be an integer from 0 to 64\n");
				return 2;
			}
			workers = (int)value;
		}
		else if (std::strcmp(argv[i], "--dump-dir") == 0 && i + 1 < argc)
		{
			dump_dir = argv[++i];
		}
		else if (std::strcmp(argv[i], "--trace-dir") == 0 && i + 1 < argc)
		{
			trace_dir = argv[++i];
		}
		else if (std::strcmp(argv[i], "--no-sleep") == 0)
		{
			g_enable_sleep = false;
		}
		else if (std::strcmp(argv[i], "--metrics") == 0 && i + 1 < argc)
		{
			metrics_path = argv[++i];
		}
		else if (std::strcmp(argv[i], "--scene") == 0 && i + 1 < argc)
		{
			const char* n = argv[++i];
			if (!is_scene(n))
			{
				std::fprintf(stderr, "unknown scene %s\n", n);
				return 2;
			}
			scenes.push_back(n);
		}
		else
		{
			std::fprintf(stderr, "unknown arg %s\n", argv[i]);
			return 2;
		}
	}
	if (scenes.empty())
	{
		for (const char* s : kScenes)
		{
			scenes.push_back(s);
		}
	}
	if (frames < 1)
	{
		frames = 1;
	}

	std::string json;
	json += "{\n  \"adapter\": \"Box3D CPU\",\n  \"engine\": \"box3d-cpu\",\n  \"sub_steps\": 4,\n";
	json += "  \"warmup_steps\": ";
	json += std::to_string(warmup);
	json += ",\n  \"timed_steps\": ";
	json += std::to_string(frames);
	json += ",\n  \"scenes\": {\n";

	for (size_t si = 0; si < scenes.size(); ++si)
	{
		const char* name = scenes[si];
		uint32_t scene_bodies = bodies_n;
		if (!bodies_set)
		{
			if (std::strcmp(name, "mixed-stacks") == 0)
			{
				scene_bodies = 600;
			}
			else if (std::strcmp(name, "dominoes") == 0)
			{
				scene_bodies = 30;
			}
		}
		std::fprintf(stderr, "oracle %s\n", name);

		float wall_ms = 0.0f;
		float profile_sum = 0.0f;
		uint32_t nbody = 0;
		std::vector<float> profile_samples;
		std::vector<float> wall_samples;
		if (dump_dir || metrics_path)
		{
			SceneState timed = build_scene(name, scene_bodies, workers);
			nbody = (uint32_t)timed.bodies.size();
			for (uint32_t i = 0; i < warmup; ++i)
			{
				b3World_Step(timed.world, kDt, kSubSteps);
			}
			auto t0 = std::chrono::steady_clock::now();
			for (uint32_t i = 0; i < frames; ++i)
			{
				auto s0 = std::chrono::steady_clock::now();
				b3World_Step(timed.world, kDt, kSubSteps);
				auto s1 = std::chrono::steady_clock::now();
				wall_samples.push_back(std::chrono::duration<float, std::milli>(s1 - s0).count());
				float p = b3World_GetProfile(timed.world).step;
				profile_sum += p;
				profile_samples.push_back(p);
			}
			auto t1 = std::chrono::steady_clock::now();
			wall_ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
            if (std::strcmp(name, "falling-cubes") == 0 && warmup + frames >= 180) {
                if (b3World_GetCounters(timed.world).contactCount == 0) std::abort();
                for (const Vis& vis : timed.bodies) {
                    b3Pos pos = b3Body_GetPosition(vis.id);
                    if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z) || pos.y < -1.01) std::abort();
                }
            }
			destroy_scene(timed);
		}

		SceneState restw = build_scene(name, scene_bodies, workers);
		if (nbody == 0)
		{
			nbody = (uint32_t)restw.bodies.size();
		}
		std::vector<RestSnap> rest;
		FILE* dump = nullptr;
		FILE* trace = nullptr;
		std::vector<BodyGpu> dump_buf;
		if (dump_dir)
		{
			char path[1024];
			std::snprintf(path, sizeof(path), "%s/%s.bin", dump_dir, name);
			dump = open_dump(path, frames, (uint32_t)restw.bodies.size());
			dump_buf.resize(restw.bodies.size());
			std::fprintf(stderr, "  dump %s\n", path);
		}
		if (trace_dir)
		{
			char path[1024];
			std::snprintf(path, sizeof(path), "%s/%s.b3tr", trace_dir, name);
			uint32_t tframes = frames + 1;
			trace = open_trace(path, tframes, (uint32_t)restw.bodies.size());
			if (dump_buf.empty())
			{
				dump_buf.resize(restw.bodies.size());
			}
			std::fprintf(stderr, "  trace %s (%u frames 0..=%u)\n", path, tframes, frames);
		}
		for (uint32_t step = 0; step <= frames; ++step)
		{
			if (dump && step < frames)
			{
				write_dump_frame(dump, restw, dump_buf);
			}
			if (trace)
			{
				write_trace_frame(trace, restw, dump_buf, step);
			}
			for (uint32_t cp : kCheckpoints)
			{
				if (cp == step && cp <= frames)
				{
					rest.push_back(measure_rest(restw, name, step));
				}
			}
			if (step == frames)
			{
				break;
			}
			b3World_Step(restw.world, kDt, kSubSteps);
		}
		if (dump)
		{
			std::fclose(dump);
		}
		if (trace)
		{
			std::fclose(trace);
		}
		destroy_scene(restw);

		float ms_per = wall_ms / (float)frames;
		float prof = profile_sum / (float)frames;
		json += "    \"";
		json += name;
		json += "\": {\n      \"bodies\": ";
		json += std::to_string(nbody);
		json += ",\n      \"workers\": ";
		json += std::to_string(workers);
		json += ",\n      \"wall_ms\": ";
		char num[64];
		std::snprintf(num, sizeof(num), "%.3f", wall_ms);
		json += num;
		json += ",\n      \"ms_per_step\": ";
		std::snprintf(num, sizeof(num), "%.4f", ms_per);
		json += num;
		json += ",\n      \"profile_ms_per_step\": ";
		std::snprintf(num, sizeof(num), "%.4f", prof);
		json += num;
		if (!profile_samples.empty())
		{
			std::vector<float> sorted = profile_samples;
			std::sort(sorted.begin(), sorted.end());
			auto pct = [&](float q) {
				size_t i = (size_t)std::clamp((int)std::lround((sorted.size() - 1) * q), 0, (int)sorted.size() - 1);
				return sorted[i];
			};
			json += ",\n      \"profile_p50_ms\": ";
			std::snprintf(num, sizeof(num), "%.4f", pct(0.5f));
			json += num;
			json += ",\n      \"profile_p95_ms\": ";
			std::snprintf(num, sizeof(num), "%.4f", pct(0.95f));
			json += num;
			json += ",\n      \"profile_samples_ms\": [";
			for (size_t i = 0; i < profile_samples.size(); ++i)
			{
				std::snprintf(num, sizeof(num), "%.4f", profile_samples[i]);
				json += num;
				if (i + 1 < profile_samples.size())
				{
					json += ",";
				}
			}
			json += "]";
		}
		if (!wall_samples.empty())
		{
			std::vector<float> sorted = wall_samples;
			std::sort(sorted.begin(), sorted.end());
			auto pct = [&](float q) {
				size_t i = (size_t)std::clamp((int)std::lround((sorted.size() - 1) * q), 0, (int)sorted.size() - 1);
				return sorted[i];
			};
			json += ",\n      \"wall_p50_ms\": ";
			std::snprintf(num, sizeof(num), "%.4f", pct(0.5f));
			json += num;
			json += ",\n      \"wall_p95_ms\": ";
			std::snprintf(num, sizeof(num), "%.4f", pct(0.95f));
			json += num;
			json += ",\n      \"wall_samples_ms\": [";
			for (size_t i = 0; i < wall_samples.size(); ++i)
			{
				std::snprintf(num, sizeof(num), "%.4f", wall_samples[i]);
				json += num;
				if (i + 1 < wall_samples.size())
				{
					json += ",";
				}
			}
			json += "]";
		}
		json += ",\n      \"rest\": [";
		for (size_t ri = 0; ri < rest.size(); ++ri)
		{
			const RestSnap& r = rest[ri];
			json += "{\"step\":";
			json += std::to_string(r.step);
			std::snprintf(num, sizeof(num), ",\"speed\":%.6e", r.speed);
			json += num;
			if (r.has_y)
			{
				std::snprintf(num, sizeof(num), ",\"y_err\":%.6e", r.y_err);
				json += num;
			}
			if (r.has_stack)
			{
				std::snprintf(num, sizeof(num), ",\"bottom_err\":%.6e,\"gap_err\":%.6e", r.bottom_err, r.gap_err);
				json += num;
			}
			if (r.has_floor)
			{
				std::snprintf(num, sizeof(num), ",\"floor_err\":%.6e", r.floor_err);
				json += num;
			}
			if (r.has_joint)
			{
				std::snprintf(num, sizeof(num), ",\"joint_err\":%.6e", r.joint_err);
				json += num;
			}
			if (r.has_hold)
			{
				std::snprintf(num, sizeof(num), ",\"hold_err\":%.6e", r.hold_err);
				json += num;
			}
			if (r.has_bounce)
			{
				std::snprintf(num, sizeof(num), ",\"bounce_y\":%.6e", r.bounce_y);
				json += num;
			}
			if (r.has_omega)
			{
				std::snprintf(num, sizeof(num), ",\"omega\":%.6e", r.omega);
				json += num;
			}
			if (r.has_pyramid)
			{
				std::snprintf(num, sizeof(num), ",\"fallen\":%u,\"xz_err\":%.6e", r.fallen, r.xz_err);
				json += num;
			}
			if (r.has_mixed)
			{
				std::snprintf(num, sizeof(num), ",\"min_y\":%.6e", r.min_y);
				json += num;
			}
			if (r.has_ramp)
			{
				std::snprintf(num, sizeof(num), ",\"slide_x\":%.6e,\"slide_y\":%.6e", r.slide_x, r.slide_y);
				json += num;
			}
			json += "}";
			if (ri + 1 < rest.size())
			{
				json += ", ";
			}
		}
		json += "]\n    }";
		if (si + 1 < scenes.size())
		{
			json += ",";
		}
		json += "\n";
		std::fprintf(stderr, "  bodies=%u wall=%.1f ms (%.3f ms/step, profile %.3f)\n", nbody, wall_ms, ms_per, prof);
	}

	json += "  }\n}\n";
	if (metrics_path)
	{
		FILE* f = std::fopen(metrics_path, "wb");
		if (!f)
		{
			std::fprintf(stderr, "cannot write %s\n", metrics_path);
			return 1;
		}
		std::fwrite(json.data(), 1, json.size(), f);
		std::fclose(f);
		std::fprintf(stderr, "wrote %s\n", metrics_path);
	}
	else
	{
		std::fputs(json.c_str(), stdout);
	}
	return 0;
}

#endif // GPU_PHYSICS_ORACLE_LIBRARY
