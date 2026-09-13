#include "box3d/box3d.h"
#include "box3d/collision.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" float gpu_b3_body_inv_mass(b3BodyId body);
extern "C" void gpu_b3_destroy_joint(b3JointId joint, bool wakeAttached);
extern "C" int gpu_b3_shape_kind(b3ShapeId shape);
extern "C" int gpu_b3_body_get_shape_count(b3BodyId body);
extern "C" void b3Shape_SetHeightField(b3ShapeId shapeId, const b3HeightFieldData* heightField);

struct OwnedHull
{
	b3HullData* p = nullptr;
	~OwnedHull()
	{
		if (p)
		{
			b3DestroyHull(p);
		}
	}
};

static b3HullData* create_octahedron_hull()
{
	const b3Vec3 points[6] = {
		{0.5f, 0.0f, 0.0f}, {-0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f},
		{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}, {0.0f, 0.0f, -0.5f},
	};
	return b3CreateHull(points, 6, 6);
}

static b3ShapeId attach_cooked_hull(b3BodyId body, const b3ShapeDef* def, b3HullData* hull)
{
	b3ShapeId id = {0};
	if (hull)
	{
		id = b3CreateHullShape(body, def, hull);
		b3DestroyHull(hull);
	}
	return id;
}

static b3ShapeId attach_octahedron(b3BodyId body, const b3ShapeDef* def)
{
	return attach_cooked_hull(body, def, create_octahedron_hull());
}

struct PackedHullCompound
{
	b3CompoundData header;
	b3SurfaceMaterial material;
	struct
	{
		b3Transform transform;
		uint32_t hullOffset;
		uint32_t materialIndex;
	} hull;
	b3BoxHull box;
};

static PackedHullCompound* make_packed_hull_compound()
{
	PackedHullCompound* packed = (PackedHullCompound*)calloc(1, sizeof(PackedHullCompound));
	if (packed == nullptr)
	{
		return nullptr;
	}
	packed->header.version = B3_COMPOUND_VERSION;
	packed->header.byteCount = (int)sizeof(PackedHullCompound);
	packed->header.materialOffset = (int)offsetof(PackedHullCompound, material);
	packed->header.materialCount = 1;
	packed->header.hullOffset = (int)offsetof(PackedHullCompound, hull);
	packed->header.hullCount = 1;
	packed->header.sharedHullCount = 1;
	packed->material = b3DefaultSurfaceMaterial();
	packed->hull.transform.p = b3Vec3_zero;
	packed->hull.transform.q = b3Quat_identity;
	packed->hull.hullOffset = (uint32_t)offsetof(PackedHullCompound, box);
	packed->hull.materialIndex = 0;
	packed->box = b3MakeOffsetBoxHull(4.0f, 0.5f, 4.0f, (b3Vec3){0.0f, -0.5f, 0.0f});
	return packed;
}

static size_t align8(size_t value)
{
	return (value + 7u) & ~size_t(7u);
}

static b3MeshData* make_grid_mesh(int cells, float halfWidth)
{
	const int side = cells + 1;
	const int vertexCount = side * side;
	const int triangleCount = 2 * cells * cells;
	if (triangleCount > 8)
	{
		return nullptr;
	}
	size_t nodeOffset = align8(sizeof(b3MeshData));
	size_t vertexOffset = align8(nodeOffset + sizeof(b3MeshNode));
	size_t triangleOffset = align8(vertexOffset + size_t(vertexCount) * sizeof(b3Vec3));
	size_t materialOffset = align8(triangleOffset + size_t(triangleCount) * sizeof(b3MeshTriangle));
	size_t flagsOffset = align8(materialOffset + size_t(triangleCount));
	size_t byteCount = align8(flagsOffset + size_t(triangleCount));
	b3MeshData* mesh = static_cast<b3MeshData*>(calloc(1, byteCount));
	mesh->version = B3_MESH_VERSION;
	mesh->hash = 1;
	mesh->byteCount = int32_t(byteCount);
	mesh->bounds = {{-halfWidth, 0.0f, -halfWidth}, {halfWidth, 0.0f, halfWidth}};
	mesh->nodeOffset = int32_t(nodeOffset);
	mesh->nodeCount = 1;
	mesh->vertexOffset = int32_t(vertexOffset);
	mesh->vertexCount = vertexCount;
	mesh->triangleOffset = int32_t(triangleOffset);
	mesh->triangleCount = triangleCount;
	mesh->materialOffset = int32_t(materialOffset);
	mesh->materialCount = 1;
	mesh->flagsOffset = int32_t(flagsOffset);
	b3MeshNode* node = reinterpret_cast<b3MeshNode*>(reinterpret_cast<char*>(mesh) + nodeOffset);
	node->lowerBound = mesh->bounds.lowerBound;
	node->upperBound = mesh->bounds.upperBound;
	node->data.asLeaf.type = 3;
	node->data.asLeaf.triangleCount = uint32_t(triangleCount);
	node->triangleOffset = 0;
	b3Vec3* vertices = reinterpret_cast<b3Vec3*>(reinterpret_cast<char*>(mesh) + vertexOffset);
	b3MeshTriangle* triangles =
		reinterpret_cast<b3MeshTriangle*>(reinterpret_cast<char*>(mesh) + triangleOffset);
	uint8_t* flags = reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(mesh) + flagsOffset);
	int vertex = 0;
	for (int z = 0; z < side; ++z)
	{
		for (int x = 0; x < side; ++x)
		{
			vertices[vertex++] = {-halfWidth + 2.0f * halfWidth * float(x) / float(cells), 0.0f,
								  -halfWidth + 2.0f * halfWidth * float(z) / float(cells)};
		}
	}
	int triangle = 0;
	for (int z = 0; z < cells; ++z)
	{
		for (int x = 0; x < cells; ++x)
		{
			int a = z * side + x;
			int b = a + 1;
			int c = a + side;
			int d = c + 1;
			triangles[triangle] = {a, d, b};
			flags[triangle++] = b3_allFlatEdges;
			triangles[triangle] = {a, c, d};
			flags[triangle++] = b3_allFlatEdges;
		}
	}
	return mesh;
}

static b3HeightFieldData* make_height_field(int cells, int mode, bool clockwise)
{
	const int columns = cells + 1;
	const int rows = cells + 1;
	const int heightCount = columns * rows;
	const int cellCount = cells * cells;
	const int triangleCount = 2 * cellCount;
	size_t heightsOffset = align8(sizeof(b3HeightFieldData));
	size_t materialOffset = align8(heightsOffset + size_t(heightCount) * sizeof(uint16_t));
	size_t flagsOffset = align8(materialOffset + size_t(cellCount));
	size_t byteCount = align8(flagsOffset + size_t(triangleCount));
	b3HeightFieldData* hf = static_cast<b3HeightFieldData*>(calloc(1, byteCount));
	hf->version = B3_HEIGHT_FIELD_VERSION;
	hf->hash = 1;
	hf->byteCount = int32_t(byteCount);
	hf->minHeight = -1.0f;
	hf->maxHeight = 1.0f;
	hf->heightScale = 2.0f / 65535.0f;
	hf->scale = {8.0f / float(cells), 1.0f, 8.0f / float(cells)};
	hf->columnCount = columns;
	hf->rowCount = rows;
	hf->heightsOffset = int32_t(heightsOffset);
	hf->materialOffset = int32_t(materialOffset);
	hf->flagsOffset = int32_t(flagsOffset);
	hf->clockwise = clockwise;
	uint16_t* heights = reinterpret_cast<uint16_t*>(reinterpret_cast<char*>(hf) + heightsOffset);
	uint8_t* materials = reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(hf) + materialOffset);
	uint8_t* flags = reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(hf) + flagsOffset);
	float low = FLT_MAX;
	float high = -FLT_MAX;
	for (int z = 0; z < rows; ++z)
	{
		for (int x = 0; x < columns; ++x)
		{
			float height = 0.0f;
			if (mode == 1)
			{
				height = -0.75f + 1.5f * float(x) / float(cells);
			}
			else if (mode == 2)
			{
				height = 0.35f * sinf(0.45f * float(x)) * cosf(0.35f * float(z));
			}
			float quantized = fminf(65535.0f, fmaxf(0.0f, (height + 1.0f) / hf->heightScale));
			heights[z * columns + x] = uint16_t(quantized);
			float decoded = hf->minHeight + hf->heightScale * float(heights[z * columns + x]);
			low = fminf(low, decoded);
			high = fmaxf(high, decoded);
		}
	}
	for (int i = 0; i < cellCount; ++i)
	{
		materials[i] = 0;
	}
	if (mode == 3)
	{
		materials[(cells / 2) * cells + cells / 2] = B3_HEIGHT_FIELD_HOLE;
	}
	for (int i = 0; i < triangleCount; ++i)
	{
		flags[i] = b3_allFlatEdges;
	}
	hf->aabb = {{0.0f, low, 0.0f}, {8.0f, high, 8.0f}};
	return hf;
}

static const uint32_t kCheckpoints[] = {0, 50, 100, 200, 300};

static b3BodyId add_ground(b3WorldId world, float extent)
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.position = b3Pos{0.0f, -1.0f, 0.0f};
	b3BodyId ground = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3BoxHull hull = b3MakeBoxHull(extent, 1.0f, extent);
	b3CreateHullShape(ground, &shapeDef, &hull.base);
	return ground;
}

static void dump_bodies(b3BodyId* bodies, int count, uint32_t step)
{
	printf("step %u\n", step);
	for (int i = 0; i < count; ++i)
	{
		b3Pos p = b3Body_GetPosition(bodies[i]);
		b3Quat q = b3Body_GetRotation(bodies[i]);
		printf("  %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n", i, p.x, p.y, p.z, q.v.x, q.v.y, q.v.z, q.s);
	}
}

static int run_filter_event_cohort(const char* name)
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	b3WorldId world = b3CreateWorld(&worldDef);
	b3BodyDef staticDef = b3DefaultBodyDef();
	b3BodyId bodyA = b3CreateBody(world, &staticDef);
	b3BodyDef dynamicDef = b3DefaultBodyDef();
	dynamicDef.type = b3_dynamicBody;
	dynamicDef.enableSleep = false;
	dynamicDef.position = {0.25f, 0.0f, 0.0f};
	b3BodyId bodyB = b3CreateBody(world, &dynamicDef);
	b3BoxHull box = b3MakeCubeHull(0.5f);
	b3ShapeDef defA = b3DefaultShapeDef();
	b3ShapeDef defB = b3DefaultShapeDef();
	defB.enableContactEvents = true;

	if (strcmp(name, "filter-mask") == 0 || strcmp(name, "filter-runtime") == 0)
	{
		defA.filter.categoryBits = 1;
		defA.filter.maskBits = 2;
		defB.filter.categoryBits = 2;
		defB.filter.maskBits = strcmp(name, "filter-mask") == 0 ? 0 : 1;
	}
	else if (strcmp(name, "filter-positive-group") == 0)
	{
		defA.filter.maskBits = 0;
		defB.filter.maskBits = 0;
		defA.filter.groupIndex = 5;
		defB.filter.groupIndex = 5;
	}
	else if (strcmp(name, "filter-negative-group") == 0)
	{
		defA.filter.groupIndex = -5;
		defB.filter.groupIndex = -5;
	}

	b3ShapeId shapeA = b3CreateHullShape(bodyA, &defA, &box.base);
	b3ShapeId shapeB = b3CreateHullShape(bodyB, &defB, &box.base);
	b3Filter roundTrip = b3Shape_GetFilter(shapeB);
	if (roundTrip.categoryBits != defB.filter.categoryBits || roundTrip.maskBits != defB.filter.maskBits ||
		roundTrip.groupIndex != defB.filter.groupIndex || !b3Shape_AreContactEventsEnabled(shapeB))
	{
		fprintf(stderr, "%s shape API round-trip failed\n", name);
		return 2;
	}

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3ContactEvents first = b3World_GetContactEvents(world);
	printf("%s first begin=%d end=%d hit=%d\n", name, first.beginCount, first.endCount, first.hitCount);
	bool shouldBegin = strcmp(name, "filter-positive-group") == 0 || strcmp(name, "filter-runtime") == 0 ||
					   strcmp(name, "contact-events") == 0 || strcmp(name, "contact-events-destroy") == 0;
	if (first.beginCount != (shouldBegin ? 1 : 0) || first.endCount != 0 || first.hitCount != 0)
	{
		return 3;
	}

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3ContactEvents persistent = b3World_GetContactEvents(world);
	printf("%s persist begin=%d end=%d\n", name, persistent.beginCount, persistent.endCount);
	if (persistent.beginCount != 0 || persistent.endCount != 0)
	{
		return 4;
	}

	if (strcmp(name, "filter-runtime") == 0)
	{
		b3Filter filter = b3Shape_GetFilter(shapeB);
		filter.maskBits = 0;
		b3Shape_SetFilter(shapeB, filter, true);
	}
	else if (strcmp(name, "contact-events") == 0)
	{
		b3Body_SetTransform(bodyB, {4.0f, 0.0f, 0.0f}, b3Quat_identity);
	}
	else if (strcmp(name, "contact-events-destroy") == 0)
	{
		b3DestroyShape(shapeB, false);
	}
	else
	{
		b3DestroyWorld(world);
		return 0;
	}

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3ContactEvents ended = b3World_GetContactEvents(world);
	printf("%s changed begin=%d end=%d\n", name, ended.beginCount, ended.endCount);
	b3DestroyWorld(world);
	return ended.beginCount == 0 && ended.endCount == 1 ? 0 : 5;
}

static int run_sensor_cohort(const char* name)
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	b3WorldId world = b3CreateWorld(&worldDef);

	b3BodyDef sensorBodyDef = b3DefaultBodyDef();
	b3BodyId sensorBody = b3CreateBody(world, &sensorBodyDef);
	b3ShapeDef sensorDef = b3DefaultShapeDef();
	sensorDef.isSensor = true;
	sensorDef.enableSensorEvents = true;
	if (strcmp(name, "sensor-filter") == 0)
	{
		sensorDef.filter.maskBits = 0;
	}
	b3BoxHull sensorHull = b3MakeCubeHull(1.0f);
	b3ShapeId sensorShape = b3CreateHullShape(sensorBody, &sensorDef, &sensorHull.base);

	b3BodyDef visitorBodyDef = b3DefaultBodyDef();
	visitorBodyDef.type = b3_dynamicBody;
	visitorBodyDef.linearVelocity = {1.0f, 0.0f, 0.0f};
	visitorBodyDef.enableSleep = false;
	b3BodyId visitorBody = b3CreateBody(world, &visitorBodyDef);
	b3ShapeDef visitorDef = b3DefaultShapeDef();
	visitorDef.enableSensorEvents = true;
	b3Sphere visitorSphere = {b3Vec3_zero, 0.25f};
	b3ShapeId visitorShape = b3CreateSphereShape(visitorBody, &visitorDef, &visitorSphere);

	if (!b3Shape_IsSensor(sensorShape) || b3Shape_IsSensor(visitorShape) ||
		!b3Shape_AreSensorEventsEnabled(sensorShape) || !b3Shape_AreSensorEventsEnabled(visitorShape))
	{
		fprintf(stderr, "%s sensor API round-trip failed\n", name);
		return 20;
	}

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3SensorEvents first = b3World_GetSensorEvents(world);
	bool filtered = strcmp(name, "sensor-filter") == 0;
	printf("%s first begin=%d end=%d\n", name, first.beginCount, first.endCount);
	if (first.beginCount != (filtered ? 0 : 1) || first.endCount != 0)
	{
		fprintf(stderr, "%s unexpected first sensor events\n", name);
		return 21;
	}
	if (!filtered)
	{
		if (first.beginEvents[0].sensorShapeId.index1 != sensorShape.index1 ||
			first.beginEvents[0].visitorShapeId.index1 != visitorShape.index1)
		{
			fprintf(stderr, "%s sensor event roles are reversed\n", name);
			return 22;
		}
		b3ShapeId overlaps[2] = {};
		int capacity = b3Shape_GetSensorCapacity(sensorShape);
		int count = b3Shape_GetSensorData(sensorShape, overlaps, 2);
		if (capacity != 1 || count != 1 || overlaps[0].index1 != visitorShape.index1)
		{
			fprintf(stderr, "%s sensor data mismatch\n", name);
			return 23;
		}
	}

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3SensorEvents persistent = b3World_GetSensorEvents(world);
	if (persistent.beginCount != 0 || persistent.endCount != 0)
	{
		fprintf(stderr, "%s repeated persistent sensor event\n", name);
		return 24;
	}
	float expectedX = 2.0f / 60.0f;
	float actualX = b3Body_GetPosition(visitorBody).x;
	if (fabsf(actualX - expectedX) > 1.0e-4f)
	{
		fprintf(stderr, "%s sensor changed trajectory: x=%g expected=%g\n", name, actualX, expectedX);
		return 25;
	}

	if (strcmp(name, "sensor-filter-change") == 0)
	{
		b3Filter filter = b3Shape_GetFilter(visitorShape);
		filter.maskBits = 0;
		b3Shape_SetFilter(visitorShape, filter, true);
	}
	else if (strcmp(name, "sensor-destroy") == 0)
	{
		b3DestroyShape(visitorShape, false);
	}
	else if (strcmp(name, "sensor-disable") == 0)
	{
		b3Shape_EnableSensorEvents(visitorShape, false);
	}

	int endCount = 0;
	if (!filtered)
	{
		for (int i = 0; i < 120 && endCount == 0; ++i)
		{
			b3World_Step(world, 1.0f / 60.0f, 4);
			b3SensorEvents events = b3World_GetSensorEvents(world);
			if (events.beginCount != 0 || events.endCount > 1)
			{
				fprintf(stderr, "%s invalid sensor event cadence\n", name);
				return 26;
			}
			endCount += events.endCount;
		}
		if (endCount != 1 || b3Shape_GetSensorCapacity(sensorShape) != 0)
		{
			fprintf(stderr, "%s expected exactly one sensor end\n", name);
			return 27;
		}
	}
	printf("%s final end=%d x=%.6f\n", name, endCount, b3Body_GetPosition(visitorBody).x);
	b3DestroyWorld(world);
	return 0;
}

struct CallbackContext
{
	b3ShapeId shapeA;
	b3ShapeId shapeB;
	int customCount;
	int preSolveCount;
	bool customAccept;
	bool preSolveAccept;
	bool badContextOrId;
	int rejectShapeIndex;
	b3Pos point;
	b3Vec3 normal;
};

static bool custom_filter_callback(b3ShapeId shapeIdA, b3ShapeId shapeIdB, void* raw)
{
	CallbackContext* context = static_cast<CallbackContext*>(raw);
	if (context == nullptr)
	{
		return false;
	}
	context->customCount += 1;
	bool idsMatch = (shapeIdA.index1 == context->shapeA.index1 && shapeIdB.index1 == context->shapeB.index1) ||
					(shapeIdA.index1 == context->shapeB.index1 && shapeIdB.index1 == context->shapeA.index1);
	context->badContextOrId = context->badContextOrId || !idsMatch ||
							  shapeIdA.world0 != context->shapeA.world0 || shapeIdB.world0 != context->shapeA.world0;
	return context->customAccept;
}

static bool pre_solve_callback(b3ShapeId shapeIdA, b3ShapeId shapeIdB, b3Pos point, b3Vec3 normal, void* raw)
{
	CallbackContext* context = static_cast<CallbackContext*>(raw);
	if (context == nullptr)
	{
		return false;
	}
	context->preSolveCount += 1;
	context->point = point;
	context->normal = normal;
	bool idsMatch = (shapeIdA.index1 == context->shapeA.index1 && shapeIdB.index1 == context->shapeB.index1) ||
					(shapeIdA.index1 == context->shapeB.index1 && shapeIdB.index1 == context->shapeA.index1);
	context->badContextOrId = context->badContextOrId || !idsMatch;
	if (context->rejectShapeIndex != 0 &&
		(shapeIdA.index1 == context->rejectShapeIndex || shapeIdB.index1 == context->rejectShapeIndex))
	{
		return false;
	}
	return context->preSolveAccept;
}

static int run_callback_cohort(const char* name)
{
	bool sensor = strcmp(name, "callback-custom-sensor") == 0;
	bool custom = strncmp(name, "callback-custom", 15) == 0;
	bool optIn = strcmp(name, "callback-custom-opt-in") != 0;
	bool reject = strcmp(name, "callback-custom-reject") == 0 || sensor;

	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	b3WorldId world = b3CreateWorld(&worldDef);
	b3BodyDef groundDef = b3DefaultBodyDef();
	b3BodyId ground = b3CreateBody(world, &groundDef);
	b3ShapeDef groundShapeDef = b3DefaultShapeDef();
	groundShapeDef.isSensor = sensor;
	groundShapeDef.enableSensorEvents = sensor;
	groundShapeDef.enableContactEvents = !sensor;
	groundShapeDef.enableCustomFiltering = custom && optIn;
	groundShapeDef.enablePreSolveEvents = !custom;
	b3BoxHull groundHull = b3MakeBoxHull(2.0f, 0.5f, 2.0f);
	b3ShapeId groundShape = b3CreateHullShape(ground, &groundShapeDef, &groundHull.base);

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = {0.0f, 0.6f, 0.0f};
	bodyDef.linearVelocity = {0.0f, -2.0f, 0.0f};
	bodyDef.enableSleep = false;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.enableSensorEvents = sensor;
	shapeDef.enableContactEvents = !sensor;
	shapeDef.enableHitEvents = !sensor;
	b3Sphere sphere = {b3Vec3_zero, 0.25f};
	b3ShapeId shape = b3CreateSphereShape(body, &shapeDef, &sphere);

	CallbackContext context = {};
	context.shapeA = groundShape;
	context.shapeB = shape;
	context.customAccept = !reject;
	context.preSolveAccept = false;
	if (custom)
	{
		b3World_SetCustomFilterCallback(world, custom_filter_callback, &context);
	}
	else
	{
		if (!b3Shape_ArePreSolveEventsEnabled(groundShape))
		{
			fprintf(stderr, "%s pre-solve ShapeDef flag missing\n", name);
			return 30;
		}
		b3World_SetPreSolveCallback(world, pre_solve_callback, &context);
	}

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3ContactEvents firstContacts = b3World_GetContactEvents(world);
	b3SensorEvents firstSensors = b3World_GetSensorEvents(world);
	float firstVelocity = b3Body_GetLinearVelocity(body).y;
	if (context.badContextOrId || (custom && optIn && context.customCount == 0) ||
		(custom && !optIn && context.customCount != 0))
	{
		fprintf(stderr, "%s callback context/id/opt-in mismatch custom=%d bad=%d\n", name, context.customCount,
				context.badContextOrId);
		return 31;
	}
	if (sensor)
	{
		if (firstSensors.beginCount != 0)
		{
			fprintf(stderr, "%s rejected sensor emitted begin\n", name);
			return 32;
		}
	}
	else if (custom)
	{
		int expectedBegin = reject ? 0 : 1;
		if (firstContacts.beginCount != expectedBegin)
		{
			fprintf(stderr, "%s begin=%d expected=%d\n", name, firstContacts.beginCount, expectedBegin);
			return 33;
		}
	}
	else
	{
		if (context.preSolveCount == 0 || firstContacts.beginCount != 0 || firstContacts.hitCount != 0 ||
			fabsf(firstVelocity + 2.0f) > 1.0e-4f)
		{
			fprintf(stderr, "%s first pre=%d begin=%d hit=%d vy=%g\n", name, context.preSolveCount,
					firstContacts.beginCount, firstContacts.hitCount, firstVelocity);
			return 34;
		}
		context.preSolveAccept = true;
		b3World_Step(world, 1.0f / 60.0f, 4);
		b3ContactEvents second = b3World_GetContactEvents(world);
		if (context.preSolveCount < 2 || second.beginCount != 1 ||
			(context.normal.x == 0.0f && context.normal.y == 0.0f && context.normal.z == 0.0f))
		{
			fprintf(stderr, "%s re-enable pre=%d begin=%d normal=(%g,%g,%g)\n", name, context.preSolveCount,
					second.beginCount, context.normal.x, context.normal.y, context.normal.z);
			return 35;
		}
		b3Shape_EnablePreSolveEvents(groundShape, false);
		if (b3Shape_ArePreSolveEventsEnabled(groundShape))
		{
			fprintf(stderr, "%s pre-solve runtime disable failed\n", name);
			return 36;
		}
	}
	printf("%s custom=%d pre=%d contactBegin=%d sensorBegin=%d vy=%.6f\n", name, context.customCount,
		   context.preSolveCount, firstContacts.beginCount, firstSensors.beginCount, firstVelocity);
	b3World_SetCustomFilterCallback(world, nullptr, nullptr);
	b3World_SetPreSolveCallback(world, nullptr, nullptr);
	b3DestroyWorld(world);
	return 0;
}

static int run_body_event_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	b3WorldId world = b3CreateWorld(&worldDef);

	b3BodyDef staticDef = b3DefaultBodyDef();
	b3CreateBody(world, &staticDef);
	b3BodyDef dynamicDef = b3DefaultBodyDef();
	dynamicDef.type = b3_dynamicBody;
	dynamicDef.enableSleep = false;
	dynamicDef.linearVelocity = {1.0f, 0.0f, 0.0f};
	dynamicDef.userData = (void*)(uintptr_t)0x1234;
	b3BodyId dynamicBody = b3CreateBody(world, &dynamicDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3Sphere sphere = {b3Vec3_zero, 0.25f};
	b3CreateSphereShape(dynamicBody, &shapeDef, &sphere);

	b3BodyDef kinematicDef = b3DefaultBodyDef();
	kinematicDef.type = b3_kinematicBody;
	kinematicDef.linearVelocity = {0.0f, 1.0f, 0.0f};
	b3BodyId kinematicBody = b3CreateBody(world, &kinematicDef);
	b3CreateSphereShape(kinematicBody, &shapeDef, &sphere);

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3BodyEvents events = b3World_GetBodyEvents(world);
	printf("body-events move=%d\n", events.moveCount);
	if (events.moveCount != 2)
	{
		b3DestroyWorld(world);
		return 30;
	}
	bool foundDynamic = false;
	for (int i = 0; i < events.moveCount; ++i)
	{
		const b3BodyMoveEvent* event = events.moveEvents + i;
		if (B3_ID_EQUALS(event->bodyId, dynamicBody))
		{
			foundDynamic = event->userData == (void*)(uintptr_t)0x1234 && !event->fellAsleep &&
						   event->transform.p.x > 0.0f;
		}
	}
	if (!foundDynamic || b3Body_GetUserData(dynamicBody) != (void*)(uintptr_t)0x1234)
	{
		b3DestroyWorld(world);
		return 31;
	}
	b3Body_SetUserData(dynamicBody, (void*)(uintptr_t)0x5678);
	if (b3Body_GetUserData(dynamicBody) != (void*)(uintptr_t)0x5678)
	{
		b3DestroyWorld(world);
		return 32;
	}
	b3DestroyWorld(world);
	worldDef = b3DefaultWorldDef();
	world = b3CreateWorld(&worldDef);
	add_ground(world, 10.0f);
	b3BodyDef sleeperDef = b3DefaultBodyDef();
	sleeperDef.type = b3_dynamicBody;
	sleeperDef.position = {0.0f, 0.5f, 0.0f};
	b3BodyId sleeper = b3CreateBody(world, &sleeperDef);
	b3BoxHull sleeperHull = b3MakeCubeHull(0.5f);
	b3CreateHullShape(sleeper, &shapeDef, &sleeperHull.base);
	bool fellAsleep = false;
	for (int step = 0; step < 600 && !fellAsleep; ++step)
	{
		b3World_Step(world, 1.0f / 60.0f, 4);
		b3BodyEvents sleepEvents = b3World_GetBodyEvents(world);
		for (int i = 0; i < sleepEvents.moveCount; ++i)
		{
			fellAsleep = fellAsleep || (B3_ID_EQUALS(sleepEvents.moveEvents[i].bodyId, sleeper) &&
									   sleepEvents.moveEvents[i].fellAsleep);
		}
	}
	if (!fellAsleep)
	{
		b3DestroyWorld(world);
		return 33;
	}
	b3World_Step(world, 1.0f / 60.0f, 4);
	if (b3World_GetBodyEvents(world).moveCount != 0)
	{
		b3DestroyWorld(world);
		return 34;
	}
	b3DestroyWorld(world);
	return 0;
}

static int run_hit_case(float threshold, bool enabled, bool expectHit)
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.hitEventThreshold = threshold;
	b3WorldId world = b3CreateWorld(&worldDef);
	if (fabsf(b3World_GetHitEventThreshold(world) - threshold) > 1e-6f)
	{
		return 40;
	}
	add_ground(world, 10.0f);
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = {0.0f, 2.0f, 0.0f};
	bodyDef.linearVelocity = {0.0f, -5.0f, 0.0f};
	bodyDef.enableSleep = false;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.enableHitEvents = enabled;
	shapeDef.baseMaterial.userMaterialId = 77;
	b3Sphere sphere = {b3Vec3_zero, 0.25f};
	b3ShapeId shape = b3CreateSphereShape(body, &shapeDef, &sphere);
	if (b3Shape_AreHitEventsEnabled(shape) != enabled)
	{
		return 41;
	}

	int hitCount = 0;
	for (int step = 0; step < 90; ++step)
	{
		b3World_Step(world, 1.0f / 60.0f, 4);
		b3ContactEvents events = b3World_GetContactEvents(world);
		if (events.hitCount > 1)
		{
			return 42;
		}
		hitCount += events.hitCount;
		for (int i = 0; i < events.hitCount; ++i)
		{
			const b3ContactHitEvent* hit = events.hitEvents + i;
			printf("  hit speed=%.9g normal=(%.9g %.9g %.9g) materials=%llu/%llu\n", hit->approachSpeed,
				   hit->normal.x, hit->normal.y, hit->normal.z, (unsigned long long)hit->userMaterialIdA,
				   (unsigned long long)hit->userMaterialIdB);
			bool materialMatches = hit->userMaterialIdA == 77 || hit->userMaterialIdB == 77;
			bool orientationMatches = (hit->userMaterialIdA == 77 && hit->normal.y < -0.9f) ||
									  (hit->userMaterialIdB == 77 && hit->normal.y > 0.9f);
			if (hit->approachSpeed <= threshold || !materialMatches || !orientationMatches)
			{
				return 43;
			}
		}
	}
	printf("hit-events threshold=%.1f enabled=%d hits=%d\n", threshold, enabled, hitCount);
	b3World_SetHitEventThreshold(world, 100.0f);
	b3World_Step(world, 1.0f / 60.0f, 4);
	if (b3World_GetContactEvents(world).hitCount != 0 ||
		fabsf(b3World_GetHitEventThreshold(world) - 100.0f) > 1e-6f)
	{
		return 44;
	}
	b3DestroyWorld(world);
	return (hitCount > 0) == expectHit ? 0 : 45;
}

static int run_hit_event_cohort()
{
	int result = run_hit_case(100.0f, true, false);
	if (result != 0)
	{
		return result;
	}
	result = run_hit_case(1.0f, false, false);
	if (result != 0)
	{
		return result;
	}
	return run_hit_case(1.0f, true, true);
}

static b3BodyId make_joint_event_body(b3WorldId world, b3Pos position, b3Vec3 angularVelocity)
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = position;
	bodyDef.angularVelocity = angularVelocity;
	bodyDef.enableSleep = false;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3BoxHull hull = b3MakeBoxHull(0.5f, 0.5f, 0.5f);
	b3CreateHullShape(body, &shapeDef, &hull.base);
	return body;
}

static int run_joint_event_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	b3WorldId world = b3CreateWorld(&worldDef);
	b3BodyDef staticDef = b3DefaultBodyDef();
	b3BodyId ground = b3CreateBody(world, &staticDef);

	b3BodyId distanceBody = make_joint_event_body(world, {-4.0f, 2.0f, 0.0f}, b3Vec3_zero);
	b3DistanceJointDef distanceDef = b3DefaultDistanceJointDef();
	if (distanceDef.base.forceThreshold != FLT_MAX || distanceDef.base.torqueThreshold != FLT_MAX)
	{
		fprintf(stderr, "joint threshold defaults are not FLT_MAX\n");
		return 2;
	}
	distanceDef.base.bodyIdA = ground;
	distanceDef.base.bodyIdB = distanceBody;
	distanceDef.base.localFrameA.p = {-4.0f, 4.0f, 0.0f};
	distanceDef.length = 2.0f;
	distanceDef.enableSpring = true;
	distanceDef.enableMotor = true;
	distanceDef.maxMotorForce = 1000.0f;
	distanceDef.motorSpeed = 10.0f;
	distanceDef.base.userData = (void*)(uintptr_t)0xD157;
	b3JointId distanceJoint = b3CreateDistanceJoint(world, &distanceDef);

	b3BodyId weldBody = make_joint_event_body(world, {4.0f, 4.0f, 0.0f}, {0.0f, 0.0f, 5.0f});
	b3WeldJointDef weldDef = b3DefaultWeldJointDef();
	weldDef.base.bodyIdA = ground;
	weldDef.base.bodyIdB = weldBody;
	weldDef.base.localFrameA.p = {4.0f, 4.0f, 0.0f};
	weldDef.base.forceThreshold = 0.02f;
	weldDef.base.userData = (void*)(uintptr_t)0xA11;
	b3JointId weldJoint = b3CreateWeldJoint(world, &weldDef);

	b3BodyId disabledBody = make_joint_event_body(world, {0.0f, 2.0f, 0.0f}, b3Vec3_zero);
	b3RevoluteJointDef disabledDef = b3DefaultRevoluteJointDef();
	disabledDef.base.bodyIdA = ground;
	disabledDef.base.bodyIdB = disabledBody;
	disabledDef.base.localFrameA.p = {0.0f, 4.0f, 0.0f};
	disabledDef.base.torqueThreshold = 0.0f;
	disabledDef.base.userData = (void*)(uintptr_t)0xBAD;
	b3JointId disabledJoint = b3CreateRevoluteJoint(world, &disabledDef);

	b3FilterJointDef filterDef = b3DefaultFilterJointDef();
	filterDef.base.bodyIdA = distanceBody;
	filterDef.base.bodyIdB = weldBody;
	filterDef.base.forceThreshold = 0.0f;
	filterDef.base.torqueThreshold = 0.0f;
	filterDef.base.userData = (void*)(uintptr_t)0xF117;
	b3CreateFilterJoint(world, &filterDef);

	b3DistanceJointDef destroyedDef = b3DefaultDistanceJointDef();
	destroyedDef.base.bodyIdA = ground;
	destroyedDef.base.bodyIdB = disabledBody;
	destroyedDef.base.forceThreshold = 0.0f;
	destroyedDef.base.userData = (void*)(uintptr_t)0xDEAD;
	b3JointId destroyedJoint = b3CreateDistanceJoint(world, &destroyedDef);
	gpu_b3_destroy_joint(destroyedJoint, false);

	b3Joint_SetForceThreshold(weldJoint, 0.02f);
	b3Joint_SetTorqueThreshold(disabledJoint, 0.0f);
	b3Joint_SetUserData(weldJoint, (void*)(uintptr_t)0xBEEF);
	if (b3Joint_GetForceThreshold(weldJoint) != 0.02f ||
		b3Joint_GetTorqueThreshold(distanceJoint) != FLT_MAX ||
		b3Joint_GetTorqueThreshold(disabledJoint) != 0.0f ||
		b3Joint_GetUserData(weldJoint) != (void*)(uintptr_t)0xBEEF ||
		b3Joint_GetForceThreshold(distanceJoint) != FLT_MAX)
	{
		fprintf(stderr, "joint threshold/user-data round trip failed\n");
		b3DestroyWorld(world);
		return 3;
	}

	b3World_Step(world, 1.0f / 60.0f, 4);
	b3JointEvents events = b3World_GetJointEvents(world);
	bool foundTorque = false;
	bool foundWeld = false;
	for (int i = 0; i < events.count; ++i)
	{
		b3JointEvent event = events.jointEvents[i];
		if (event.jointId.index1 == disabledJoint.index1 && event.userData == (void*)(uintptr_t)0xBAD)
		{
			foundTorque = true;
		}
		else if (event.jointId.index1 == weldJoint.index1 && event.userData == (void*)(uintptr_t)0xBEEF)
		{
			foundWeld = true;
		}
		else
		{
			fprintf(stderr, "unexpected joint event id=%d user=%p\n", event.jointId.index1, event.userData);
			b3DestroyWorld(world);
			return 4;
		}
	}
	if (events.count != 2 || !foundTorque || !foundWeld)
	{
		fprintf(stderr, "joint events mismatch: count=%d torque=%d weld=%d\n", events.count, foundTorque, foundWeld);
		b3DestroyWorld(world);
		return 5;
	}

	b3Joint_SetForceThreshold(weldJoint, FLT_MAX);
	b3Joint_SetTorqueThreshold(disabledJoint, FLT_MAX);
	b3World_Step(world, 1.0f / 60.0f, 4);
	events = b3World_GetJointEvents(world);
	if (events.count != 0)
	{
		fprintf(stderr, "joint events did not clear: count=%d\n", events.count);
		b3DestroyWorld(world);
		return 6;
	}

	printf("joint-events force=1 torque=1 cleared=1 destroyed=1 default=1 filter=1\n");
	b3DestroyWorld(world);
	return 0;
}

static int run_ccd_cohort(const char* name)
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	if (strcmp(name, "ccd-disabled") == 0)
	{
		worldDef.enableContinuous = false;
	}
	b3WorldId world = b3CreateWorld(&worldDef);
	if (b3World_IsContinuousEnabled(world) != worldDef.enableContinuous)
	{
		fprintf(stderr, "%s continuous world flag mismatch\n", name);
		return 40;
	}
	b3World_EnableContinuous(world, worldDef.enableContinuous);

	bool sensor = strcmp(name, "ccd-sensor") == 0;
	bool dynamicTarget = strcmp(name, "ccd-bullet-dynamic") == 0 ||
						 strcmp(name, "ccd-nonbullet-dynamic") == 0 ||
						 strcmp(name, "ccd-bullet-bullet") == 0;
	bool kinematicTarget = strcmp(name, "ccd-bullet-kinematic") == 0;
	b3BodyDef targetBodyDef = b3DefaultBodyDef();
	targetBodyDef.type = dynamicTarget ? b3_dynamicBody : (kinematicTarget ? b3_kinematicBody : b3_staticBody);
	targetBodyDef.enableSleep = false;
	b3BodyId targetBody = b3CreateBody(world, &targetBodyDef);
	if (strcmp(name, "ccd-bullet-bullet") == 0)
	{
		b3Body_SetBullet(targetBody, true);
	}
	b3ShapeDef targetShapeDef = b3DefaultShapeDef();
	targetShapeDef.isSensor = sensor;
	targetShapeDef.enableSensorEvents = sensor;
	targetShapeDef.enablePreSolveEvents = strcmp(name, "ccd-pre-solve") == 0;
	targetShapeDef.enableCustomFiltering = strcmp(name, "ccd-custom-filter") == 0;
	b3BoxHull wall = b3MakeBoxHull(0.05f, 2.0f, 2.0f);
	b3ShapeId targetShape = b3CreateHullShape(targetBody, &targetShapeDef, &wall.base);

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = {strcmp(name, "ccd-initial-overlap") == 0 ? 0.0f : -2.0f, 0.0f, 0.0f};
	bodyDef.linearVelocity = {120.0f, 0.0f, 0.0f};
	bodyDef.enableSleep = false;
	bodyDef.isBullet = strncmp(name, "ccd-bullet-", 11) == 0;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	if (b3Body_IsBullet(body) != bodyDef.isBullet)
	{
		fprintf(stderr, "%s bullet default mismatch\n", name);
		return 41;
	}
	b3Body_SetBullet(body, bodyDef.isBullet);
	b3Body_AllowFastRotation(body, false);
	if (b3Body_IsFastRotationAllowed(body))
	{
		fprintf(stderr, "%s fast rotation default mismatch\n", name);
		return 42;
	}
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.enableSensorEvents = sensor;
	shapeDef.enablePreSolveEvents = strcmp(name, "ccd-pre-solve") == 0;
	shapeDef.enableCustomFiltering = strcmp(name, "ccd-custom-filter") == 0;
	b3ShapeId visitorShape = {0};
	if (strcmp(name, "ccd-capsule") == 0)
	{
		b3Capsule capsule = {{0.0f, -0.3f, 0.0f}, {0.0f, 0.3f, 0.0f}, 0.1f};
		visitorShape = b3CreateCapsuleShape(body, &shapeDef, &capsule);
	}
	else if (strcmp(name, "ccd-box") == 0)
	{
		b3BoxHull box = b3MakeBoxHull(0.2f, 0.2f, 0.2f);
		visitorShape = b3CreateHullShape(body, &shapeDef, &box.base);
	}
	else if (strcmp(name, "ccd-hull") == 0)
	{
		visitorShape = attach_octahedron(body, &shapeDef);
	}
	else
	{
		b3Sphere sphere = {b3Vec3_zero, 0.2f};
		visitorShape = b3CreateSphereShape(body, &shapeDef, &sphere);
		if (strcmp(name, "ccd-multi-shape") == 0)
		{
			sphere.center = {0.0f, 0.5f, 0.0f};
			b3CreateSphereShape(body, &shapeDef, &sphere);
		}
	}

	CallbackContext callbackContext = {};
	if (strcmp(name, "ccd-custom-filter") == 0)
	{
		callbackContext.shapeA = targetShape;
		callbackContext.shapeB = visitorShape;
		callbackContext.customAccept = false;
		b3World_SetCustomFilterCallback(world, custom_filter_callback, &callbackContext);
	}
	else if (strcmp(name, "ccd-pre-solve") == 0)
	{
		b3BodyDef secondBodyDef = b3DefaultBodyDef();
		secondBodyDef.position = {1.0f, 0.0f, 0.0f};
		b3BodyId secondBody = b3CreateBody(world, &secondBodyDef);
		b3CreateHullShape(secondBody, &targetShapeDef, &wall.base);
		callbackContext.shapeA = targetShape;
		callbackContext.shapeB = visitorShape;
		callbackContext.preSolveAccept = true;
		callbackContext.rejectShapeIndex = targetShape.index1;
		b3World_SetPreSolveCallback(world, pre_solve_callback, &callbackContext);
	}

	b3World_Step(world, 1.0f / 30.0f, 4);
	float x = b3Body_GetPosition(body).x;
	bool shouldTunnel = strcmp(name, "ccd-disabled") == 0 ||
						strcmp(name, "ccd-nonbullet-dynamic") == 0 ||
						strcmp(name, "ccd-bullet-bullet") == 0 ||
						strcmp(name, "ccd-initial-overlap") == 0 ||
						strcmp(name, "ccd-custom-filter") == 0 ||
						sensor;
	bool badPosition = strcmp(name, "ccd-pre-solve") == 0 ? (x < 0.5f || x > 0.9f)
														 : (shouldTunnel ? x < 0.5f : x > -0.05f);
	if (badPosition)
	{
		fprintf(stderr, "%s unexpected x=%g\n", name, x);
		return 43;
	}
	if (strcmp(name, "ccd-pre-solve") == 0 && callbackContext.preSolveCount < 2)
	{
		fprintf(stderr, "%s veto did not continue to next target x=%g calls=%d\n", name, x,
				callbackContext.preSolveCount);
		return 48;
	}
	if (strcmp(name, "ccd-custom-filter") == 0 && callbackContext.customCount == 0)
	{
		fprintf(stderr, "%s callback was not invoked\n", name);
		return 49;
	}
	if (sensor)
	{
		b3SensorEvents begin = b3World_GetSensorEvents(world);
		if (begin.beginCount != 1 || begin.endCount != 0 ||
			begin.beginEvents[0].sensorShapeId.index1 != targetShape.index1)
		{
			fprintf(stderr, "%s missing sensor CCD begin\n", name);
			return 44;
		}
		b3World_Step(world, 1.0f / 30.0f, 4);
		b3SensorEvents end = b3World_GetSensorEvents(world);
		if (end.beginCount != 0 || end.endCount != 1 || b3Body_GetPosition(body).x < 4.0f)
		{
			fprintf(stderr, "%s sensor CCD response/end mismatch\n", name);
			return 45;
		}
	}
	printf("%s x=%.6f continuous=%d bullet=%d sensor=%d\n", name, x,
		   b3World_IsContinuousEnabled(world), b3Body_IsBullet(body), sensor);
	b3DestroyWorld(world);
	return 0;
}

static int run_ccd_spinning_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	b3WorldId world = b3CreateWorld(&worldDef);
	b3BodyDef targetDef = b3DefaultBodyDef();
	targetDef.position = {0.0f, 0.85f, 0.0f};
	b3BodyId target = b3CreateBody(world, &targetDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3Sphere targetSphere = {b3Vec3_zero, 0.08f};
	b3CreateSphereShape(target, &shapeDef, &targetSphere);

	b3BodyDef stickDef = b3DefaultBodyDef();
	stickDef.type = b3_dynamicBody;
	stickDef.angularVelocity = {0.0f, 0.0f, 120.0f};
	stickDef.enableSleep = false;
	stickDef.allowFastRotation = true;
	b3BodyId stick = b3CreateBody(world, &stickDef);
	b3BoxHull stickHull = b3MakeBoxHull(1.0f, 0.03f, 0.03f);
	b3CreateHullShape(stick, &shapeDef, &stickHull.base);
	if (!b3Body_IsFastRotationAllowed(stick))
	{
		fprintf(stderr, "ccd-spinning-stick fast rotation flag mismatch\n");
		return 46;
	}
	b3World_Step(world, 1.0f / 30.0f, 4);
	b3Quat rotation = b3Body_GetRotation(stick);
	if (fabsf(rotation.v.z) > 0.7f)
	{
		fprintf(stderr, "ccd-spinning-stick missed rotational sweep qz=%g\n", rotation.v.z);
		return 47;
	}
	printf("ccd-spinning-stick qz=%.6f\n", rotation.v.z);
	b3DestroyWorld(world);
	return 0;
}

static bool reject_first_terrain_triangle(b3ShapeId, b3ShapeId, b3Pos, b3Vec3, void* raw)
{
	int* count = static_cast<int*>(raw);
	*count += 1;
	return *count > 1;
}

static int g_materialFrictionMixCount;
static int g_materialRestitutionMixCount;

static float terrain_friction_mix(float, uint64_t materialA, float, uint64_t materialB)
{
	g_materialFrictionMixCount += 1;
	return materialA == 202 || materialB == 202 ? 1.0f : 0.0f;
}

static float terrain_restitution_mix(float restitutionA, uint64_t, float restitutionB, uint64_t)
{
	g_materialRestitutionMixCount += 1;
	return fmaxf(restitutionA, restitutionB);
}

static int run_terrain_material_cohort(const char* name)
{
	bool heightField = strcmp(name, "height-materials") == 0;
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.hitEventThreshold = 0.1f;
	b3WorldId world = b3CreateWorld(&worldDef);
	g_materialFrictionMixCount = 0;
	g_materialRestitutionMixCount = 0;
	b3World_SetFrictionCallback(world, terrain_friction_mix);
	b3World_SetRestitutionCallback(world, terrain_restitution_mix);

	b3SurfaceMaterial materials[2] = {b3DefaultSurfaceMaterial(), b3DefaultSurfaceMaterial()};
	materials[0].friction = 0.0f;
	materials[0].userMaterialId = 101;
	materials[1].friction = 1.0f;
	materials[1].restitution = 0.35f;
	materials[1].rollingResistance = 0.2f;
	materials[1].tangentVelocity = {3.0f, 0.0f, 0.0f};
	materials[1].userMaterialId = 202;
	materials[1].customColor = 0x112233;

	b3BodyDef terrainBodyDef = b3DefaultBodyDef();
	terrainBodyDef.position = heightField ? b3Vec3{-4.0f, 0.0f, -4.0f} : b3Vec3_zero;
	b3BodyId terrainBody = b3CreateBody(world, &terrainBodyDef);
	b3ShapeDef terrainDef = b3DefaultShapeDef();
	terrainDef.materials = materials;
	terrainDef.materialCount = 2;
	terrainDef.enableHitEvents = true;
	b3MeshData* mesh = nullptr;
	b3HeightFieldData* hf = nullptr;
	b3ShapeId terrainShape = {};
	b3Vec3 probePosition = {2.0f, 1.0f, -2.0f};
	if (heightField)
	{
		hf = make_height_field(4, 0, false);
		uint8_t* cellMaterials =
			reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(hf) + hf->materialOffset);
		for (int i = 0; i < 16; ++i)
		{
			cellMaterials[i] = 1;
		}
		terrainShape = b3CreateHeightFieldShape(terrainBody, &terrainDef, hf);
		probePosition = {1.0f, 1.0f, 0.0f};
	}
	else
	{
		mesh = make_grid_mesh(1, 4.0f);
		uint8_t* triangleMaterials =
			reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(mesh) + mesh->materialOffset);
		triangleMaterials[1] = 1;
		terrainShape = b3CreateMeshShape(terrainBody, &terrainDef, mesh, b3Vec3_one);
		probePosition = {-2.0f, 1.0f, 2.0f};
	}

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = probePosition;
	bodyDef.enableSleep = false;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef probeDef = b3DefaultShapeDef();
	probeDef.baseMaterial.friction = 1.0f;
	probeDef.baseMaterial.userMaterialId = 303;
	probeDef.enableHitEvents = true;
	b3ShapeId probeShape = {};
	if (heightField)
	{
		b3BoxHull box = b3MakeBoxHull(0.2f, 0.2f, 0.2f);
		probeShape = b3CreateHullShape(body, &probeDef, &box.base);
	}
	else
	{
		b3Sphere sphere = {b3Vec3_zero, 0.2f};
		probeShape = b3CreateSphereShape(body, &probeDef, &sphere);
	}

	b3SurfaceMaterial got = b3Shape_GetMeshSurfaceMaterial(terrainShape, 1);
	b3SurfaceMaterial updated = got;
	updated.tangentVelocity = {4.0f, 0.0f, 0.0f};
	b3Shape_SetMeshMaterial(terrainShape, updated, 1);
	b3SurfaceMaterial convexMaterial = b3Shape_GetSurfaceMaterial(probeShape);
	convexMaterial.customColor = 0x445566;
	b3Shape_SetSurfaceMaterial(probeShape, convexMaterial);
	convexMaterial = b3Shape_GetSurfaceMaterial(probeShape);

	bool hitIds = false;
	for (int step = 0; step < 120; ++step)
	{
		b3World_Step(world, 1.0f / 60.0f, 4);
		b3ContactEvents events = b3World_GetContactEvents(world);
		for (int i = 0; i < events.hitCount; ++i)
		{
			const b3ContactHitEvent& hit = events.hitEvents[i];
			hitIds = hitIds || (hit.userMaterialIdA == 202 && hit.userMaterialIdB == 303);
		}
	}
	b3Vec3 velocity = b3Body_GetLinearVelocity(body);
	got = b3Shape_GetMeshSurfaceMaterial(terrainShape, 1);
	bool bad = terrainShape.index1 == 0 || b3Shape_GetMeshMaterialCount(terrainShape) != 2 ||
			   got.userMaterialId != 202 || fabsf(got.tangentVelocity.x - 4.0f) > 1e-6f ||
			   convexMaterial.customColor != 0x445566 || g_materialFrictionMixCount == 0 ||
			   g_materialRestitutionMixCount == 0 || velocity.x < 0.5f || !hitIds;
	if (bad)
	{
		fprintf(stderr,
				"%s failed count=%d id=%llu tangent=%.9g color=%x mix=(%d,%d) vx=%.9g hit=%d\n",
				name, b3Shape_GetMeshMaterialCount(terrainShape), (unsigned long long)got.userMaterialId,
				got.tangentVelocity.x, convexMaterial.customColor, g_materialFrictionMixCount,
				g_materialRestitutionMixCount, velocity.x, hitIds);
		return 90;
	}
	printf("%s vx=%.6f mix=%d/%d hit=%d\n", name, velocity.x, g_materialFrictionMixCount,
		   g_materialRestitutionMixCount, hitIds);
	b3DestroyWorld(world);
	free(mesh);
	free(hf);
	return 0;
}

static int run_terrain_ccd_cohort(const char* name)
{
	bool heightField = strncmp(name, "height-ccd-", 11) == 0;
	bool disabled = strstr(name, "disabled") != nullptr;
	bool sensor = strstr(name, "sensor") != nullptr;
	bool veto = strstr(name, "pre-solve") != nullptr;
	const char* kind = strrchr(name, '-');
	kind = kind != nullptr ? kind + 1 : "sphere";
	if (disabled || sensor || veto)
	{
		kind = "sphere";
	}

	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	worldDef.enableContinuous = !disabled;
	b3WorldId world = b3CreateWorld(&worldDef);
	b3BodyDef terrainBodyDef = b3DefaultBodyDef();
	terrainBodyDef.position = heightField ? b3Vec3{-4.0f, 0.0f, -4.0f} : b3Vec3_zero;
	b3BodyId terrainBody = b3CreateBody(world, &terrainBodyDef);
	b3ShapeDef terrainDef = b3DefaultShapeDef();
	terrainDef.isSensor = sensor;
	terrainDef.enableSensorEvents = sensor;
	terrainDef.enablePreSolveEvents = veto;
	b3MeshData* mesh = nullptr;
	b3HeightFieldData* hf = nullptr;
	b3ShapeId terrainShape = {};
	if (heightField)
	{
		hf = make_height_field(4, 0, false);
		terrainShape = b3CreateHeightFieldShape(terrainBody, &terrainDef, hf);
	}
	else
	{
		mesh = make_grid_mesh(2, 4.0f);
		terrainShape = b3CreateMeshShape(terrainBody, &terrainDef, mesh, b3Vec3_one);
	}

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = {0.0f, 2.0f, 0.0f};
	bodyDef.linearVelocity = {0.0f, -300.0f, 0.0f};
	bodyDef.angularVelocity = strstr(name, "rotating") != nullptr ? b3Vec3{0.0f, 0.0f, 120.0f} : b3Vec3_zero;
	bodyDef.allowFastRotation = strstr(name, "rotating") != nullptr;
	bodyDef.enableSleep = false;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.enableSensorEvents = sensor;
	b3ShapeId visitor = {};
	if (strcmp(kind, "capsule") == 0)
	{
		b3Capsule capsule = {{-0.35f, 0.0f, 0.0f}, {0.35f, 0.0f, 0.0f}, 0.12f};
		visitor = b3CreateCapsuleShape(body, &shapeDef, &capsule);
	}
	else if (strcmp(kind, "box") == 0 || strstr(name, "rotating") != nullptr)
	{
		b3BoxHull box = b3MakeBoxHull(0.45f, 0.08f, 0.12f);
		visitor = b3CreateHullShape(body, &shapeDef, &box.base);
	}
	else if (strcmp(kind, "hull") == 0)
	{
		visitor = attach_octahedron(body, &shapeDef);
	}
	else
	{
		b3Sphere sphere = {b3Vec3_zero, 0.12f};
		visitor = b3CreateSphereShape(body, &shapeDef, &sphere);
	}

	int vetoCount = 0;
	if (veto)
	{
		b3World_SetPreSolveCallback(world, reject_first_terrain_triangle, &vetoCount);
	}
	b3World_Step(world, 1.0f / 60.0f, 1);
	float y = b3Body_GetPosition(body).y;
	int sensorBegins = b3World_GetSensorEvents(world).beginCount;
	bool shouldTunnel = disabled || sensor;
	bool bad = shouldTunnel ? y > -1.0f : y < -0.2f;
	bad = bad || (sensor && sensorBegins == 0) || (veto && vetoCount < 2);
	if (terrainShape.index1 == 0 || visitor.index1 == 0 || bad)
	{
		fprintf(stderr, "%s failed y=%.9g sensor=%d veto=%d\n", name, y, sensorBegins, vetoCount);
		return 89;
	}
	printf("%s y=%.6f sensor=%d veto=%d\n", name, y, sensorBegins, vetoCount);
	b3DestroyWorld(world);
	free(mesh);
	free(hf);
	return 0;
}

static int run_scene(const char* name)
{
	if (strcmp(name, "mesh-materials") == 0 || strcmp(name, "height-materials") == 0)
	{
		return run_terrain_material_cohort(name);
	}
	if (strncmp(name, "mesh-ccd-", 9) == 0 || strncmp(name, "height-ccd-", 11) == 0)
	{
		return run_terrain_ccd_cohort(name);
	}
	if (strcmp(name, "ccd-spinning-stick") == 0)
	{
		return run_ccd_spinning_cohort();
	}
	if (strncmp(name, "ccd-", 4) == 0)
	{
		return run_ccd_cohort(name);
	}
	if (strncmp(name, "callback-", 9) == 0)
	{
		return run_callback_cohort(name);
	}
	if (strcmp(name, "joint-events") == 0)
	{
		return run_joint_event_cohort();
	}
	if (strcmp(name, "body-events") == 0)
	{
		return run_body_event_cohort();
	}
	if (strcmp(name, "hit-events") == 0)
	{
		return run_hit_event_cohort();
	}
	if (strncmp(name, "sensor-", 7) == 0)
	{
		return run_sensor_cohort(name);
	}
	if (strcmp(name, "filter-mask") == 0 || strcmp(name, "filter-positive-group") == 0 ||
		strcmp(name, "filter-negative-group") == 0 || strcmp(name, "filter-runtime") == 0 ||
		strcmp(name, "contact-events") == 0 || strcmp(name, "contact-events-destroy") == 0)
	{
		return run_filter_event_cohort(name);
	}

	b3WorldDef worldDef = b3DefaultWorldDef();
	if (strcmp(name, "mesh-backside") == 0)
	{
		worldDef.gravity = {0.0f, 10.0f, 0.0f};
	}
	b3WorldId world = b3CreateWorld(&worldDef);
	if (!b3World_IsValid(world))
	{
		fprintf(stderr, "b3CreateWorld failed (GPU?)\n");
		return 1;
	}

	OwnedHull ownedHull;

	b3BodyId tracked[48];
	b3MeshData* ownedMesh = nullptr;
	b3HeightFieldData* ownedHeightField = nullptr;
	b3CompoundData* ownedCompound = nullptr;
	b3JointId trackedJoint = {0};
	int n = 0;
	bool sawMeshBeginEvent = false;
	bool isPrismatic = strncmp(name, "prismatic", 9) == 0;
	bool isMotor = strncmp(name, "motor-", 6) == 0;
	bool isRevolute = strncmp(name, "revolute", 8) == 0;
	bool isSpherical = strncmp(name, "spherical", 9) == 0;
	bool isWeld = strncmp(name, "weld", 4) == 0;
	bool isWheel = strncmp(name, "wheel-", 6) == 0;

	if (strncmp(name, "height-", 7) == 0)
	{
		int mode = strcmp(name, "height-slope") == 0 ? 1 : strcmp(name, "height-wave-stress") == 0 ? 2 : 0;
		if (strcmp(name, "height-hole") == 0)
		{
			mode = 3;
		}
		int cells = strcmp(name, "height-wave-stress") == 0 ? 24 : 4;
		bool clockwise = strcmp(name, "height-clockwise-backface") == 0;
		ownedHeightField = make_height_field(cells, mode, clockwise);
		b3BodyDef heightBodyDef = b3DefaultBodyDef();
		heightBodyDef.position = {-4.0f, 0.0f, -4.0f};
		if (strcmp(name, "height-transform") == 0)
		{
			heightBodyDef.position = {-3.0f, 0.25f, -4.0f};
			heightBodyDef.rotation = b3Quat{{0.0f, 0.0f, 0.087155743f}, 0.996194698f};
		}
		b3BodyId heightBody = b3CreateBody(world, &heightBodyDef);
		b3ShapeDef heightDef = b3DefaultShapeDef();
		heightDef.filter.maskBits = strcmp(name, "height-filter") == 0 ? 0 : UINT64_MAX;
		heightDef.enableContactEvents = strcmp(name, "height-events") == 0;
		b3ShapeId heightShape = b3CreateHeightFieldShape(heightBody, &heightDef, ownedHeightField);
		if (heightShape.index1 == 0 || b3Shape_GetHeightField(heightShape) != ownedHeightField)
		{
			fprintf(stderr, "height field create/type/getter failed\n");
			return 73;
		}
		b3BodyDef invalidBodyDef = b3DefaultBodyDef();
		invalidBodyDef.type = b3_dynamicBody;
		b3BodyId invalidBody = b3CreateBody(world, &invalidBodyDef);
		if (b3CreateHeightFieldShape(invalidBody, &heightDef, ownedHeightField).index1 != 0)
		{
			fprintf(stderr, "height field accepted a dynamic body\n");
			return 74;
		}
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.enableSleep = false;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.enableContactEvents = strcmp(name, "height-events") == 0;
		int count = strcmp(name, "height-wave-stress") == 0 ? 24 :
					strcmp(name, "height-flat-convex") == 0 ? 4 :
					strcmp(name, "height-seam") == 0 ? 5 : 1;
		for (int i = 0; i < count; ++i)
		{
			float x = count == 1 ? (strcmp(name, "height-hole") == 0 ? 1.0f : 0.0f) :
									 -3.0f + 6.0f * float(i) / float(count - 1);
			float z = strcmp(name, "height-hole") == 0 ? 1.0f :
					  strcmp(name, "height-wave-stress") == 0 ? -3.0f + 0.27f * float(i) : 0.0f;
			bodyDef.position = {x, 2.0f + 0.08f * float(i), z};
			tracked[i] = b3CreateBody(world, &bodyDef);
			if (strcmp(name, "height-flat-convex") == 0 && i == 0)
			{
				b3Sphere sphere = {b3Vec3_zero, 0.3f};
				b3CreateSphereShape(tracked[i], &shapeDef, &sphere);
			}
			else if (strcmp(name, "height-flat-convex") == 0 && i == 1)
			{
				b3Capsule capsule = {{-0.3f, 0.0f, 0.0f}, {0.3f, 0.0f, 0.0f}, 0.2f};
				b3CreateCapsuleShape(tracked[i], &shapeDef, &capsule);
			}
			else if (strcmp(name, "height-flat-convex") == 0 && i == 3)
			{
				attach_octahedron(tracked[i], &shapeDef);
			}
			else
			{
				b3BoxHull box = b3MakeBoxHull(0.25f, 0.25f, 0.25f);
				b3CreateHullShape(tracked[i], &shapeDef, &box.base);
			}
		}
		n = count;
	}
	else if (strncmp(name, "mesh-", 5) == 0)
	{
		ownedMesh = make_grid_mesh(strcmp(name, "mesh-seam-grid") == 0 ? 2 : 1, 4.0f);
		b3BodyDef meshBodyDef = b3DefaultBodyDef();
		if (strcmp(name, "mesh-transform-scale") == 0)
		{
			meshBodyDef.position = {1.0f, 0.25f, 0.0f};
			meshBodyDef.rotation = b3Quat{{0.0f, 0.0f, 0.087155743f}, 0.996194698f};
		}
		b3BodyId meshBody = b3CreateBody(world, &meshBodyDef);
		b3ShapeDef meshDef = b3DefaultShapeDef();
		if (strcmp(name, "mesh-filter") == 0)
		{
			meshDef.filter.maskBits = 0;
		}
		if (strcmp(name, "mesh-events") == 0)
		{
			meshDef.enableContactEvents = true;
		}
		b3Vec3 scale = strcmp(name, "mesh-transform-scale") == 0 ? b3Vec3{-2.0f, 1.5f, 0.5f} : b3Vec3_one;
		b3CreateMeshShape(meshBody, &meshDef, ownedMesh, scale);

		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.enableContactEvents = strcmp(name, "mesh-events") == 0;
		if (strcmp(name, "mesh-backside") == 0)
		{
			bodyDef.position = {0.0f, -2.0f, 0.0f};
			tracked[0] = b3CreateBody(world, &bodyDef);
			b3Sphere sphere = {{0.0f, 0.0f, 0.0f}, 0.35f};
			b3CreateSphereShape(tracked[0], &shapeDef, &sphere);
			n = 1;
		}
		else if (strcmp(name, "mesh-convex-drop") == 0)
		{
			for (int i = 0; i < 4; ++i)
			{
				bodyDef.position = {-1.5f + float(i), 2.0f + 0.3f * float(i), 0.0f};
				tracked[i] = b3CreateBody(world, &bodyDef);
			}
			b3Sphere sphere = {{0.0f, 0.0f, 0.0f}, 0.3f};
			b3CreateSphereShape(tracked[0], &shapeDef, &sphere);
			b3Capsule capsule = {{-0.3f, 0.0f, 0.0f}, {0.3f, 0.0f, 0.0f}, 0.2f};
			b3CreateCapsuleShape(tracked[1], &shapeDef, &capsule);
			b3BoxHull box = b3MakeBoxHull(0.3f, 0.3f, 0.3f);
			b3CreateHullShape(tracked[2], &shapeDef, &box.base);
			attach_octahedron(tracked[3], &shapeDef);
			n = 4;
		}
		else
		{
			int count = strcmp(name, "mesh-seam-grid") == 0 ? 5 : 1;
			for (int i = 0; i < count; ++i)
			{
				bodyDef.position = {count == 1 ? 1.0f : -1.0f + 0.5f * float(i), 2.0f + 0.1f * float(i), 0.0f};
				tracked[i] = b3CreateBody(world, &bodyDef);
				b3BoxHull box = b3MakeBoxHull(0.25f, 0.25f, 0.25f);
				b3CreateHullShape(tracked[i], &shapeDef, &box.base);
			}
			n = count;
		}
	}
	else if (strcmp(name, "single-box") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 0.5f, 0.0f};
		tracked[n] = b3CreateBody(world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull cube = b3MakeCubeHull(0.5f);
		b3CreateHullShape(tracked[n], &shapeDef, &cube.base);
		n = 1;
	}
	else if (strcmp(name, "box-stack") == 0 || strcmp(name, "stack") == 0)
	{
		add_ground(world, 40.0f);
		float a = 0.5f;
		int count = strcmp(name, "stack") == 0 ? 6 : 40;
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3BoxHull cube = b3MakeBoxHull(a, a, a);
		for (int i = 0; i < count; ++i)
		{
			if (strcmp(name, "stack") == 0)
			{
				bodyDef.position = b3Pos{0.0f, 0.5f + (float)i * 1.05f, 0.0f};
			}
			else
			{
				bodyDef.position = b3Pos{0.0f, 1.5f * a + 2.5f * a * (float)i, 0.0f};
			}
			tracked[n] = b3CreateBody(world, &bodyDef);
			b3ShapeDef shapeDef = b3DefaultShapeDef();
			shapeDef.baseMaterial.rollingResistance = 0.1f;
			b3CreateHullShape(tracked[n], &shapeDef, &cube.base);
			n++;
		}
	}
	else if (strcmp(name, "filter") == 0)
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.gravityScale = 0.0f;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeCubeHull(0.5f);

		bodyDef.position = {-1.0f, 2.0f, 0.0f};
		bodyDef.linearVelocity = {1.0f, 0.0f, 0.0f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);

		bodyDef.position = {1.0f, 2.0f, 0.0f};
		bodyDef.linearVelocity = {-1.0f, 0.0f, 0.0f};
		tracked[1] = b3CreateBody(world, &bodyDef);
		b3CreateHullShape(tracked[1], &shapeDef, &box.base);
		n = 2;

		b3FilterJointDef jointDef = b3DefaultFilterJointDef();
		jointDef.base.bodyIdA = tracked[0];
		jointDef.base.bodyIdB = tracked[1];
		b3CreateFilterJoint(world, &jointDef);
	}
	else if (strncmp(name, "distance", 8) == 0)
	{
		b3BodyDef anchorDef = b3DefaultBodyDef();
		b3BodyId anchor = b3CreateBody(world, &anchorDef);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.gravityScale = 0.0f;
		bodyDef.enableSleep = false;
		bodyDef.position = {strcmp(name, "distance-motor") == 0 ? 1.0f : 2.0f, 2.0f, 0.0f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3Sphere sphere = {b3Vec3_zero, 0.25f};
		b3CreateSphereShape(tracked[0], &shapeDef, &sphere);
		n = 1;

		b3DistanceJointDef jointDef = b3DefaultDistanceJointDef();
		jointDef.base.bodyIdA = anchor;
		jointDef.base.bodyIdB = tracked[0];
		jointDef.base.localFrameA.p = {0.0f, 2.0f, 0.0f};
		jointDef.length = 1.0f;
		if (strcmp(name, "distance-spring") == 0)
		{
			jointDef.enableSpring = true;
			jointDef.hertz = 2.0f;
			jointDef.dampingRatio = 0.7f;
		}
		else if (strcmp(name, "distance-limit") == 0)
		{
			jointDef.enableSpring = true;
			jointDef.enableLimit = true;
			jointDef.minLength = 0.8f;
			jointDef.maxLength = 1.2f;
		}
		else if (strcmp(name, "distance-motor") == 0)
		{
			jointDef.enableSpring = true;
			jointDef.enableMotor = true;
			jointDef.maxMotorForce = 100000.0f;
			jointDef.motorSpeed = 0.5f;
		}
		b3JointId joint = b3CreateDistanceJoint(world, &jointDef);
		b3DistanceJoint_SetLength(joint, 1.0f);
		b3DistanceJoint_SetSpringForceRange(joint, -100000.0f, 100000.0f);
		b3DistanceJoint_SetSpringHertz(joint, jointDef.hertz);
		b3DistanceJoint_SetSpringDampingRatio(joint, jointDef.dampingRatio);
		b3DistanceJoint_EnableSpring(joint, jointDef.enableSpring);
		b3DistanceJoint_SetLengthRange(joint, jointDef.minLength, jointDef.maxLength);
		b3DistanceJoint_EnableLimit(joint, jointDef.enableLimit);
		b3DistanceJoint_SetMotorSpeed(joint, jointDef.motorSpeed);
		b3DistanceJoint_SetMaxMotorForce(joint, jointDef.maxMotorForce);
		b3DistanceJoint_EnableMotor(joint, jointDef.enableMotor);
	}
	else if (isMotor)
	{
		b3BodyDef anchorDef = b3DefaultBodyDef();
		b3BodyId anchor = b3CreateBody(world, &anchorDef);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.gravityScale = 0.0f;
		bodyDef.enableSleep = false;
		if (strcmp(name, "motor-spring") == 0)
		{
			bodyDef.position = {2.0f, 0.0f, 0.0f};
			bodyDef.rotation = {{0.0f, 0.0f, 0.24740396f}, 0.96891242f};
		}
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeCubeHull(0.5f);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);

		b3MotorJointDef jointDef = b3DefaultMotorJointDef();
		jointDef.base.bodyIdA = anchor;
		jointDef.base.bodyIdB = tracked[0];
		if (strcmp(name, "motor-velocity") == 0)
		{
			jointDef.linearVelocity = {1.0f, 0.0f, 0.0f};
			jointDef.maxVelocityForce = 1000000.0f;
			jointDef.angularVelocity = {0.0f, 0.0f, 1.0f};
			jointDef.maxVelocityTorque = 1000000.0f;
		}
		else
		{
			jointDef.linearHertz = 5.0f;
			jointDef.linearDampingRatio = 1.0f;
			jointDef.maxSpringForce = 1000000.0f;
			jointDef.angularHertz = 5.0f;
			jointDef.angularDampingRatio = 1.0f;
			jointDef.maxSpringTorque = 1000000.0f;
		}
		b3JointId joint = b3CreateMotorJoint(world, &jointDef);
		b3MotorJoint_SetLinearVelocity(joint, jointDef.linearVelocity);
		b3MotorJoint_SetAngularVelocity(joint, jointDef.angularVelocity);
		b3MotorJoint_SetMaxVelocityForce(joint, jointDef.maxVelocityForce);
		b3MotorJoint_SetMaxVelocityTorque(joint, jointDef.maxVelocityTorque);
		b3MotorJoint_SetLinearHertz(joint, jointDef.linearHertz);
		b3MotorJoint_SetLinearDampingRatio(joint, jointDef.linearDampingRatio);
		b3MotorJoint_SetAngularHertz(joint, jointDef.angularHertz);
		b3MotorJoint_SetAngularDampingRatio(joint, jointDef.angularDampingRatio);
		b3MotorJoint_SetMaxSpringForce(joint, jointDef.maxSpringForce);
		b3MotorJoint_SetMaxSpringTorque(joint, jointDef.maxSpringTorque);
		b3Vec3 linearVelocity = b3MotorJoint_GetLinearVelocity(joint);
		b3Vec3 angularVelocity = b3MotorJoint_GetAngularVelocity(joint);
		if (linearVelocity.x != jointDef.linearVelocity.x || angularVelocity.z != jointDef.angularVelocity.z ||
			b3MotorJoint_GetMaxVelocityForce(joint) != jointDef.maxVelocityForce ||
			b3MotorJoint_GetMaxVelocityTorque(joint) != jointDef.maxVelocityTorque ||
			b3MotorJoint_GetLinearHertz(joint) != jointDef.linearHertz ||
			b3MotorJoint_GetLinearDampingRatio(joint) != jointDef.linearDampingRatio ||
			b3MotorJoint_GetAngularHertz(joint) != jointDef.angularHertz ||
			b3MotorJoint_GetAngularDampingRatio(joint) != jointDef.angularDampingRatio ||
			b3MotorJoint_GetMaxSpringForce(joint) != jointDef.maxSpringForce ||
			b3MotorJoint_GetMaxSpringTorque(joint) != jointDef.maxSpringTorque)
		{
			fprintf(stderr, "motor controls failed round-trip\n");
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "parallel") == 0)
	{
		b3BodyDef anchorDef = b3DefaultBodyDef();
		b3BodyId anchor = b3CreateBody(world, &anchorDef);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.gravityScale = 0.0f;
		bodyDef.enableSleep = false;
		bodyDef.position = {0.0f, 2.0f, 0.0f};
		bodyDef.rotation = {{0.24740396f, 0.0f, 0.0f}, 0.96891242f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeCubeHull(0.5f);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);
		n = 1;

		b3ParallelJointDef jointDef = b3DefaultParallelJointDef();
		jointDef.base.bodyIdA = anchor;
		jointDef.base.bodyIdB = tracked[0];
		jointDef.hertz = 10.0f;
		jointDef.dampingRatio = 0.7f;
		b3JointId joint = b3CreateParallelJoint(world, &jointDef);
		b3ParallelJoint_SetSpringHertz(joint, 10.0f);
		b3ParallelJoint_SetSpringDampingRatio(joint, 0.7f);
		b3ParallelJoint_SetMaxTorque(joint, FLT_MAX);
	}
	else if (strcmp(name, "sphere-stack") == 0)
	{
		add_ground(world, 15.0f);
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
			tracked[n++] = b3CreateBody(world, &bodyDef);
			b3CreateSphereShape(tracked[n - 1], &shapeDef, &sphere);
			y += 3.0f * r;
		}
	}
	else if (strcmp(name, "capsule-stack") == 0)
	{
		add_ground(world, 40.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.motionLocks.linearZ = true;
		bodyDef.motionLocks.angularX = true;
		bodyDef.motionLocks.angularY = true;
		bodyDef.motionLocks.angularZ = true;
		float r = 0.5f;
		b3Capsule capsule = {{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, r};
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		float y = 1.5f * r;
		for (int i = 0; i < 20; ++i)
		{
			bodyDef.position.y = y;
			tracked[n++] = b3CreateBody(world, &bodyDef);
			b3CreateCapsuleShape(tracked[n - 1], &shapeDef, &capsule);
			y += 2.0f * r;
		}
	}
	else if (strcmp(name, "cylinder-stack") == 0 || strcmp(name, "cylinder-stack-plain") == 0)
	{
		add_ground(world, 10.0f);
		ownedHull.p = b3CreateCylinder(1.0f, 0.5f, 0.0f, 15);
		const b3Vec3 scales[4] = {
			b3Vec3_one,
			{-0.75f, 1.0f, 1.0f},
			{1.2f, 1.0f, -0.9f},
			{0.9f, 0.9f, 0.9f},
		};
		bool scaled = strcmp(name, "cylinder-stack-plain") != 0;
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		for (int i = 0; i < 10; ++i)
		{
			bodyDef.position = {0.0f, 1.1f * (float)i, 0.0f};
			tracked[n++] = b3CreateBody(world, &bodyDef);
			if (scaled)
			{
				b3CreateTransformedHullShape(tracked[n - 1], &shapeDef, ownedHull.p, b3Transform_identity, scales[i % 4]);
			}
			else
			{
				b3CreateHullShape(tracked[n - 1], &shapeDef, ownedHull.p);
			}
		}
	}
	else if (strcmp(name, "wheel-driving") == 0)
	{
		add_ground(world, 30.0f);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BodyDef chassisDef = b3DefaultBodyDef();
		chassisDef.type = b3_dynamicBody;
		chassisDef.enableSleep = false;
		chassisDef.position = {0.0f, 2.0f, 0.0f};
		tracked[0] = b3CreateBody(world, &chassisDef);
		b3BoxHull chassis = b3MakeBoxHull(1.5f, 0.25f, 2.5f);
		b3CreateHullShape(tracked[0], &shapeDef, &chassis.base);
		n = 1;

		const b3Vec3 offsets[4] = {
			{-1.5f, -0.25f, 1.75f}, {1.5f, -0.25f, 1.75f},
			{-1.5f, -0.25f, -1.75f}, {1.5f, -0.25f, -1.75f},
		};
		b3BoxHull tire = b3MakeBoxHull(0.3f, 0.65f, 0.65f);
		for (int i = 0; i < 4; ++i)
		{
			b3BodyDef wheelDef = b3DefaultBodyDef();
			wheelDef.type = b3_dynamicBody;
			wheelDef.enableSleep = false;
			wheelDef.position = {offsets[i].x, 1.75f, offsets[i].z};
			tracked[n] = b3CreateBody(world, &wheelDef);
			b3CreateHullShape(tracked[n], &shapeDef, &tire.base);

			b3WheelJointDef jointDef = b3DefaultWheelJointDef();
			jointDef.base.bodyIdA = tracked[0];
			jointDef.base.bodyIdB = tracked[n];
			jointDef.base.localFrameA.p = offsets[i];
			jointDef.suspensionHertz = 3.0f;
			jointDef.suspensionDampingRatio = 0.7f;
			jointDef.enableSuspensionLimit = true;
			jointDef.lowerSuspensionLimit = -0.25f;
			jointDef.upperSuspensionLimit = 0.25f;
			jointDef.enableSpinMotor = i >= 2;
			jointDef.spinSpeed = 8.0f;
			jointDef.maxSpinTorque = 50.0f;
			jointDef.enableSteering = i < 2;
			jointDef.targetSteeringAngle = 0.2f;
			jointDef.steeringHertz = 4.0f;
			jointDef.steeringDampingRatio = 0.8f;
			jointDef.maxSteeringTorque = 40.0f;
			b3JointId joint = b3CreateWheelJoint(world, &jointDef);
			if (i == 0) trackedJoint = joint;
			n++;
		}
	}
	else if (isWheel)
	{
		b3BodyDef anchorDef = b3DefaultBodyDef();
		b3BodyId anchor = b3CreateBody(world, &anchorDef);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.gravityScale = 0.0f;
		bodyDef.enableSleep = false;
		bodyDef.position = {strcmp(name, "wheel-axis") == 0 ? 0.75f : 0.0f, 2.0f, 0.0f};
		bodyDef.linearVelocity = {0.5f, 1.0f, -0.75f};
		bodyDef.angularVelocity = {1.0f, -0.5f, 2.0f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull wheel = b3MakeBoxHull(0.3f, 0.7f, 0.7f);
		b3CreateHullShape(tracked[0], &shapeDef, &wheel.base);

		b3WheelJointDef jointDef = b3DefaultWheelJointDef();
		jointDef.base.bodyIdA = anchor;
		jointDef.base.bodyIdB = tracked[0];
		jointDef.base.localFrameA.p = {0.0f, 2.0f, 0.0f};
		jointDef.base.localFrameB.p = {0.0f, 0.0f, 0.0f};
		jointDef.base.localFrameA.q = {{0.0f, 0.0f, 0.24740396f}, 0.96891242f};
		jointDef.base.localFrameB.q = jointDef.base.localFrameA.q;
		jointDef.enableSuspensionSpring = strcmp(name, "wheel-axis") != 0;
		if (strcmp(name, "wheel-spring") == 0)
		{
			jointDef.suspensionHertz = 3.0f;
			jointDef.suspensionDampingRatio = 0.7f;
		}
		else if (strcmp(name, "wheel-limits") == 0)
		{
			jointDef.enableSuspensionSpring = false;
			jointDef.enableSuspensionLimit = true;
			jointDef.lowerSuspensionLimit = -0.2f;
			jointDef.upperSuspensionLimit = 0.2f;
		}
		else if (strcmp(name, "wheel-spin") == 0)
		{
			jointDef.enableSpinMotor = true;
			jointDef.spinSpeed = 4.0f;
			jointDef.maxSpinTorque = 100.0f;
		}
		else if (strcmp(name, "wheel-steering") == 0)
		{
			jointDef.enableSteering = true;
			jointDef.steeringHertz = 4.0f;
			jointDef.steeringDampingRatio = 0.8f;
			jointDef.targetSteeringAngle = 0.35f;
			jointDef.maxSteeringTorque = 100.0f;
		}
		else if (strcmp(name, "wheel-steering-limits") == 0)
		{
			jointDef.enableSteering = true;
			jointDef.targetSteeringAngle = 1.0f;
			jointDef.maxSteeringTorque = 100.0f;
			jointDef.enableSteeringLimit = true;
			jointDef.lowerSteeringLimit = -0.25f;
			jointDef.upperSteeringLimit = 0.25f;
		}
		b3JointId joint = b3CreateWheelJoint(world, &jointDef);
		trackedJoint = joint;
		if (strcmp(name, "wheel-controls") == 0)
		{
			b3WheelJoint_EnableSuspension(joint, false);
			b3WheelJoint_SetSuspensionHertz(joint, 2.5f);
			b3WheelJoint_SetSuspensionDampingRatio(joint, 0.6f);
			b3WheelJoint_EnableSuspensionLimit(joint, true);
			b3WheelJoint_SetSuspensionLimits(joint, -0.4f, 0.5f);
			b3WheelJoint_EnableSpinMotor(joint, true);
			b3WheelJoint_SetSpinMotorSpeed(joint, 3.0f);
			b3WheelJoint_SetMaxSpinTorque(joint, 20.0f);
			b3WheelJoint_EnableSteering(joint, true);
			b3WheelJoint_SetSteeringHertz(joint, 5.0f);
			b3WheelJoint_SetSteeringDampingRatio(joint, 0.9f);
			b3WheelJoint_SetTargetSteeringAngle(joint, 0.2f);
			b3WheelJoint_SetMaxSteeringTorque(joint, 30.0f);
			b3WheelJoint_EnableSteeringLimit(joint, true);
			b3WheelJoint_SetSteeringLimits(joint, -0.3f, 0.4f);
			if (b3WheelJoint_IsSuspensionEnabled(joint) ||
				!b3WheelJoint_IsSuspensionLimitEnabled(joint) || !b3WheelJoint_IsSpinMotorEnabled(joint) ||
				!b3WheelJoint_IsSteeringEnabled(joint) || !b3WheelJoint_IsSteeringLimitEnabled(joint) ||
				fabsf(b3WheelJoint_GetSuspensionHertz(joint) - 2.5f) > 1e-6f ||
				fabsf(b3WheelJoint_GetSuspensionDampingRatio(joint) - 0.6f) > 1e-6f ||
				fabsf(b3WheelJoint_GetLowerSuspensionLimit(joint) + 0.4f) > 1e-6f ||
				fabsf(b3WheelJoint_GetUpperSuspensionLimit(joint) - 0.5f) > 1e-6f ||
				fabsf(b3WheelJoint_GetSpinMotorSpeed(joint) - 3.0f) > 1e-6f ||
				fabsf(b3WheelJoint_GetMaxSpinTorque(joint) - 20.0f) > 1e-6f ||
				fabsf(b3WheelJoint_GetSteeringHertz(joint) - 5.0f) > 1e-6f ||
				fabsf(b3WheelJoint_GetSteeringDampingRatio(joint) - 0.9f) > 1e-6f ||
				fabsf(b3WheelJoint_GetTargetSteeringAngle(joint) - 0.2f) > 1e-6f ||
				fabsf(b3WheelJoint_GetMaxSteeringTorque(joint) - 30.0f) > 1e-6f ||
				fabsf(b3WheelJoint_GetLowerSteeringLimit(joint) + 0.3f) > 1e-6f ||
				fabsf(b3WheelJoint_GetUpperSteeringLimit(joint) - 0.4f) > 1e-6f)
			{
				fprintf(stderr, "wheel controls did not round-trip\n");
				return 9;
			}
		}
	}
	else if (isRevolute || isSpherical || isWeld || isPrismatic)
	{
		add_ground(world, 20.0f);
		b3BodyDef groundDef = b3DefaultBodyDef();
		groundDef.position = b3Pos{0.0f, -1.0f, 0.0f};
		b3BodyId groundId = b3CreateBody(world, &groundDef);

		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{isPrismatic ? 0.5f : 0.0f, 4.0f, 0.0f};
		if (strcmp(name, "prismatic-spring") == 0)
		{
			bodyDef.position.x = 2.0f;
		}
		if (isWeld || isPrismatic || strcmp(name, "revolute") != 0)
		{
			bodyDef.gravityScale = 0.0f;
		}
		if (strcmp(name, "revolute-lock") == 0)
		{
			bodyDef.angularVelocity = {0.37f, 2.0f, 3.0f};
		}
		else if (strcmp(name, "revolute-spring") == 0)
		{
			bodyDef.rotation = {{0.0f, 0.0f, 0.47942555f}, 0.87758256f};
		}
		else if (strcmp(name, "revolute-limit") == 0)
		{
			bodyDef.angularVelocity = {0.0f, 0.0f, 5.0f};
		}
		else if (strcmp(name, "spherical") == 0)
		{
			bodyDef.angularVelocity = {1.0f, 2.0f, -0.5f};
			bodyDef.linearVelocity = {2.0f, -1.0f, 0.5f};
		}
		else if (strcmp(name, "spherical-spring") == 0)
		{
			bodyDef.rotation = {{0.3428978f, 0.3428978f, 0.0f}, 0.874524f};
		}
		else if (strcmp(name, "spherical-cone") == 0)
		{
			bodyDef.rotation = {{0.47942555f, 0.0f, 0.0f}, 0.87758256f};
			bodyDef.angularVelocity = {2.0f, 1.0f, 0.0f};
		}
		else if (strcmp(name, "spherical-twist") == 0)
		{
			bodyDef.rotation = {{0.17434874f, -0.09524715f, 0.46986896f}, 0.86008936f};
			bodyDef.angularVelocity = {0.0f, 0.0f, 3.0f};
		}
		else if (strcmp(name, "spherical-frames") == 0)
		{
			bodyDef.rotation = {{0.17434874f, -0.09524715f, 0.46986896f}, 0.86008936f};
			bodyDef.angularVelocity = {3.0f, 0.0f, 0.0f};
		}
		if (isPrismatic)
		{
			if (strcmp(name, "prismatic") == 0)
			{
				bodyDef.linearVelocity = {0.5f, 0.0f, 0.0f};
			}
			else if (strcmp(name, "prismatic-motor") == 0)
			{
				bodyDef.linearVelocity = b3Vec3_zero;
			}
			else if (strcmp(name, "prismatic-limit") == 0)
			{
				bodyDef.linearVelocity = {2.0f, 0.0f, 0.0f};
			}
			bodyDef.angularVelocity = {0.0f, 0.0f, 1.0f};
		}
		else if (isWeld)
		{
			bodyDef.linearVelocity = {0.75f, -0.25f, 0.5f};
			bodyDef.angularVelocity = {1.0f, -0.5f, 0.75f};
		}
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeBoxHull(0.5f, 1.5f, 0.25f);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);

		if (isRevolute)
		{
			b3RevoluteJointDef jointDef = b3DefaultRevoluteJointDef();
			jointDef.base.bodyIdA = groundId;
			jointDef.base.bodyIdB = tracked[0];
			jointDef.base.localFrameA.p = {0.0f, 6.5f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, 1.5f, 0.0f};
			if (strcmp(name, "revolute-lock") == 0)
			{
				b3Quat frame = {{0.0f, 0.70710678f, 0.0f}, 0.70710678f};
				jointDef.base.localFrameA.q = frame;
				jointDef.base.localFrameB.q = frame;
			}
			b3JointId jointId = b3CreateRevoluteJoint(world, &jointDef);
			trackedJoint = jointId;
			b3Joint_SetConstraintTuning(jointId, 45.0f, 1.5f);
			float constraintHertz = 0.0f;
			float constraintDamping = 0.0f;
			b3Joint_GetConstraintTuning(jointId, &constraintHertz, &constraintDamping);
			if (fabsf(constraintHertz - 45.0f) > 1e-6f || fabsf(constraintDamping - 1.5f) > 1e-6f)
			{
				fprintf(stderr, "revolute base tuning did not round-trip\n");
				return 9;
			}
			if (strcmp(name, "revolute-spring") == 0)
			{
				b3RevoluteJoint_SetSpringHertz(jointId, 3.0f);
				b3RevoluteJoint_SetSpringDampingRatio(jointId, 0.8f);
				b3RevoluteJoint_SetTargetAngle(jointId, 0.25f);
				b3RevoluteJoint_EnableSpring(jointId, true);
				if (!b3RevoluteJoint_IsSpringEnabled(jointId) ||
					fabsf(b3RevoluteJoint_GetSpringHertz(jointId) - 3.0f) > 1e-6f ||
					fabsf(b3RevoluteJoint_GetSpringDampingRatio(jointId) - 0.8f) > 1e-6f ||
					fabsf(b3RevoluteJoint_GetTargetAngle(jointId) - 0.25f) > 1e-6f)
				{
					fprintf(stderr, "revolute spring controls did not round-trip\n");
					return 9;
				}
			}
			else if (strcmp(name, "revolute-limit") == 0)
			{
				b3RevoluteJoint_SetLimits(jointId, -0.25f, 0.25f);
				b3RevoluteJoint_EnableLimit(jointId, true);
				if (!b3RevoluteJoint_IsLimitEnabled(jointId) ||
					fabsf(b3RevoluteJoint_GetLowerLimit(jointId) + 0.25f) > 1e-6f ||
					fabsf(b3RevoluteJoint_GetUpperLimit(jointId) - 0.25f) > 1e-6f)
				{
					fprintf(stderr, "revolute limit controls did not round-trip\n");
					return 9;
				}
			}
			else if (strcmp(name, "revolute-motor") == 0)
			{
				b3RevoluteJoint_SetMotorSpeed(jointId, 0.25f);
				b3RevoluteJoint_SetMaxMotorTorque(jointId, 100000.0f);
				b3RevoluteJoint_EnableMotor(jointId, true);
				if (!b3RevoluteJoint_IsMotorEnabled(jointId) ||
					fabsf(b3RevoluteJoint_GetMotorSpeed(jointId) - 0.25f) > 1e-6f ||
					fabsf(b3RevoluteJoint_GetMaxMotorTorque(jointId) - 100000.0f) > 1e-3f)
				{
					fprintf(stderr, "revolute motor controls did not round-trip\n");
					return 9;
				}
			}
		}
		else if (isSpherical)
		{
			b3SphericalJointDef jointDef = b3DefaultSphericalJointDef();
			jointDef.base.bodyIdA = groundId;
			jointDef.base.bodyIdB = tracked[0];
			jointDef.base.localFrameA.p = {0.0f, 6.5f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, 1.5f, 0.0f};
			if (strcmp(name, "spherical-frames") == 0)
			{
				b3Quat frame = {{0.0f, 0.70710678f, 0.0f}, 0.70710678f};
				jointDef.base.localFrameA.q = frame;
				jointDef.base.localFrameB.q = frame;
				jointDef.base.collideConnected = true;
			}
			b3JointId jointId = b3CreateSphericalJoint(world, &jointDef);
			trackedJoint = jointId;
			b3Joint_SetConstraintTuning(jointId, 45.0f, 1.5f);
			float constraintHertz = 0.0f;
			float constraintDamping = 0.0f;
			b3Joint_GetConstraintTuning(jointId, &constraintHertz, &constraintDamping);
			if (fabsf(constraintHertz - 45.0f) > 1e-6f || fabsf(constraintDamping - 1.5f) > 1e-6f)
			{
				fprintf(stderr, "spherical base tuning did not round-trip\n");
				return 9;
			}
			if (strcmp(name, "spherical-spring") == 0)
			{
				b3Quat target = {{0.14048043f, 0.14048043f, 0.0f}, 0.9800666f};
				b3SphericalJoint_SetTargetRotation(jointId, target);
				b3SphericalJoint_SetSpringHertz(jointId, 3.0f);
				b3SphericalJoint_SetSpringDampingRatio(jointId, 0.8f);
				b3SphericalJoint_EnableSpring(jointId, true);
				b3Quat actual = b3SphericalJoint_GetTargetRotation(jointId);
				if (!b3SphericalJoint_IsSpringEnabled(jointId) ||
					fabsf(b3SphericalJoint_GetSpringHertz(jointId) - 3.0f) > 1e-6f ||
					fabsf(b3SphericalJoint_GetSpringDampingRatio(jointId) - 0.8f) > 1e-6f ||
					fabsf(actual.v.x - target.v.x) > 1e-6f || fabsf(actual.v.y - target.v.y) > 1e-6f ||
					fabsf(actual.s - target.s) > 1e-6f)
				{
					fprintf(stderr, "spherical spring controls did not round-trip\n");
					return 9;
				}
			}
			else if (strcmp(name, "spherical-cone") == 0)
			{
				b3SphericalJoint_SetConeLimit(jointId, 0.25f);
				b3SphericalJoint_EnableConeLimit(jointId, true);
				if (!b3SphericalJoint_IsConeLimitEnabled(jointId) ||
					fabsf(b3SphericalJoint_GetConeLimit(jointId) - 0.25f) > 1e-6f)
				{
					fprintf(stderr, "spherical cone controls did not round-trip\n");
					return 9;
				}
			}
			else if (strcmp(name, "spherical-twist") == 0)
			{
				b3SphericalJoint_SetTwistLimits(jointId, -0.2f, 0.3f);
				b3SphericalJoint_EnableTwistLimit(jointId, true);
				if (!b3SphericalJoint_IsTwistLimitEnabled(jointId) ||
					fabsf(b3SphericalJoint_GetLowerTwistLimit(jointId) + 0.2f) > 1e-6f ||
					fabsf(b3SphericalJoint_GetUpperTwistLimit(jointId) - 0.3f) > 1e-6f)
				{
					fprintf(stderr, "spherical twist controls did not round-trip\n");
					return 9;
				}
			}
			else if (strcmp(name, "spherical-frames") == 0)
			{
				b3SphericalJoint_SetTwistLimits(jointId, -0.2f, 0.3f);
				b3SphericalJoint_EnableTwistLimit(jointId, true);
			}
			else if (strcmp(name, "spherical-motor") == 0)
			{
				b3Vec3 velocity = {0.5f, -0.75f, 1.0f};
				b3SphericalJoint_SetMotorVelocity(jointId, velocity);
				b3SphericalJoint_SetMaxMotorTorque(jointId, 100000.0f);
				b3SphericalJoint_EnableMotor(jointId, true);
				b3Vec3 actual = b3SphericalJoint_GetMotorVelocity(jointId);
				if (!b3SphericalJoint_IsMotorEnabled(jointId) ||
					fabsf(b3SphericalJoint_GetMaxMotorTorque(jointId) - 100000.0f) > 1e-3f ||
					fabsf(actual.x - velocity.x) > 1e-6f || fabsf(actual.y - velocity.y) > 1e-6f ||
					fabsf(actual.z - velocity.z) > 1e-6f)
				{
					fprintf(stderr, "spherical motor controls did not round-trip\n");
					return 9;
				}
			}
		}
		else if (isWeld)
		{
			b3WeldJointDef jointDef = b3DefaultWeldJointDef();
			jointDef.base.bodyIdA = groundId;
			jointDef.base.bodyIdB = tracked[0];
			jointDef.base.localFrameA.p = {0.0f, 6.5f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, 1.5f, 0.0f};
			jointDef.base.collideConnected = true;
			if (strcmp(name, "weld-angular-soft") == 0)
			{
				jointDef.angularHertz = 3.0f;
				jointDef.angularDampingRatio = 0.7f;
			}
			else if (strcmp(name, "weld-linear-soft") == 0)
			{
				jointDef.linearHertz = 4.0f;
				jointDef.linearDampingRatio = 0.8f;
			}
			else if (strcmp(name, "weld-frames") == 0)
			{
				b3Quat frameA = {{0.0f, 0.38268343f, 0.0f}, 0.92387953f};
				b3Quat frameB = {{0.25881905f, 0.0f, 0.0f}, 0.96592583f};
				jointDef.base.localFrameA.q = frameA;
				jointDef.base.localFrameB.q = frameB;
			}
			b3JointId jointId = b3CreateWeldJoint(world, &jointDef);
			trackedJoint = jointId;
			if (strcmp(name, "weld-controls") == 0)
			{
				b3Joint_SetConstraintTuning(jointId, 37.0f, 1.25f);
				b3WeldJoint_SetLinearHertz(jointId, 2.5f);
				b3WeldJoint_SetLinearDampingRatio(jointId, 0.65f);
				b3WeldJoint_SetAngularHertz(jointId, 3.5f);
				b3WeldJoint_SetAngularDampingRatio(jointId, 0.75f);
				float baseHertz = 0.0f;
				float baseDamping = 0.0f;
				b3Joint_GetConstraintTuning(jointId, &baseHertz, &baseDamping);
				if (fabsf(baseHertz - 37.0f) > 1e-6f || fabsf(baseDamping - 1.25f) > 1e-6f ||
					fabsf(b3WeldJoint_GetLinearHertz(jointId) - 2.5f) > 1e-6f ||
					fabsf(b3WeldJoint_GetLinearDampingRatio(jointId) - 0.65f) > 1e-6f ||
					fabsf(b3WeldJoint_GetAngularHertz(jointId) - 3.5f) > 1e-6f ||
					fabsf(b3WeldJoint_GetAngularDampingRatio(jointId) - 0.75f) > 1e-6f)
				{
					fprintf(stderr, "weld controls did not round-trip\n");
					return 9;
				}
			}
		}
		else
		{
			b3PrismaticJointDef jointDef = b3DefaultPrismaticJointDef();
			jointDef.base.bodyIdA = groundId;
			jointDef.base.bodyIdB = tracked[0];
			jointDef.base.localFrameA.p = {0.0f, 6.5f, 0.0f};
			jointDef.base.localFrameB.p = {0.0f, 1.5f, 0.0f};
			b3JointId jointId = b3CreatePrismaticJoint(world, &jointDef);
			if (strcmp(name, "prismatic-spring") == 0)
			{
				b3PrismaticJoint_SetSpringHertz(jointId, 2.0f);
				b3PrismaticJoint_SetSpringDampingRatio(jointId, 0.7f);
				b3PrismaticJoint_SetTargetTranslation(jointId, 0.0f);
				b3PrismaticJoint_EnableSpring(jointId, true);
				if (!b3PrismaticJoint_IsSpringEnabled(jointId) ||
					fabsf(b3PrismaticJoint_GetSpringHertz(jointId) - 2.0f) > 1e-6f ||
					fabsf(b3PrismaticJoint_GetSpringDampingRatio(jointId) - 0.7f) > 1e-6f)
				{
					fprintf(stderr, "prismatic spring controls did not round-trip\n");
					return 9;
				}
			}
			else if (strcmp(name, "prismatic-motor") == 0)
			{
				b3PrismaticJoint_SetMaxMotorForce(jointId, 100000.0f);
				b3PrismaticJoint_SetMotorSpeed(jointId, 1.0f);
				b3PrismaticJoint_EnableMotor(jointId, true);
				if (!b3PrismaticJoint_IsMotorEnabled(jointId) ||
					fabsf(b3PrismaticJoint_GetMotorSpeed(jointId) - 1.0f) > 1e-6f ||
					fabsf(b3PrismaticJoint_GetMaxMotorForce(jointId) - 100000.0f) > 1e-3f)
				{
					fprintf(stderr, "prismatic motor controls did not round-trip\n");
					return 9;
				}
			}
			else if (strcmp(name, "prismatic-limit") == 0)
			{
				b3PrismaticJoint_SetLimits(jointId, -1.0f, 1.0f);
				b3PrismaticJoint_EnableLimit(jointId, true);
				if (!b3PrismaticJoint_IsLimitEnabled(jointId) ||
					fabsf(b3PrismaticJoint_GetLowerLimit(jointId) + 1.0f) > 1e-6f ||
					fabsf(b3PrismaticJoint_GetUpperLimit(jointId) - 1.0f) > 1e-6f)
				{
					fprintf(stderr, "prismatic limit controls did not round-trip\n");
					return 9;
				}
			}
		}
	}
	else if (strcmp(name, "gyro-box") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 2.0f, 0.0f};
		bodyDef.gravityScale = 0.0f;
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.updateBodyMass = false;
		b3BoxHull box = b3MakeBoxHull(1.0f, 0.05f, 0.1f);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);
		b3Body_ApplyMassFromShapes(tracked[0]);
		b3Body_SetAngularVelocity(tracked[0], b3Vec3{0.01f, 0.01f, 10.0f});
	}
	else if (strcmp(name, "gyro-compound") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 2.0f, 0.0f};
		bodyDef.rotation = b3Quat{{-0.707106781f, 0.0f, 0.0f}, 0.707106781f};
		bodyDef.gravityScale = 0.0f;
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.updateBodyMass = false;
		ownedHull.p = b3CreateCylinder(0.6f, 0.15f, 0.0f, 32);
		b3BoxHull box = b3MakeBoxHull(1.0f, 0.05f, 0.1f);
		b3CreateHullShape(tracked[0], &shapeDef, ownedHull.p);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);
		b3Body_ApplyMassFromShapes(tracked[0]);
		b3Body_SetAngularVelocity(tracked[0], b3Vec3{0.01f, 0.01f, 10.0f});
	}
	else if (strcmp(name, "compound-spheres") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 3.0f, 0.0f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3Sphere left = {b3Vec3{-0.65f, 0.0f, 0.0f}, 0.5f};
		b3Sphere right = {b3Vec3{0.65f, 0.0f, 0.0f}, 0.5f};
		b3CreateSphereShape(tracked[0], &shapeDef, &left);
		b3CreateSphereShape(tracked[0], &shapeDef, &right);
	}
	else if (strcmp(name, "baked-compound") == 0)
	{
		PackedHullCompound* packed = make_packed_hull_compound();
		if (packed == nullptr)
		{
			fprintf(stderr, "baked compound alloc failed\n");
			b3DestroyWorld(world);
			return 80;
		}
		ownedCompound = &packed->header;
		b3BodyDef groundDef = b3DefaultBodyDef();
		b3BodyId groundId = b3CreateBody(world, &groundDef);
		b3ShapeDef groundShapeDef = b3DefaultShapeDef();
		b3ShapeId compoundShape = b3CreateBakedCompoundShape(groundId, &groundShapeDef, ownedCompound);
		if (compoundShape.index1 == 0 || gpu_b3_shape_kind(compoundShape) != 6)
		{
			fprintf(stderr, "baked compound create/type failed\n");
			b3DestroyWorld(world);
			free(ownedCompound);
			return 81;
		}
		if (gpu_b3_body_get_shape_count(groundId) != 1)
		{
			fprintf(stderr, "baked compound exposed hidden children\n");
			b3DestroyWorld(world);
			free(ownedCompound);
			return 83;
		}
		b3BodyDef invalidBodyDef = b3DefaultBodyDef();
		invalidBodyDef.type = b3_dynamicBody;
		b3BodyId invalidBody = b3CreateBody(world, &invalidBodyDef);
		if (b3CreateBakedCompoundShape(invalidBody, &groundShapeDef, ownedCompound).index1 != 0)
		{
			fprintf(stderr, "baked compound accepted a dynamic body\n");
			b3DestroyWorld(world);
			free(ownedCompound);
			return 82;
		}
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = {0.0f, 2.0f, 0.0f};
		bodyDef.enableSleep = false;
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3Sphere sphere = {b3Vec3_zero, 0.25f};
		b3CreateSphereShape(tracked[0], &shapeDef, &sphere);
	}
	else if (strcmp(name, "convex-octahedron") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 3.0f, 0.0f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		attach_octahedron(tracked[0], &shapeDef);
	}
	else if (strcmp(name, "convex-cylinder") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 3.0f, 0.0f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		ownedHull.p = b3CreateCylinder(0.6f, 0.15f, 0.0f, 32);
		b3CreateHullShape(tracked[0], &shapeDef, ownedHull.p);
	}
	else if (strcmp(name, "kinematic-target") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_kinematicBody;
		bodyDef.position = b3Pos{0.0f, 2.0f, 0.0f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeBoxHull(0.25f, 0.5f, 0.25f);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);
	}
	else if (strcmp(name, "off-center-impulse") == 0)
	{
		add_ground(world, 20.0f);
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Pos{0.0f, 2.0f, 0.0f};
		bodyDef.gravityScale = 0.0f;
		tracked[0] = b3CreateBody(world, &bodyDef);
		n = 1;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeBoxHull(0.2f, 0.8f, 0.05f);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);
		b3Body_ApplyLinearImpulse(tracked[0], b3Vec3{0.0f, 0.0f, 25.0f}, b3Pos{0.0f, 2.8f, 0.0f}, true);
	}
	else if (strcmp(name, "thin-box-gap") == 0)
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.gravityScale = 0.0f;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull box = b3MakeBoxHull(0.2f, 0.8f, 0.05f);

		bodyDef.position = b3Pos{0.0f, 2.0f, -0.224f};
		bodyDef.rotation = b3Quat{{0.0f, 0.017452406f, 0.0f}, 0.999847695f};
		tracked[0] = b3CreateBody(world, &bodyDef);
		b3CreateHullShape(tracked[0], &shapeDef, &box.base);

		bodyDef.position = b3Pos{0.0f, 2.0f, 0.0f};
		bodyDef.rotation = b3Quat_identity;
		tracked[1] = b3CreateBody(world, &bodyDef);
		b3CreateHullShape(tracked[1], &shapeDef, &box.base);
		n = 2;
	}
	else
	{
		fprintf(stderr, "unknown scene %s\n", name);
		b3DestroyWorld(world);
		return 2;
	}

	uint32_t step = 0;
	size_t cp = 0;
	const uint32_t last = 300;
	while (1)
	{
		if (cp < 5 && step == kCheckpoints[cp])
		{
			dump_bodies(tracked, n, step);
			cp++;
		}
		if (step == last)
		{
			break;
		}
		if (strcmp(name, "kinematic-target") == 0)
		{
			if (step == 0)
			{
				b3WorldTransform target = {{1.0f, 2.0f, 0.0f}, {{0.0f, 0.0f, 0.258819045f}, 0.965925826f}};
				b3Body_SetTargetTransform(tracked[0], target, 1.0f / 60.0f, true);
			}
			else if (step == 1)
			{
				b3Body_SetLinearVelocity(tracked[0], b3Vec3_zero);
				b3Body_SetAngularVelocity(tracked[0], b3Vec3_zero);
			}
		}
		b3World_Step(world, 1.0f / 60.0f, 4);
		if (strcmp(name, "mesh-events") == 0 || strcmp(name, "height-events") == 0)
		{
			b3ContactEvents events = b3World_GetContactEvents(world);
			sawMeshBeginEvent = sawMeshBeginEvent || events.beginCount > 0;
		}
		step++;
		if ((strcmp(name, "cylinder-stack") == 0 || strcmp(name, "cylinder-stack-plain") == 0) && step == 9)
		{
			b3Pos bottom = b3Body_GetPosition(tracked[0]);
			b3Pos second = b3Body_GetPosition(tracked[1]);
			if (second.y - bottom.y < 0.95f)
			{
				fprintf(stderr, "cylinder stack overlapped at step 9: bottom=%.9g second=%.9g\n", bottom.y, second.y);
				b3DestroyWorld(world);
				return 3;
			}
		}
	}

	if (strcmp(name, "capsule-stack") == 0)
	{
		float previousY = 0.0f;
		for (int i = 0; i < n; ++i)
		{
			b3Pos p = b3Body_GetPosition(tracked[i]);
			if (fabsf(p.x) > 1.0e-5f || fabsf(p.z) > 1.0e-5f)
			{
				fprintf(stderr, "fixed-rotation capsule walked: body=%d x=%.9g z=%.9g\n", i, p.x, p.z);
				b3DestroyWorld(world);
				return 3;
			}
			float verticalError = i == 0 ? fabsf(p.y - 0.5f) : fabsf((p.y - previousY) - 1.0f);
			if (verticalError > 0.01f)
			{
				fprintf(stderr, "fixed-rotation capsule stack unstable: body=%d y=%.9g error=%.9g\n", i, p.y,
						verticalError);
				b3DestroyWorld(world);
				return 3;
			}
			previousY = p.y;
		}
	}
	else if (strcmp(name, "baked-compound") == 0)
	{
		b3Pos p = b3Body_GetPosition(tracked[0]);
		if (p.y < 0.2f || p.y > 0.4f)
		{
			fprintf(stderr, "baked compound failed to catch the sphere: y=%.9g\n", p.y);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "cylinder-stack") == 0 || strcmp(name, "cylinder-stack-plain") == 0)
	{
		float previousY = b3Body_GetPosition(tracked[0]).y;
		for (int i = 1; i < n; ++i)
		{
			b3Pos p = b3Body_GetPosition(tracked[i]);
			b3Quat q = b3Body_GetRotation(tracked[i]);
			if (p.y - previousY < 0.5f || q.v.x * q.v.x + q.v.z * q.v.z > 0.04f)
			{
				fprintf(stderr, "cylinder stack unstable: body=%d y=%.9g previous=%.9g qx=%.9g qz=%.9g\n", i, p.y,
						previousY, q.v.x, q.v.z);
				b3DestroyWorld(world);
				return 3;
			}
			previousY = p.y;
		}
	}
	else if (strcmp(name, "revolute-lock") == 0)
	{
		b3Quat q = b3Body_GetRotation(tracked[0]);
		if (fabsf(q.v.y) > 0.02f || fabsf(q.v.z) > 0.02f)
		{
			fprintf(stderr, "revolute hinge-axis lock failed: q=(%.9g %.9g %.9g %.9g)\n", q.v.x, q.v.y, q.v.z,
					q.s);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "revolute-spring") == 0)
	{
		float angle = b3RevoluteJoint_GetAngle(trackedJoint);
		if (fabsf(angle - 0.25f) > 0.02f)
		{
			fprintf(stderr, "revolute spring failed: angle=%.9g\n", angle);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "revolute-limit") == 0)
	{
		float angle = b3RevoluteJoint_GetAngle(trackedJoint);
		if (angle < -0.27f || angle > 0.27f)
		{
			fprintf(stderr, "revolute limit failed: angle=%.9g\n", angle);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "revolute-motor") == 0)
	{
		float angle = b3RevoluteJoint_GetAngle(trackedJoint);
		float torque = b3RevoluteJoint_GetMotorTorque(trackedJoint);
		if (angle < 1.2f || !isfinite(torque) || fabsf(torque) > 100001.0f)
		{
			fprintf(stderr, "revolute motor failed: angle=%.9g torque=%.9g\n", angle, torque);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (isWheel)
	{
		b3Pos p = b3Body_GetPosition(tracked[0]);
		b3Quat q = b3Body_GetRotation(tracked[0]);
		float spinSpeed = b3WheelJoint_GetSpinSpeed(trackedJoint);
		float spinTorque = b3WheelJoint_GetSpinTorque(trackedJoint);
		float steeringAngle = b3WheelJoint_GetSteeringAngle(trackedJoint);
		float steeringTorque = b3WheelJoint_GetSteeringTorque(trackedJoint);
		if (!isfinite(p.x) || !isfinite(p.y) || !isfinite(p.z) || !isfinite(q.s) || !isfinite(spinSpeed) ||
			!isfinite(spinTorque) || !isfinite(steeringAngle) || !isfinite(steeringTorque))
		{
			fprintf(stderr, "wheel produced non-finite state\n");
			b3DestroyWorld(world);
			return 3;
		}
		if (strcmp(name, "wheel-steering-limits") == 0 && (steeringAngle < -0.28f || steeringAngle > 0.28f))
		{
			fprintf(stderr, "wheel steering limit failed: angle=%.9g\n", steeringAngle);
			b3DestroyWorld(world);
			return 3;
		}
		if (strcmp(name, "wheel-spin") == 0 && fabsf(spinTorque) > 100.1f)
		{
			fprintf(stderr, "wheel spin torque cap failed: torque=%.9g\n", spinTorque);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "spherical") == 0)
	{
		b3Pos position = b3Body_GetPosition(tracked[0]);
		b3Quat rotation = b3Body_GetRotation(tracked[0]);
		b3Vec3 offset = b3RotateVector(rotation, {0.0f, 1.5f, 0.0f});
		b3Pos anchor = {position.x + offset.x, position.y + offset.y, position.z + offset.z};
		if (fabsf(anchor.x) > 0.02f || fabsf(anchor.y - 5.5f) > 0.02f || fabsf(anchor.z) > 0.02f)
		{
			fprintf(stderr, "spherical point anchor drifted: p=(%.9g %.9g %.9g)\n", anchor.x, anchor.y, anchor.z);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "spherical-spring") == 0)
	{
		float cone = b3SphericalJoint_GetConeAngle(trackedJoint);
		float twist = b3SphericalJoint_GetTwistAngle(trackedJoint);
		if (fabsf(cone - 0.4f) > 0.03f || fabsf(twist) > 0.03f)
		{
			fprintf(stderr, "spherical spring failed: cone=%.9g twist=%.9g\n", cone, twist);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "spherical-cone") == 0)
	{
		float angle = b3SphericalJoint_GetConeAngle(trackedJoint);
		if (angle > 0.27f)
		{
			fprintf(stderr, "spherical cone limit failed: angle=%.9g\n", angle);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "spherical-twist") == 0 || strcmp(name, "spherical-frames") == 0)
	{
		float angle = b3SphericalJoint_GetTwistAngle(trackedJoint);
		if (angle < -0.22f || angle > 0.32f)
		{
			fprintf(stderr, "spherical twist limit failed: angle=%.9g\n", angle);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "spherical-motor") == 0)
	{
		b3Quat rotation = b3Body_GetRotation(tracked[0]);
		b3Vec3 torque = b3SphericalJoint_GetMotorTorque(trackedJoint);
		if (rotation.v.x * rotation.v.x + rotation.v.y * rotation.v.y + rotation.v.z * rotation.v.z < 0.01f ||
			!isfinite(torque.x) || !isfinite(torque.y) || !isfinite(torque.z) ||
			sqrtf(torque.x * torque.x + torque.y * torque.y + torque.z * torque.z) > 100001.0f)
		{
			fprintf(stderr, "spherical motor failed: q=(%.9g %.9g %.9g) torque=(%.9g %.9g %.9g)\n",
					rotation.v.x, rotation.v.y, rotation.v.z, torque.x, torque.y, torque.z);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "filter") == 0)
	{
		b3Pos a = b3Body_GetPosition(tracked[0]);
		b3Pos b = b3Body_GetPosition(tracked[1]);
		if (a.x < 3.9f || b.x > -3.9f)
		{
			fprintf(stderr, "filter joint failed to suppress collision: ax=%.9g bx=%.9g\n", a.x, b.x);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "motor-velocity") == 0)
	{
		b3Pos p = b3Body_GetPosition(tracked[0]);
		b3Quat q = b3Body_GetRotation(tracked[0]);
		if (p.x < 4.5f || q.v.z * q.v.z < 0.05f)
		{
			fprintf(stderr, "motor velocity drive failed: x=%.9g qz=%.9g\n", p.x, q.v.z);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "motor-spring") == 0)
	{
		b3Pos p = b3Body_GetPosition(tracked[0]);
		b3Quat q = b3Body_GetRotation(tracked[0]);
		if (fabsf(p.x) > 0.02f || q.v.z * q.v.z > 0.001f)
		{
			fprintf(stderr, "motor spring failed: x=%.9g qz=%.9g\n", p.x, q.v.z);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strncmp(name, "distance", 8) == 0)
	{
		b3Pos p = b3Body_GetPosition(tracked[0]);
		float distance = sqrtf(p.x * p.x + (p.y - 2.0f) * (p.y - 2.0f) + p.z * p.z);
		if (strcmp(name, "distance-motor") == 0)
		{
			if (distance < 3.4f)
			{
				fprintf(stderr, "distance motor failed: length=%.9g\n", distance);
				b3DestroyWorld(world);
				return 3;
			}
		}
		else
		{
			float minLength = strcmp(name, "distance-limit") == 0 ? 0.79f : 0.97f;
			float maxLength = strcmp(name, "distance-limit") == 0 ? 1.21f : 1.06f;
			if (distance < minLength || distance > maxLength)
			{
				fprintf(stderr, "distance constraint drifted: length=%.9g\n", distance);
				b3DestroyWorld(world);
				return 3;
			}
		}
	}
	else if (strcmp(name, "parallel") == 0)
	{
		b3Quat q = b3Body_GetRotation(tracked[0]);
		if (fabsf(q.v.x) > 0.03f || fabsf(q.v.y) > 0.03f)
		{
			fprintf(stderr, "parallel joint failed to align z axes: q=(%.9g %.9g %.9g %.9g)\n", q.v.x, q.v.y,
					q.v.z, q.s);
			b3DestroyWorld(world);
			return 3;
		}
	}
	else if (strcmp(name, "gyro-box") == 0 || strcmp(name, "gyro-compound") == 0)
	{
		float invMass = gpu_b3_body_inv_mass(tracked[0]);
		if (invMass <= 0.0f)
		{
			fprintf(stderr, "%s has no dynamic mass: %.9g\n", name, invMass);
			b3DestroyWorld(world);
			return 3;
		}
		b3Quat q = b3Body_GetRotation(tracked[0]);
		if (q.v.x * q.v.x + q.v.y * q.v.y + q.v.z * q.v.z < 0.01f)
		{
			fprintf(stderr, "%s did not rotate\n", name);
			b3DestroyWorld(world);
			return 3;
		}
		b3Pos center = b3Body_GetWorldCenter(tracked[0]);
		if (center.y < 1.9f || center.y > 2.1f)
		{
			fprintf(stderr, "%s center drifted: %.9g\n", name, center.y);
			b3DestroyWorld(world);
			return 4;
		}
	}
	else if (strcmp(name, "convex-octahedron") == 0)
	{
		b3Pos center = b3Body_GetWorldCenter(tracked[0]);
		if (center.y < 0.2f || center.y > 1.5f)
		{
			fprintf(stderr, "convex-octahedron did not settle: %.9g\n", center.y);
			b3DestroyWorld(world);
			return 5;
		}
	}
	else if (strcmp(name, "convex-cylinder") == 0)
	{
		b3Pos center = b3Body_GetWorldCenter(tracked[0]);
		if (center.y < 0.2f || center.y > 0.5f)
		{
			fprintf(stderr, "convex-cylinder did not settle: %.9g\n", center.y);
			b3DestroyWorld(world);
			return 6;
		}
	}
	else if (strcmp(name, "kinematic-target") == 0)
	{
		b3Pos position = b3Body_GetPosition(tracked[0]);
		b3Quat rotation = b3Body_GetRotation(tracked[0]);
		if (fabsf(position.x - 1.0f) > 0.01f || fabsf(position.y - 2.0f) > 0.01f ||
			rotation.v.z * rotation.v.z < 0.04f)
		{
			fprintf(stderr, "kinematic-target mismatch: p=(%.9g, %.9g) qz=%.9g\n", position.x, position.y,
					rotation.v.z);
			b3DestroyWorld(world);
			return 7;
		}
	}
	else if (strcmp(name, "off-center-impulse") == 0)
	{
		b3Quat rotation = b3Body_GetRotation(tracked[0]);
		if (rotation.v.x * rotation.v.x < 0.01f)
		{
			fprintf(stderr, "off-center-impulse did not rotate about X: qx=%.9g\n", rotation.v.x);
			b3DestroyWorld(world);
			return 8;
		}
	}
	else if (isWeld)
	{
		b3Pos position = b3Body_GetPosition(tracked[0]);
		b3Quat rotation = b3Body_GetRotation(tracked[0]);
		float q2 = rotation.v.x * rotation.v.x + rotation.v.y * rotation.v.y + rotation.v.z * rotation.v.z +
				   rotation.s * rotation.s;
		bool bounded = isfinite(position.x) && isfinite(position.y) && isfinite(position.z) && isfinite(q2) &&
					   fabsf(position.x) < 10.0f && fabsf(position.y) < 10.0f && fabsf(position.z) < 10.0f &&
					   fabsf(q2 - 1.0f) < 1e-3f;
		if (strcmp(name, "weld-frames") != 0)
		{
			float rotationError =
				rotation.v.x * rotation.v.x + rotation.v.y * rotation.v.y + rotation.v.z * rotation.v.z;
			bounded = bounded && fabsf(position.x) < 0.05f && fabsf(position.y - 4.0f) < 0.05f &&
					  fabsf(position.z) < 0.05f && rotationError < 1e-3f;
		}
		if (!bounded)
		{
			fprintf(stderr, "weld became unstable: p=(%.9g %.9g %.9g) q=(%.9g %.9g %.9g %.9g)\n", position.x,
					position.y, position.z, rotation.v.x, rotation.v.y, rotation.v.z, rotation.s);
			b3DestroyWorld(world);
			return 9;
		}
	}
	else if (isPrismatic)
	{
		b3Pos position = b3Body_GetPosition(tracked[0]);
		b3Quat rotation = b3Body_GetRotation(tracked[0]);
		float rotation_error =
			rotation.v.x * rotation.v.x + rotation.v.y * rotation.v.y + rotation.v.z * rotation.v.z;
		bool axialError = strcmp(name, "prismatic") == 0 && fabsf(position.x - 3.0f) > 0.01f;
		axialError = axialError || (strcmp(name, "prismatic-spring") == 0 && fabsf(position.x) > 0.9f);
		axialError = axialError || (strcmp(name, "prismatic-motor") == 0 && position.x < 1.8f);
		axialError = axialError || (strcmp(name, "prismatic-limit") == 0 && position.x > 1.05f);
		if (axialError || fabsf(position.y - 4.0f) > 0.01f || fabsf(position.z) > 0.01f ||
			rotation_error > 1e-3f)
		{
			fprintf(stderr, "prismatic constraint drifted: p=(%.9g, %.9g, %.9g) qv2=%.9g\n", position.x,
					position.y, position.z, rotation_error);
			b3DestroyWorld(world);
			return 9;
		}
	}
	else if (strcmp(name, "thin-box-gap") == 0)
	{
		b3Pos a = b3Body_GetPosition(tracked[0]);
		b3Pos b = b3Body_GetPosition(tracked[1]);
		if (fabsf(a.z + 0.224f) > 1e-4f || fabsf(b.z) > 1e-4f)
		{
			fprintf(stderr, "thin-box-gap created a false contact: az=%.9g bz=%.9g\n", a.z, b.z);
			b3DestroyWorld(world);
			return 10;
		}
	}

	if (strcmp(name, "mesh-events") == 0 && !sawMeshBeginEvent)
	{
		fprintf(stderr, "mesh contact begin event missing\n");
		b3DestroyWorld(world);
		free(ownedMesh);
		return 71;
	}
	if (strcmp(name, "height-events") == 0 && !sawMeshBeginEvent)
	{
		fprintf(stderr, "height field contact begin event missing\n");
		b3DestroyWorld(world);
		free(ownedHeightField);
		return 76;
	}
	if (strcmp(name, "mesh-filter") == 0 && b3Body_GetPosition(tracked[0]).y > -10.0f)
	{
		fprintf(stderr, "mesh filter did not suppress contact\n");
		b3DestroyWorld(world);
		free(ownedMesh);
		return 72;
	}
	if ((strcmp(name, "height-hole") == 0 || strcmp(name, "height-clockwise-backface") == 0 ||
		 strcmp(name, "height-filter") == 0) &&
		b3Body_GetPosition(tracked[0]).y > -10.0f)
	{
		fprintf(stderr, "%s did not pass through as expected\n", name);
		b3DestroyWorld(world);
		free(ownedHeightField);
		return 75;
	}
	b3DestroyWorld(world);
	free(ownedMesh);
	free(ownedHeightField);
	free(ownedCompound);
	return 0;
}

enum ExplosionShape
{
	explosionSphere,
	explosionCapsule,
	explosionBox,
	explosionOctahedron,
};

static b3BodyId add_explosion_body(b3WorldId world, b3Pos position, b3BodyType type, ExplosionShape kind,
								   float scale, uint64_t category)
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = type;
	bodyDef.position = position;
	bodyDef.gravityScale = 0.0f;
	bodyDef.enableSleep = false;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.explosionScale = scale;
	shapeDef.filter.categoryBits = category;
	if (kind == explosionSphere)
	{
		b3Sphere sphere = {b3Vec3_zero, 0.5f};
		b3CreateSphereShape(body, &shapeDef, &sphere);
	}
	else if (kind == explosionCapsule)
	{
		b3Capsule capsule = {{0.0f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f}, 0.25f};
		b3CreateCapsuleShape(body, &shapeDef, &capsule);
	}
	else if (kind == explosionBox)
	{
		b3BoxHull box = b3MakeBoxHull(0.5f, 0.75f, 0.25f);
		b3CreateHullShape(body, &shapeDef, &box.base);
	}
	else
	{
		attach_octahedron(body, &shapeDef);
	}
	return body;
}

static b3ExplosionDef explosion_at(b3Pos position)
{
	b3ExplosionDef def = b3DefaultExplosionDef();
	def.position = position;
	def.radius = 3.0f;
	def.falloff = 0.0f;
	def.impulsePerArea = 1000.0f;
	return def;
}

static float radial_speed(b3BodyId body, b3Vec3 direction)
{
	b3Vec3 velocity = b3Body_GetLinearVelocity(body);
	return velocity.x * direction.x + velocity.y * direction.y + velocity.z * direction.z;
}

static int run_explosion_cohort(const char* name)
{
	b3ExplosionDef defaultExplosion = b3DefaultExplosionDef();
	b3ShapeDef defaultShape = b3DefaultShapeDef();
	if (defaultExplosion.maskBits != UINT64_MAX || defaultExplosion.position.x != 0.0f ||
		defaultExplosion.position.y != 0.0f || defaultExplosion.position.z != 0.0f ||
		defaultExplosion.radius != 0.0f || defaultExplosion.falloff != 0.0f ||
		defaultExplosion.impulsePerArea != 0.0f || defaultShape.explosionScale != 1.0f)
	{
		fprintf(stderr, "explosion defaults do not match Box3D\n");
		return 40;
	}

	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3_zero;
	b3WorldId world = b3CreateWorld(&worldDef);
	b3ExplosionDef def = explosion_at(b3Pos_zero);
	int result = 0;

	if (strcmp(name, "explosion-radial") == 0)
	{
		b3BodyId bodies[4] = {
			add_explosion_body(world, {2.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 1.0f, 1),
			add_explosion_body(world, {-2.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionCapsule, 1.0f, 1),
			add_explosion_body(world, {0.0f, 2.0f, 0.0f}, b3_dynamicBody, explosionBox, 1.0f, 1),
			add_explosion_body(world, {0.0f, -2.0f, 0.0f}, b3_dynamicBody, explosionOctahedron, 1.0f, 1),
		};
		const b3Vec3 directions[4] = {{1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
									 {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};
		b3World_Explode(world, &def);
		float speeds[4];
		for (int i = 0; i < 4; ++i)
		{
			speeds[i] = radial_speed(bodies[i], directions[i]);
			if (!(speeds[i] > 0.0f && isfinite(speeds[i])))
			{
				fprintf(stderr, "explosion radial shape %d failed: %.9g\n", i, speeds[i]);
				result = 41;
			}
		}
		b3Pos before = b3Body_GetPosition(bodies[0]);
		b3World_Step(world, 1.0f / 60.0f, 4);
		b3Pos after = b3Body_GetPosition(bodies[0]);
		if (after.x <= before.x || radial_speed(bodies[0], directions[0]) <= 0.0f)
		{
			fprintf(stderr, "explosion host velocity was not uploaded before step\n");
			result = 42;
		}
		printf("explosion radial %.9g %.9g %.9g %.9g\n", speeds[0], speeds[1], speeds[2], speeds[3]);
	}
	else if (strcmp(name, "explosion-falloff") == 0)
	{
		b3BodyId nearBody =
			add_explosion_body(world, {1.5f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 1.0f, 1);
		b3BodyId farBody =
			add_explosion_body(world, {3.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 1.0f, 1);
		def.radius = 1.0f;
		def.falloff = 2.0f;
		b3World_Explode(world, &def);
		float nearSpeed = radial_speed(nearBody, {1.0f, 0.0f, 0.0f});
		float farSpeed = radial_speed(farBody, {1.0f, 0.0f, 0.0f});
		if (!(nearSpeed > 3.5f * farSpeed && farSpeed > 0.0f))
		{
			fprintf(stderr, "explosion falloff failed: near=%.9g far=%.9g\n", nearSpeed, farSpeed);
			result = 43;
		}
		printf("explosion falloff %.9g %.9g\n", nearSpeed, farSpeed);
	}
	else if (strcmp(name, "explosion-mask") == 0)
	{
		b3BodyId accepted =
			add_explosion_body(world, {2.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 1.0f, 1);
		b3BodyId rejected =
			add_explosion_body(world, {-2.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 1.0f, 2);
		def.maskBits = 1;
		b3World_Explode(world, &def);
		if (radial_speed(accepted, {1.0f, 0.0f, 0.0f}) <= 0.0f ||
			b3Length(b3Body_GetLinearVelocity(rejected)) != 0.0f)
		{
			fprintf(stderr, "explosion category mask failed\n");
			result = 44;
		}
		printf("explosion mask %.9g %.9g\n", b3Length(b3Body_GetLinearVelocity(accepted)),
			   b3Length(b3Body_GetLinearVelocity(rejected)));
	}
	else if (strcmp(name, "explosion-scale") == 0)
	{
		b3BodyId zero =
			add_explosion_body(world, {2.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 0.0f, 1);
		b3BodyId twice =
			add_explosion_body(world, {-2.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 2.0f, 1);
		b3World_Explode(world, &def);
		float zeroSpeed = b3Length(b3Body_GetLinearVelocity(zero));
		float twiceSpeed = radial_speed(twice, {-1.0f, 0.0f, 0.0f});
		if (zeroSpeed != 0.0f || twiceSpeed <= 2.9f)
		{
			fprintf(stderr, "explosion scale failed: zero=%.9g twice=%.9g\n", zeroSpeed, twiceSpeed);
			result = 45;
		}
		printf("explosion scale %.9g %.9g\n", zeroSpeed, twiceSpeed);
	}
	else if (strcmp(name, "explosion-static") == 0)
	{
		b3BodyId staticBody =
			add_explosion_body(world, {2.0f, 0.0f, 0.0f}, b3_staticBody, explosionSphere, 1.0f, 1);
		b3BodyId dynamicBody =
			add_explosion_body(world, {-2.0f, 0.0f, 0.0f}, b3_dynamicBody, explosionSphere, 1.0f, 1);
		b3World_Explode(world, &def);
		if (b3Length(b3Body_GetLinearVelocity(staticBody)) != 0.0f ||
			radial_speed(dynamicBody, {-1.0f, 0.0f, 0.0f}) <= 0.0f)
		{
			fprintf(stderr, "explosion static exclusion failed\n");
			result = 46;
		}
		printf("explosion static %.9g %.9g\n", b3Length(b3Body_GetLinearVelocity(staticBody)),
			   b3Length(b3Body_GetLinearVelocity(dynamicBody)));
	}
	else if (strcmp(name, "explosion-angular") == 0)
	{
		b3BodyId body =
			add_explosion_body(world, {2.0f, 1.0f, 0.0f}, b3_dynamicBody, explosionBox, 1.0f, 1);
		b3World_Explode(world, &def);
		b3Vec3 omega = b3Body_GetAngularVelocity(body);
		if (!(fabsf(omega.z) > 0.01f && isfinite(omega.z)))
		{
			fprintf(stderr, "explosion off-center angular response failed: %.9g\n", omega.z);
			result = 47;
		}
		printf("explosion angular %.9g %.9g %.9g\n", omega.x, omega.y, omega.z);
	}
	else if (strcmp(name, "explosion-compound") == 0)
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.gravityScale = 0.0f;
		bodyDef.enableSleep = false;
		bodyDef.position = {2.0f, 0.0f, 0.0f};
		b3BodyId compound = b3CreateBody(world, &bodyDef);
		bodyDef.position = {-2.0f, 0.0f, 0.0f};
		b3BodyId singleContribution = b3CreateBody(world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.updateBodyMass = false;
		b3Sphere upper = {{0.0f, 0.6f, 0.0f}, 0.5f};
		b3Sphere lower = {{0.0f, -0.6f, 0.0f}, 0.5f};
		b3CreateSphereShape(compound, &shapeDef, &upper);
		b3CreateSphereShape(compound, &shapeDef, &lower);
		b3CreateSphereShape(singleContribution, &shapeDef, &upper);
		shapeDef.explosionScale = 0.0f;
		b3CreateSphereShape(singleContribution, &shapeDef, &lower);
		b3Body_ApplyMassFromShapes(compound);
		b3Body_ApplyMassFromShapes(singleContribution);
		b3World_Explode(world, &def);
		float combined = radial_speed(compound, {1.0f, 0.0f, 0.0f});
		float single = radial_speed(singleContribution, {-1.0f, 0.0f, 0.0f});
		if (!(combined > 1.8f * single && single > 0.0f))
		{
			fprintf(stderr, "explosion compound accumulation failed: combined=%.9g single=%.9g\n", combined,
					single);
			result = 48;
		}
		printf("explosion compound %.9g %.9g\n", combined, single);
	}

	b3DestroyWorld(world);
	return result;
}

struct QueryCounts
{
	int count;
	int stopAfter;
	float clip;
};

static bool CountOverlap(b3ShapeId, void* context)
{
	QueryCounts* counts = static_cast<QueryCounts*>(context);
	counts->count += 1;
	return counts->count < counts->stopAfter;
}

static float ClipCast(b3ShapeId, b3Pos, b3Vec3, float fraction, uint64_t, int, int, void* context)
{
	QueryCounts* counts = static_cast<QueryCounts*>(context);
	counts->count += 1;
	return counts->clip >= 0.0f ? counts->clip : fraction;
}

struct MoverContext
{
	int count;
	int stopAfter;
	bool accept;
	b3ShapeId shapeId;
	b3PlaneResult planes[8];
	int planeCount;
};

static bool FilterMover(b3ShapeId shapeId, void* context)
{
	MoverContext* mover = static_cast<MoverContext*>(context);
	mover->count += 1;
	mover->shapeId = shapeId;
	return mover->accept;
}

static bool GatherMoverPlanes(b3ShapeId shapeId, const b3PlaneResult* planes, int count, void* context)
{
	MoverContext* mover = static_cast<MoverContext*>(context);
	mover->shapeId = shapeId;
	for (int i = 0; i < count && mover->planeCount < 8; ++i)
	{
		mover->planes[mover->planeCount++] = planes[i];
	}
	mover->count += 1;
	return mover->count < mover->stopAfter;
}

static int run_mover_query_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	b3WorldId world = b3CreateWorld(&worldDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.filter.categoryBits = 0x4;
	shapeDef.filter.maskBits = 0x2;
	b3BodyDef bodyDef = b3DefaultBodyDef();

	bodyDef.position = {0.0f, 5.0f, 0.0f};
	b3BodyId castBody = b3CreateBody(world, &bodyDef);
	b3Sphere castSphere = {b3Vec3_zero, 0.5f};
	b3ShapeId castShape = b3CreateSphereShape(castBody, &shapeDef, &castSphere);

	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.categoryBits = 0x2;
	filter.maskBits = 0x4;
	b3Capsule castMover = {b3Vec3_zero, b3Vec3_zero, 0.5f};
	float hit = b3World_CastMover(world, {-3.0f, 5.0f, 0.0f}, &castMover, {6.0f, 0.0f, 0.0f}, filter,
								  nullptr, nullptr);
	float miss = b3World_CastMover(world, {-3.0f, 8.0f, 0.0f}, &castMover, {6.0f, 0.0f, 0.0f}, filter,
								   nullptr, nullptr);
	float overlap = b3World_CastMover(world, {0.0f, 5.0f, 0.0f}, &castMover, {1.0f, 0.0f, 0.0f}, filter,
									  nullptr, nullptr);
	MoverContext veto = {0, 8, false};
	float vetoed = b3World_CastMover(world, {-3.0f, 5.0f, 0.0f}, &castMover, {6.0f, 0.0f, 0.0f}, filter,
									 FilterMover, &veto);
	b3QueryFilter masked = filter;
	masked.maskBits = 0x8;
	float maskMiss = b3World_CastMover(world, {-3.0f, 5.0f, 0.0f}, &castMover, {6.0f, 0.0f, 0.0f}, masked,
									   nullptr, nullptr);
	if (!(hit > 0.3f && hit < 0.36f) || miss != 1.0f || overlap != 1.0f || vetoed != 1.0f ||
		maskMiss != 1.0f || veto.count != 1 || veto.shapeId.index1 != castShape.index1)
	{
		fprintf(stderr, "mover cast failed: %.9g %.9g %.9g %.9g %.9g veto=%d shape=%d\n", hit, miss, overlap,
				vetoed, maskMiss, veto.count, veto.shapeId.index1);
		return 60;
	}

	bodyDef.position = {0.0f, -0.5f, 0.0f};
	b3BodyId floorBody = b3CreateBody(world, &bodyDef);
	b3BoxHull floorHull = b3MakeBoxHull(4.0f, 0.5f, 4.0f);
	b3CreateHullShape(floorBody, &shapeDef, &floorHull.base);
	bodyDef.position = {-0.5f, 0.0f, 0.0f};
	b3BodyId wallBody = b3CreateBody(world, &bodyDef);
	b3BoxHull wallHull = b3MakeBoxHull(0.5f, 4.0f, 4.0f);
	b3CreateHullShape(wallBody, &shapeDef, &wallHull.base);

	b3Capsule collideMover = {b3Vec3_zero, b3Vec3_zero, 0.3f};
	MoverContext gathered = {0, 8, true};
	b3World_CollideMover(world, {0.1f, 0.1f, 0.0f}, &collideMover, filter, GatherMoverPlanes, &gathered);
	bool hasFloor = false;
	bool hasWall = false;
	for (int i = 0; i < gathered.planeCount; ++i)
	{
		hasFloor = hasFloor || gathered.planes[i].plane.normal.y > 0.99f;
		hasWall = hasWall || gathered.planes[i].plane.normal.x > 0.99f;
	}
	MoverContext stopped = {0, 1, true};
	b3World_CollideMover(world, {0.1f, 0.1f, 0.0f}, &collideMover, filter, GatherMoverPlanes, &stopped);
	if (!hasFloor || !hasWall || gathered.planeCount < 2 || stopped.count != 1)
	{
		fprintf(stderr, "mover planes failed: count=%d floor=%d wall=%d stopped=%d\n", gathered.planeCount,
				hasFloor, hasWall, stopped.count);
		return 61;
	}

	bodyDef.position = {10.0f, 0.0f, 0.0f};
	bodyDef.rotation = {{0.0f, 0.0f, 0.382683432f}, 0.923879533f};
	b3BodyId compoundBody = b3CreateBody(world, &bodyDef);
	b3Sphere left = {{-0.4f, 0.6f, 0.0f}, 0.5f};
	b3Sphere right = {{0.4f, 0.6f, 0.0f}, 0.5f};
	b3CreateSphereShape(compoundBody, &shapeDef, &left);
	b3CreateSphereShape(compoundBody, &shapeDef, &right);
	b3WorldTransform compoundTransform = {bodyDef.position, bodyDef.rotation};
	b3Capsule bodyMover = {{-0.70710677f, -0.70710677f, 0.0f}, {0.70710677f, 0.70710677f, 0.0f}, 0.2f};
	b3BodyPlaneResult bodyPlanes[4] = {};
	int capped = b3Body_CollideMover(compoundBody, bodyPlanes, 1, bodyDef.position, &bodyMover, filter,
									 compoundTransform);
	int full = b3Body_CollideMover(compoundBody, bodyPlanes, 4, bodyDef.position, &bodyMover, filter,
								   compoundTransform);
	if (capped != 1 || full != 2 || bodyPlanes[0].shapeId.index1 == 0 || bodyPlanes[1].shapeId.index1 == 0)
	{
		fprintf(stderr, "body mover capacity failed: %d %d\n", capped, full);
		return 62;
	}

	b3Capsule deepMover = {{-0.2f, -0.5f, 0.0f}, {0.2f, -0.5f, 0.0f}, 0.1f};
	b3BodyPlaneResult deepPlane = {};
	b3WorldTransform floorTransform = {{0.0f, -0.5f, 0.0f}, b3Quat_identity};
	int deep = b3Body_CollideMover(floorBody, &deepPlane, 1, b3Pos_zero, &deepMover, filter,
								   floorTransform);
	// Native Box3D omits planes when the capsule axis intersects a hull.
	if (deep != 0)
	{
		fprintf(stderr, "deep hull mover failed: %d %.9g %.9g\n", deep,
				b3Length(deepPlane.result.plane.normal), deepPlane.result.plane.offset);
		return 63;
	}

	b3CollisionPlane solverPlanes[2] = {};
	solverPlanes[0].plane = {{1.0f, 0.0f, 0.0f}, 0.2f};
	solverPlanes[0].pushLimit = FLT_MAX;
	solverPlanes[0].clipVelocity = true;
	solverPlanes[1].plane = {{0.0f, 1.0f, 0.0f}, 0.3f};
	solverPlanes[1].pushLimit = 0.1f;
	solverPlanes[1].clipVelocity = true;
	b3PlaneSolverResult solved = b3SolvePlanes(b3Vec3_zero, solverPlanes, 2);
	b3Vec3 clipped = b3ClipVector({-2.0f, -3.0f, 1.0f}, solverPlanes, 2);
	if (fabsf(solved.delta.x - 0.195f) > 1e-5f || fabsf(solved.delta.y - 0.1f) > 1e-5f ||
		fabsf(clipped.x) > 1e-6f || fabsf(clipped.y) > 1e-6f || fabsf(clipped.z - 1.0f) > 1e-6f)
	{
		fprintf(stderr, "plane solver failed: delta=(%.9g %.9g) clip=(%.9g %.9g %.9g)\n", solved.delta.x,
				solved.delta.y, clipped.x, clipped.y, clipped.z);
		return 64;
	}

	printf("mover queries %.6f %d %d %.6f %.6f\n", hit, gathered.planeCount, full, solved.delta.x,
		   solved.delta.y);
	b3DestroyWorld(world);
	return 0;
}

static int run_query_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	b3WorldId world = b3CreateWorld(&worldDef);
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.position = {2.0f, 0.0f, 0.0f};
	b3BodyId body = b3CreateBody(world, &bodyDef);
    if (!b3Body_IsContactRecyclingEnabled(body)) { return 73; }
    b3Body_EnableContactRecycling(body, false);
    if (b3Body_IsContactRecyclingEnabled(body)) { return 74; }
    b3Body_EnableContactRecycling(body, true);
    if (!b3Body_IsContactRecyclingEnabled(body)) { return 75; }


	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.filter.categoryBits = 0x4;
	shapeDef.filter.maskBits = 0x2;
	shapeDef.userData = reinterpret_cast<void*>(uintptr_t(0x1234));
	b3Sphere sphere = {{-0.5f, 0.0f, 0.0f}, 0.5f};
	b3ShapeId sphereId = b3CreateSphereShape(body, &shapeDef, &sphere);
	b3Capsule capsule = {{0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}, 0.25f};
	b3ShapeId capsuleId = b3CreateCapsuleShape(body, &shapeDef, &capsule);
	bodyDef.position = {2.0f, 3.0f, 0.0f};
	bodyDef.rotation = {{0.0f, 0.0f, 0.382683432f}, 0.923879533f};
	b3BodyId boxBody = b3CreateBody(world, &bodyDef);
	b3BoxHull box = b3MakeBoxHull(0.5f, 0.75f, 0.5f);
	b3ShapeId boxId = b3CreateHullShape(boxBody, &shapeDef, &box.base);
	bodyDef.position = {2.0f, 6.0f, 0.0f};
	bodyDef.rotation = b3Quat_identity;
	b3BodyId hullBody = b3CreateBody(world, &bodyDef);
	b3ShapeId hullId = attach_octahedron(hullBody, &shapeDef);
	const b3HullData* gotHull = b3Shape_GetHull(hullId);
	bool sawOctahedronTip = false;
	if (gotHull != NULL && gotHull->vertexCount == 6)
	{
		const b3Vec3* points = (const b3Vec3*)((const char*)gotHull + gotHull->pointOffset);
		for (int i = 0; i < 6; ++i)
		{
			sawOctahedronTip = sawOctahedronTip || points[i].x == 0.5f;
		}
	}
	if (gotHull == NULL || gotHull->vertexCount != 6 || !sawOctahedronTip)
	{
		fprintf(stderr, "hull getter failed\n");
		return 19;
	}

	b3Sphere gotSphere = b3Shape_GetSphere(sphereId);
	b3Capsule gotCapsule = b3Shape_GetCapsule(capsuleId);
	b3AABB bodyBox = b3Body_ComputeAABB(body);
	if (fabsf(gotSphere.center.x + 0.5f) > 1e-6f || fabsf(gotCapsule.center2.y - 0.5f) > 1e-6f ||
		b3Shape_GetUserData(sphereId) != reinterpret_cast<void*>(uintptr_t(0x1234)) ||
		bodyBox.lowerBound.x > 1.01f || bodyBox.upperBound.x < 2.74f)
	{
		fprintf(stderr, "query getters/body AABB failed: sphere=%.9g capsule=%.9g user=%p box=(%.9g %.9g)\n",
				gotSphere.center.x, gotCapsule.center2.y, b3Shape_GetUserData(sphereId), bodyBox.lowerBound.x,
				bodyBox.upperBound.x);
		return 20;
	}

	b3QueryFilter filter = b3DefaultQueryFilter();
	if (filter.categoryBits != UINT64_MAX || filter.maskBits != UINT64_MAX)
	{
		fprintf(stderr, "query default filter failed\n");
		return 21;
	}
	filter.categoryBits = 0x2;
	filter.maskBits = 0x4;

	b3RayResult ray = b3World_CastRayClosest(world, {-2.0f, 0.0f, 0.0f}, {8.0f, 0.0f, 0.0f}, filter);
	if (!ray.hit || ray.shapeId.index1 != sphereId.index1 || ray.fraction <= 0.3f || ray.fraction >= 0.5f)
	{
		fprintf(stderr, "closest sphere ray failed: hit=%d fraction=%.9g\n", ray.hit, ray.fraction);
		return 22;
	}
	b3WorldCastOutput localRay = b3Shape_RayCast(capsuleId, {0.0f, 0.0f, 0.0f}, {4.0f, 0.0f, 0.0f});
	if (!localRay.hit || localRay.fraction <= 0.5f || localRay.fraction >= 0.7f)
	{
		fprintf(stderr, "transformed capsule ray failed\n");
		return 23;
	}
	b3RayResult boxRay = b3World_CastRayClosest(world, {-2.0f, 3.0f, 0.0f}, {8.0f, 0.0f, 0.0f}, filter);
	b3RayResult hullRay = b3World_CastRayClosest(world, {-2.0f, 6.0f, 0.0f}, {8.0f, 0.0f, 0.0f}, filter);
	if (!boxRay.hit || boxRay.shapeId.index1 != boxId.index1 || !hullRay.hit ||
		hullRay.shapeId.index1 != hullId.index1)
	{
		fprintf(stderr, "box/hull ray failed\n");
		return 28;
	}

	QueryCounts overlap = {0, 1, -1.0f};
	b3AABB queryBox = {{0.0f, -2.0f, -2.0f}, {4.0f, 2.0f, 2.0f}};
	b3TreeStats overlapStats = b3World_OverlapAABB(world, queryBox, filter, CountOverlap, &overlap);
	if (overlap.count != 1 || overlapStats.nodeVisits < 1 || overlapStats.leafVisits != 1)
	{
		fprintf(stderr, "AABB overlap early stop failed\n");
		return 24;
	}
	b3QueryFilter rejectedFilter = filter;
	rejectedFilter.maskBits = 0x8;
	QueryCounts rejected = {0, 8, -1.0f};
	b3World_OverlapAABB(world, queryBox, rejectedFilter, CountOverlap, &rejected);
	if (rejected.count != 0)
	{
		fprintf(stderr, "query mask filter failed\n");
		return 31;
	}

	b3Vec3 queryPoint = {1.5f, 0.0f, 0.0f};
	b3ShapeProxy proxy = {&queryPoint, 1, 0.2f};
	QueryCounts shapeOverlap = {0, 8, -1.0f};
	b3World_OverlapShape(world, b3Pos_zero, &proxy, filter, CountOverlap, &shapeOverlap);
	if (shapeOverlap.count != 1)
	{
		fprintf(stderr, "shape overlap failed: %d\n", shapeOverlap.count);
		return 25;
	}
	const b3Vec3 overlapCenters[] = {{1.5f, 0.0f, 0.0f}, {2.5f, 0.0f, 0.0f}, {2.0f, 3.0f, 0.0f},
									 {2.0f, 6.0f, 0.0f}};
	for (const b3Vec3& center : overlapCenters)
	{
		b3Vec3 local = center;
		b3ShapeProxy overlapProxy = {&local, 1, 0.1f};
		QueryCounts count = {0, 8, -1.0f};
		b3World_OverlapShape(world, b3Pos_zero, &overlapProxy, filter, CountOverlap, &count);
		if (count.count == 0)
		{
			fprintf(stderr, "convex overlap kind failed at y=%.9g\n", center.y);
			return 29;
		}
	}

	b3Vec3 castPoint = {-2.0f, 0.0f, 0.0f};
	b3ShapeProxy castProxy = {&castPoint, 1, 0.25f};
	QueryCounts clipped = {0, 8, 0.0f};
	b3World_CastShape(world, b3Pos_zero, &castProxy, {8.0f, 0.0f, 0.0f}, filter, ClipCast, &clipped);
	if (clipped.count != 1)
	{
		fprintf(stderr, "shape cast clipping failed\n");
		return 26;
	}
	b3Vec3 initialPoint = {1.5f, 0.0f, 0.0f};
	b3ShapeProxy initialProxy = {&initialPoint, 1, 0.1f};
	QueryCounts initial = {0, 8, -1.0f};
	b3World_CastShape(world, b3Pos_zero, &initialProxy, {1.0f, 0.0f, 0.0f}, filter, ClipCast, &initial);
	if (initial.count == 0)
	{
		fprintf(stderr, "initial-overlap shape cast failed\n");
		return 30;
	}

	b3WorldTransform bodyTransform = {b3Body_GetPosition(body), b3Body_GetRotation(body)};
	b3BodyCastResult bodyRay =
		b3Body_CastRay(body, {-2.0f, 0.0f, 0.0f}, {8.0f, 0.0f, 0.0f}, filter, 1.0f, bodyTransform);
	if (!bodyRay.hit || bodyRay.shapeId.index1 != sphereId.index1)
	{
		fprintf(stderr, "multi-shape body ray failed\n");
		return 27;
	}

	bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = {10.0f, 5.0f, 0.0f};
	b3BodyId movingBody = b3CreateBody(world, &bodyDef);
	b3Sphere movingSphere = {b3Vec3_zero, 0.25f};
	b3ShapeId movingShape = b3CreateSphereShape(movingBody, &shapeDef, &movingSphere);
	b3World_Step(world, 1.0f / 60.0f, 4);
	b3Pos moved = b3Body_GetPosition(movingBody);
	b3AABB movedBox = b3Shape_GetAABB(movingShape);
	float aabbCenterY = 0.5f * (movedBox.lowerBound.y + movedBox.upperBound.y);
	if (moved.y >= 5.0f || fabsf(aabbCenterY - moved.y) > 1e-5f)
	{
		fprintf(stderr, "post-step synchronized AABB failed: body=%.9g aabb=%.9g\n", moved.y, aabbCenterY);
		return 32;
	}

	printf("query hits %d %d %.6f\n", overlap.count, shapeOverlap.count, ray.fraction);
	b3DestroyWorld(world);
	return 0;
}

static int run_mesh_query_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	b3WorldId world = b3CreateWorld(&worldDef);
	b3MeshData* mesh = make_grid_mesh(2, 4.0f);
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.position = {1.0f, 0.25f, 0.0f};
	bodyDef.rotation = b3Quat{{0.0f, 0.0f, 0.087155743f}, 0.996194698f};
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3ShapeId shape = b3CreateMeshShape(body, &shapeDef, mesh, {-2.0f, 1.5f, 0.5f});
	b3Mesh got = b3Shape_GetMesh(shape);
	b3MeshData* replacement = make_grid_mesh(1, 3.0f);
	b3Shape_SetMesh(shape, replacement, b3Vec3_one);
	b3Mesh gotReplacement = b3Shape_GetMesh(shape);
	b3Vec3 point = {1.0f, 0.35f, 0.0f};
	b3ShapeProxy proxy = {&point, 1, 0.25f};
	QueryCounts overlap = {0, 8, -1.0f};
	b3World_OverlapShape(world, b3Pos_zero, &proxy, b3DefaultQueryFilter(), CountOverlap, &overlap);
	b3Capsule mover = {b3Vec3_zero, b3Vec3_zero, 0.3f};
	MoverContext planes = {0, 4, true};
	b3World_CollideMover(world, {1.0f, 0.35f, 0.0f}, &mover, b3DefaultQueryFilter(), GatherMoverPlanes,
						 &planes);
	b3Vec3 closest = b3Shape_GetClosestPoint(shape, {1.0f, 2.0f, 0.0f});
	b3QueryFilter qf = b3DefaultQueryFilter();
	b3RayResult meshRay = b3World_CastRayClosest(world, {1.0f, 5.0f, 0.0f}, {0.0f, -10.0f, 0.0f}, qf);
	if (!meshRay.hit || meshRay.shapeId.index1 != shape.index1 || meshRay.triangleIndex < 0)
	{
		fprintf(stderr, "mesh closest ray failed hit=%d tri=%d\n", meshRay.hit, meshRay.triangleIndex);
		return 71;
	}
	if (got.data != mesh || got.scale.x != -2.0f || gotReplacement.data != replacement || overlap.count != 1 ||
		planes.planeCount == 0 || fabsf(closest.y - 0.25f) > 0.8f)
	{
		fprintf(stderr, "mesh queries failed getter=%d overlap=%d planes=%d closest=%.9g\n", got.data == mesh,
				overlap.count, planes.planeCount, closest.y);
		return 70;
	}
	printf("mesh queries overlap=%d planes=%d\n", overlap.count, planes.planeCount);
	b3DestroyWorld(world);
	free(mesh);
	free(replacement);
	return 0;
}

static int run_height_field_query_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	b3WorldId world = b3CreateWorld(&worldDef);
	b3HeightFieldData* flat = make_height_field(4, 0, false);
	b3HeightFieldData* slope = make_height_field(4, 1, false);
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.position = {-4.0f, 0.25f, -4.0f};
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3ShapeId shape = b3CreateHeightFieldShape(body, &shapeDef, flat);
	b3Shape_SetHeightField(shape, slope);
	b3WorldCastOutput shapeRay = b3Shape_RayCast(shape, {0.0f, 5.0f, 0.0f}, {0.0f, -10.0f, 0.0f});
	b3Vec3 point = {0.0f, 0.4f, 0.0f};
	b3ShapeProxy proxy = {&point, 1, 0.3f};
	QueryCounts overlap = {0, 8, -1.0f};
	b3World_OverlapShape(world, b3Pos_zero, &proxy, b3DefaultQueryFilter(), CountOverlap, &overlap);
	QueryCounts cast = {0, 8, -1.0f};
	b3Vec3 castPoint = {0.0f, 4.0f, 0.0f};
	b3ShapeProxy castProxy = {&castPoint, 1, 0.2f};
	b3World_CastShape(world, b3Pos_zero, &castProxy, {0.0f, -8.0f, 0.0f}, b3DefaultQueryFilter(),
					  ClipCast, &cast);
	b3Capsule mover = {b3Vec3_zero, b3Vec3_zero, 0.35f};
	MoverContext planes = {0, 4, true};
	b3World_CollideMover(world, {0.0f, 0.4f, 0.0f}, &mover, b3DefaultQueryFilter(), GatherMoverPlanes,
						 &planes);
	b3Vec3 closest = b3Shape_GetClosestPoint(shape, {0.0f, 3.0f, 0.0f});
	b3AABB bounds = b3Shape_GetAABB(shape);
	bool ok = b3Shape_GetHeightField(shape) == slope && shapeRay.hit && shapeRay.materialIndex == 0 &&
			  overlap.count == 1 && cast.count > 0 && planes.planeCount > 0 && isfinite(closest.y) &&
			  bounds.upperBound.x > bounds.lowerBound.x && bounds.upperBound.z > bounds.lowerBound.z;
	b3RayResult closestWorld = b3World_CastRayClosest(world, {-3.0f, 5.0f, -3.0f}, {0.0f, -10.0f, 0.0f},
													  b3DefaultQueryFilter());
	ok = ok && closestWorld.hit && closestWorld.shapeId.index1 == shape.index1;
	if (!ok)
	{
		fprintf(stderr, "height queries failed getter=%d shapeRay=%d material=%d overlap=%d cast=%d planes=%d\n",
				b3Shape_GetHeightField(shape) == slope, shapeRay.hit, shapeRay.materialIndex, overlap.count,
				cast.count, planes.planeCount);
		return 77;
	}
	printf("height queries %.6f %d %d %d\n", shapeRay.fraction, overlap.count, cast.count, planes.planeCount);
	b3DestroyWorld(world);
	free(flat);
	free(slope);
	return 0;
}

static int run_body_control_cohort()
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	b3WorldId world = b3CreateWorld(&worldDef);
	b3World_SetGravity(world, {0.0f, -20.0f, 0.0f});
	b3Vec3 gravity = b3World_GetGravity(world);
	if (fabsf(gravity.x) > 1e-6f || fabsf(gravity.y + 20.0f) > 1e-6f || fabsf(gravity.z) > 1e-6f)
	{
		fprintf(stderr, "set/get gravity failed: (%.9g %.9g %.9g)\n", gravity.x, gravity.y, gravity.z);
		return 90;
	}
	b3World_SetGravity(world, b3Vec3_zero);

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = {0.0f, 2.0f, 0.0f};
	bodyDef.enableSleep = false;
	b3BodyId body = b3CreateBody(world, &bodyDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3Sphere sphere = {b3Vec3_zero, 0.5f};
	b3ShapeId shape = b3CreateSphereShape(body, &shapeDef, &sphere);
	b3MassData massData = b3Body_GetMassData(body);
	float expectedMass = (4.0f / 3.0f) * 3.14159265359f * 0.125f * 1000.0f;
	if (fabsf(b3Body_GetMass(body) - expectedMass) > 0.01f || fabsf(massData.mass - expectedMass) > 0.01f)
	{
		fprintf(stderr, "mass mismatch: %.9g %.9g expected %.9g\n", b3Body_GetMass(body), massData.mass,
				expectedMass);
		return 91;
	}
	b3Pos worldPoint = b3Body_GetWorldPoint(body, {0.0f, 0.5f, 0.0f});
	b3Vec3 localPoint = b3Body_GetLocalPoint(body, worldPoint);
	if (fabsf(worldPoint.x) > 1e-5f || fabsf(worldPoint.y - 2.5f) > 1e-4f || fabsf(localPoint.y - 0.5f) > 1e-4f)
	{
		fprintf(stderr, "point transform failed: w=(%.9g %.9g) l=(%.9g %.9g %.9g)\n", worldPoint.x, worldPoint.y,
				localPoint.x, localPoint.y, localPoint.z);
		return 92;
	}

	b3Body_ApplyForceToCenter(body, {0.0f, expectedMass, 0.0f}, true);
	b3World_Step(world, 1.0f / 60.0f, 4);
	b3Vec3 velocity = b3Body_GetLinearVelocity(body);
	if (fabsf(velocity.y - 1.0f / 60.0f) > 2e-4f)
	{
		fprintf(stderr, "apply force failed: vy=%.9g\n", velocity.y);
		return 93;
	}

	b3Body_SetLinearVelocity(body, b3Vec3_zero);
	b3Body_Disable(body);
	if (b3Body_IsEnabled(body))
	{
		fprintf(stderr, "disable failed\n");
		return 94;
	}
	b3World_SetGravity(world, {0.0f, -10.0f, 0.0f});
	for (int i = 0; i < 10; ++i)
	{
		b3World_Step(world, 1.0f / 60.0f, 4);
	}
	b3Pos disabledPos = b3Body_GetPosition(body);
	if (fabsf(disabledPos.y - 2.0f) > 0.01f)
	{
		fprintf(stderr, "disabled body moved: y=%.9g\n", disabledPos.y);
		return 95;
	}
	b3Body_Enable(body);
	if (!b3Body_IsEnabled(body) || !b3Body_IsAwake(body))
	{
		fprintf(stderr, "enable/awake failed\n");
		return 96;
	}

	b3MotionLocks locks = {true, false, false, false, false, false};
	b3Body_SetMotionLocks(body, locks);
	b3MotionLocks got = b3Body_GetMotionLocks(body);
	if (!got.linearX || got.linearY)
	{
		fprintf(stderr, "motion lock round-trip failed\n");
		return 97;
	}
	b3Body_SetLinearVelocity(body, {1.0f, 0.0f, 0.0f});
	b3World_SetGravity(world, b3Vec3_zero);
	b3World_Step(world, 1.0f / 60.0f, 4);
	b3Vec3 lockedVel = b3Body_GetLinearVelocity(body);
	b3Pos lockedPos = b3Body_GetPosition(body);
	if (fabsf(lockedVel.x) > 1e-5f || fabsf(lockedPos.x) > 1e-4f)
	{
		fprintf(stderr, "linear lock failed: vx=%.9g x=%.9g\n", lockedVel.x, lockedPos.x);
		return 98;
	}

	b3Body_SetMotionLocks(body, (b3MotionLocks){0});
	b3Shape_SetFriction(shape, 0.25f);
	b3Shape_SetRestitution(shape, 0.4f);
	if (fabsf(b3Shape_GetFriction(shape) - 0.25f) > 1e-6f || fabsf(b3Shape_GetRestitution(shape) - 0.4f) > 1e-6f)
	{
		fprintf(stderr, "shape material setters failed\n");
		return 99;
	}
	b3Body_SetLinearVelocity(body, b3Vec3_zero);
	b3Shape_ApplyWind(shape, {20.0f, 0.0f, 0.0f}, 1.0f, 0.0f, 50.0f, true);
	b3World_Step(world, 1.0f / 60.0f, 4);
	b3Vec3 windVel = b3Body_GetLinearVelocity(body);
	if (windVel.x <= 0.0f)
	{
		fprintf(stderr, "apply wind failed: vx=%.9g\n", windVel.x);
		return 100;
	}

	b3BodyDef groundDef = b3DefaultBodyDef();
	b3BodyId ground = b3CreateBody(world, &groundDef);
	b3RevoluteJointDef jointDef = b3DefaultRevoluteJointDef();
	jointDef.base.bodyIdA = ground;
	jointDef.base.bodyIdB = body;
	b3JointId joint = b3CreateRevoluteJoint(world, &jointDef);
	b3JointId joints[4] = {};
	if (b3Body_GetJointCount(body) != 1 || b3Body_GetJoints(body, joints, 4) != 1 ||
		joints[0].index1 != joint.index1)
	{
		fprintf(stderr, "get joints failed: count=%d stored=%d\n", b3Body_GetJointCount(body),
				b3Body_GetJoints(body, joints, 4));
		return 101;
	}

	printf("body-controls mass=%.6f wind=%.6f\n", massData.mass, windVel.x);
	b3DestroyWorld(world);
	return 0;
}

int main(int argc, char** argv)
{
	const char* scene = argc > 1 ? argv[1] : "single-box";
	printf("scene %s\n", scene);
	if (strcmp(scene, "queries") == 0)
	{
		return run_query_cohort();
	}
	if (strcmp(scene, "mover-queries") == 0)
	{
		return run_mover_query_cohort();
	}
	if (strcmp(scene, "mesh-queries") == 0)
	{
		return run_mesh_query_cohort();
	}
	if (strcmp(scene, "height-queries") == 0)
	{
		return run_height_field_query_cohort();
	}
	if (strcmp(scene, "body-controls") == 0)
	{
		return run_body_control_cohort();
	}
	if (strncmp(scene, "explosion-", 10) == 0)
	{
		return run_explosion_cohort(scene);
	}
	return run_scene(scene);
}
