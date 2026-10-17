// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#include "shape.h"

#include "body.h"
#include "broad_phase.h"
#include "contact.h"
#include "physics_world.h"
#include "recording.h"
#include "sensor.h"

// needed for dll export
#include "aabb.h"
#include "compound.h"

#include "box3d/box3d.h"

static b3Shape* b3GetShape( b3World* world, b3ShapeId shapeId )
{
	int id = shapeId.index1 - 1;
	b3Shape* shape = b3Array_Get( world->shapes, id );
	B3_ASSERT( shape->id == id && shape->generation == shapeId.generation );
	return shape;
}

static float b3ComputeShapeMargin( b3Shape* shape )
{
	float margin = 0.0f;

	switch ( shape->type )
	{
		case b3_sphereShape:
		{
			margin = shape->sphere.radius;
		}
		break;

		case b3_capsuleShape:
		{
			margin = 0.5f * b3Distance( shape->capsule.center2, shape->capsule.center1 ) + shape->capsule.radius;
		}
		break;

		case b3_hullShape:
		{
			const b3HullData* hull = shape->hull;
			const b3Vec3* points = b3GetHullPoints( hull );
			float maxExtentSqr = 0.0f;
			int count = hull->vertexCount;
			for ( int i = 0; i < count; ++i )
			{
				float distSqr = b3DistanceSquared( points[i], hull->center );
				maxExtentSqr = b3MaxFloat( maxExtentSqr, distSqr );
			}
			margin = sqrtf( maxExtentSqr );
		}
		break;

		case b3_meshShape:
		case b3_heightShape:
		case b3_compoundShape:
		{
			// Static-only shapes: broadphase uses speculative distance for static
			// proxies, so the per-shape margin is never consumed in practice.
			// Return the cap so any incidental use is generous.
			return B3_MAX_AABB_MARGIN;
		}

		default:
			B3_VALIDATE( false );
			return B3_MAX_AABB_MARGIN;
	}

	return b3MinFloat( B3_MAX_AABB_MARGIN, B3_AABB_MARGIN_FRACTION * margin );
}

static void b3UpdateShapeAABBs( b3Shape* shape, b3WorldTransform transform, b3BodyType proxyType )
{
	// Compute a bounding box with a speculative margin
	const float speculativeDistance = B3_SPECULATIVE_DISTANCE;
	const float aabbMargin = shape->aabbMargin;

	b3AABB aabb = b3ComputeFatShapeAABB( shape, transform, speculativeDistance );
	shape->aabb = aabb;

	// Smaller margin for static bodies. Cannot be zero due to TOI tolerance.
	float margin = proxyType == b3_staticBody ? speculativeDistance : aabbMargin;
	b3AABB fatAABB;
	fatAABB.lowerBound.x = aabb.lowerBound.x - margin;
	fatAABB.lowerBound.y = aabb.lowerBound.y - margin;
	fatAABB.lowerBound.z = aabb.lowerBound.z - margin;
	fatAABB.upperBound.x = aabb.upperBound.x + margin;
	fatAABB.upperBound.y = aabb.upperBound.y + margin;
	fatAABB.upperBound.z = aabb.upperBound.z + margin;
	shape->fatAABB = fatAABB;
}

static b3Shape* b3CreateShapeInternal( b3World* world, b3Body* body, b3WorldTransform bodyTransform, const b3ShapeDef* def,
									   const void* geometry, b3ShapeType shapeType, b3Transform shapeTransform, b3Vec3 scale,
									   bool haveShapeTransform )
{
	int shapeId = b3AllocId( &world->shapeIdPool );

	if ( shapeId == world->shapes.count )
	{
		b3Array_Push( world->shapes, (b3Shape){ 0 } );
	}
	else
	{
		B3_ASSERT( world->shapes.data[shapeId].id == B3_NULL_INDEX );
	}

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );

	switch ( shapeType )
	{
		case b3_capsuleShape:
			shape->capsule = *(b3Capsule*)geometry;
			break;

		case b3_compoundShape:
			// Compounds must be a static and not a sensor
			B3_ASSERT( body->type == b3_staticBody );
			B3_ASSERT( def->isSensor == false );
			shape->compound = (b3CompoundData*)geometry;
			break;

		case b3_sphereShape:
			shape->sphere = *(b3Sphere*)geometry;
			break;

		case b3_hullShape:
			if ( haveShapeTransform )
			{
				// The transform and non-uniform scale are baked into fresh data, then shared.
				b3HullData* baked = b3CloneAndTransformHull( (b3HullData*)geometry, shapeTransform, scale );
				if ( baked == NULL )
				{
					// This can fail to produce a valid hull in extreme cases
					b3FreeId( &world->shapeIdPool, shapeId );
					shape->id = B3_NULL_INDEX;
					return NULL;
				}

				shape->hull = b3AddOwnedHullToDatabase( world, baked );
			}
			else
			{
				shape->hull = b3AddHullToDatabase( world, (const b3HullData*)geometry );
			}
			break;

		case b3_meshShape:
		{
			shape->mesh.data = (b3MeshData*)geometry;
			shape->mesh.scale = b3SafeScale( scale );
		}
		break;

		case b3_heightShape:
			shape->heightField = (b3HeightFieldData*)geometry;
			break;

		default:
			B3_ASSERT( false );
			break;
	}

	shape->id = shapeId;
	shape->bodyId = body->id;
	shape->type = shapeType;
	shape->density = def->density;
	shape->explosionScale = def->explosionScale;
	shape->filter = def->filter;
	shape->userData = def->userData;
	shape->userShape = NULL;
	shape->flags = 0;
	shape->flags |= def->enableSensorEvents ? b3_enableSensorEvents : 0;
	shape->flags |= def->enableContactEvents ? b3_enableContactEvents : 0;
	shape->flags |= def->enableCustomFiltering ? b3_enableCustomFiltering : 0;
	shape->flags |= def->enableHitEvents ? b3_enableHitEvents : 0;
	shape->flags |= def->enablePreSolveEvents ? b3_enablePreSolveEvents : 0;
	shape->flags |= def->enableSpeculativeContact ? b3_enableSpeculative : 0;
	shape->proxyKey = B3_NULL_INDEX;
	shape->localCentroid = b3GetShapeCentroid( shape );
	shape->aabbMargin = b3ComputeShapeMargin( shape );
	shape->aabb = (b3AABB){ b3Vec3_zero, b3Vec3_zero };
	shape->fatAABB = (b3AABB){ b3Vec3_zero, b3Vec3_zero };
	shape->nameId = b3AddName( &world->names, def->name );
	shape->generation += 1;

	if ( shape->type == b3_compoundShape )
	{
		// Own a copy of the compound materials so every shape frees its array the same way. Compounds
		// are few, so the copy is cheap and avoids aliasing the geometry blob.
		int materialCount = shape->compound->materialCount;
		shape->materialCount = materialCount;
		shape->materials = b3Alloc( materialCount * sizeof( b3SurfaceMaterial ) );
		memcpy( shape->materials, b3GetCompoundMaterials( shape->compound ), materialCount * sizeof( b3SurfaceMaterial ) );
	}
	else if ( def->materialCount > 1 && def->materials != NULL )
	{
		// Per triangle materials need a heap array.
		shape->materialCount = def->materialCount;
		shape->materials = b3Alloc( def->materialCount * sizeof( b3SurfaceMaterial ) );
		memcpy( shape->materials, def->materials, def->materialCount * sizeof( b3SurfaceMaterial ) );
	}
	else
	{
		// The common case is one material, stored inline with no allocation.
		shape->material = ( def->materialCount == 1 && def->materials != NULL ) ? def->materials[0] : def->baseMaterial;
		shape->materialCount = 1;
		shape->materials = NULL;
	}

	if ( body->setIndex != b3_disabledSet )
	{
		b3BodyType proxyType = body->type;
		bool forcePairCreation = def->invokeContactCreation && shape->type != b3_compoundShape;
		b3CreateShapeProxy( shape, &world->broadPhase, proxyType, bodyTransform, forcePairCreation );
	}

	// Add to shape doubly linked list
	if ( body->headShapeId != B3_NULL_INDEX )
	{
		b3Shape* headShape = b3Array_Get( world->shapes, body->headShapeId );
		headShape->prevShapeId = shapeId;
	}

	shape->prevShapeId = B3_NULL_INDEX;
	shape->nextShapeId = body->headShapeId;
	body->headShapeId = shapeId;
	body->shapeCount += 1;

	if ( def->isSensor )
	{
		shape->sensorIndex = world->sensors.count;
		b3Sensor* sensor = b3Array_Emplace( world->sensors );
		b3Array_CreateN( sensor->hits, 4 );
		b3Array_CreateN( sensor->overlaps1, 16 );
		b3Array_CreateN( sensor->overlaps2, 16 );
		sensor->shapeId = shapeId;
	}
	else
	{
		shape->sensorIndex = B3_NULL_INDEX;
	}

	b3ValidateSolverSets( world );

	return shape;
}

static b3ShapeId b3CreateShape( b3BodyId bodyId, const b3ShapeDef* def, const void* geometry, b3ShapeType shapeType,
								b3Transform transform, b3Vec3 scale, bool haveTransform )
{
	B3_CHECK_DEF( def );
	B3_ASSERT( b3IsValidFloat( def->density ) && def->density >= 0.0f );
	B3_ASSERT( b3IsValidFloat( def->baseMaterial.friction ) && def->baseMaterial.friction >= 0.0f );
	B3_ASSERT( b3IsValidFloat( def->baseMaterial.restitution ) && def->baseMaterial.restitution >= 0.0f );

	b3World* world = b3GetUnlockedWorld( bodyId.world0 );
	if ( world == NULL )
	{
		return (b3ShapeId){ 0 };
	}

	if ( world->shapes.count == B3_MAX_SHAPES && world->shapeIdPool.freeArray.count == 0 )
	{
		B3_ASSERT( false );
		return b3_nullShapeId;
	}

	b3Body* body = b3GetBodyFullId( world, bodyId );
	if ( body->type != b3_staticBody && ( shapeType == b3_compoundShape || shapeType == b3_heightShape ) )
	{
		// Compound and height shapes must be on static bodies.
		return b3_nullShapeId;
	}

	world->locked = true;

	b3WorldTransform bodyTransform = b3GetBodyTransformQuick( world, body );

	b3Shape* shape =
		b3CreateShapeInternal( world, body, bodyTransform, def, geometry, shapeType, transform, scale, haveTransform );

	if ( shape == NULL )
	{
		world->locked = false;
		return b3_nullShapeId;
	}

	if ( def->updateBodyMass == true )
	{
		b3UpdateBodyMassData( world, body );
	}
	else if ( ( body->flags & b3_dirtyMass ) == 0 )
	{
		body->flags |= b3_dirtyMass;
		b3SyncBodyFlags( world, body );
	}

	b3ValidateSolverSets( world );

	b3ShapeId id = { shape->id + 1, bodyId.world0, shape->generation };

	world->locked = false;

	return id;
}






































































































































































// Destroy a shape on a body. This doesn't need to be called when destroying a body.
static void b3DestroyShapeInternal( b3World* world, b3Shape* shape, b3Body* body, bool wakeBodies )
{
	int shapeId = shape->id;

	// Remove the shape from the body's doubly linked list.
	if ( shape->prevShapeId != B3_NULL_INDEX )
	{
		b3Shape* prevShape = b3Array_Get( world->shapes, shape->prevShapeId );
		prevShape->nextShapeId = shape->nextShapeId;
	}

	if ( shape->nextShapeId != B3_NULL_INDEX )
	{
		b3Shape* nextShape = b3Array_Get( world->shapes, shape->nextShapeId );
		nextShape->prevShapeId = shape->prevShapeId;
	}

	if ( shapeId == body->headShapeId )
	{
		body->headShapeId = shape->nextShapeId;
	}

	body->shapeCount -= 1;

	// Remove from broad-phase.
	b3DestroyShapeProxy( shape, &world->broadPhase );

	// Destroy any contacts associated with the shape.
	int contactKey = body->headContactKey;
	while ( contactKey != B3_NULL_INDEX )
	{
		int contactId = contactKey >> 1;
		int edgeIndex = contactKey & 1;

		b3Contact* contact = b3Array_Get( world->contacts, contactId );
		contactKey = contact->edges[edgeIndex].nextKey;

		if ( contact->shapeIdA == shapeId || contact->shapeIdB == shapeId )
		{
			b3DestroyContact( world, contact, wakeBodies );
		}
	}

	if ( shape->sensorIndex != B3_NULL_INDEX )
	{
		b3Sensor* sensor = b3Array_Get( world->sensors, shape->sensorIndex );
		for ( int i = 0; i < sensor->overlaps2.count; ++i )
		{
			b3Visitor* ref = sensor->overlaps2.data + i;
			b3SensorEndTouchEvent event = {
				.sensorShapeId =
					{
						.index1 = shapeId + 1,
						.world0 = world->worldId,
						.generation = shape->generation,
					},
				.visitorShapeId =
					{
						.index1 = ref->shapeId + 1,
						.world0 = world->worldId,
						.generation = ref->generation,
					},
			};

			b3Array_Push( world->sensorEndEvents[world->endEventArrayIndex], event );
		}

		// Destroy sensor
		b3Array_Destroy( sensor->hits );
		b3Array_Destroy( sensor->overlaps1 );
		b3Array_Destroy( sensor->overlaps2 );

		int movedIndex = b3Array_RemoveSwap( world->sensors, shape->sensorIndex );
		if ( movedIndex != B3_NULL_INDEX )
		{
			// Fixup moved sensor
			b3Sensor* movedSensor = b3Array_Get( world->sensors, shape->sensorIndex );
			b3Shape* otherSensorShape = b3Array_Get( world->shapes, movedSensor->shapeId );
			otherSensorShape->sensorIndex = shape->sensorIndex;
		}
	}

	// Destroy every shape member from b3Alloc
	b3DestroyShapeAllocations( world, shape );

	// Return shape to free list.
	b3FreeId( &world->shapeIdPool, shapeId );
	shape->id = B3_NULL_INDEX;

	b3ValidateSolverSets( world );
}





























b3AABB b3ComputeShapeAABB( const b3Shape* shape, b3Transform transform )
{
	switch ( shape->type )
	{
		case b3_capsuleShape:
			return b3ComputeCapsuleAABB( &shape->capsule, transform );

		case b3_compoundShape:
			return b3ComputeCompoundAABB( shape->compound, transform );

		case b3_heightShape:
			return b3ComputeHeightFieldAABB( shape->heightField, transform );

		case b3_hullShape:
			return b3ComputeHullAABB( shape->hull, transform );

		case b3_meshShape:
			return b3ComputeMeshAABB( shape->mesh.data, transform, shape->mesh.scale );

		case b3_sphereShape:
			return b3ComputeSphereAABB( &shape->sphere, transform );

		default:
		{
			B3_ASSERT( false );
			b3AABB empty = { transform.p, transform.p };
			return empty;
		}
	}
}

b3AABB b3ComputeFatShapeAABB( const b3Shape* shape, b3WorldTransform transform, float extra )
{
	b3Vec3 r = { extra, extra, extra };
#if defined( BOX3D_DOUBLE_PRECISION )
	// Build the box in the body local frame, inflate, then translate by the double origin and
	// round outward. Inflating before the single rounding matters far from the origin where the
	// float margin would otherwise vanish.
	b3Transform rotation = { b3Vec3_zero, transform.q };
	b3AABB localBox = b3ComputeShapeAABB( shape, rotation );
	localBox.lowerBound = b3Sub( localBox.lowerBound, r );
	localBox.upperBound = b3Add( localBox.upperBound, r );
	return b3OffsetAABB( localBox, transform.p );
#else
	b3AABB aabb = b3ComputeShapeAABB( shape, transform );
	aabb.lowerBound = b3Sub( aabb.lowerBound, r );
	aabb.upperBound = b3Add( aabb.upperBound, r );
	return aabb;
#endif
}

b3AABB b3ComputeSweptShapeAABB( const b3Shape* shape, const b3Sweep* sweep, float time )
{
	B3_ASSERT( 0.0f <= time && time <= 1.0f );
	b3Transform xf1 = { b3Sub( sweep->c1, b3RotateVector( sweep->q1, sweep->localCenter ) ), sweep->q1 };
	b3Transform xf2 = b3GetSweepTransform( sweep, time );

	switch ( shape->type )
	{
		case b3_capsuleShape:
			return b3ComputeSweptCapsuleAABB( &shape->capsule, xf1, xf2 );

		case b3_hullShape:
			return b3ComputeSweptHullAABB( shape->hull, xf1, xf2 );

		case b3_sphereShape:
			return b3ComputeSweptSphereAABB( &shape->sphere, xf1, xf2 );

		default:
			B3_ASSERT( false );
			return (b3AABB){ xf1.p, xf1.p };
	}
}

b3Vec3 b3GetShapeCentroid( const b3Shape* shape )
{
	switch ( shape->type )
	{
		case b3_capsuleShape:
			return b3Lerp( shape->capsule.center1, shape->capsule.center2, 0.5f );
		case b3_compoundShape:
		{
			b3AABB aabb = b3ComputeCompoundAABB( shape->compound, b3Transform_identity );
			return b3AABB_Center( aabb );
		}
		case b3_sphereShape:
			return shape->sphere.center;
		case b3_hullShape:
			return shape->hull->center;
		case b3_meshShape:
		{
			b3AABB aabb = b3ComputeMeshAABB( shape->mesh.data, b3Transform_identity, shape->mesh.scale );
			return b3AABB_Center( aabb );
		}
		case b3_heightShape:
		{
			b3AABB aabb = b3ComputeHeightFieldAABB( shape->heightField, b3Transform_identity );
			return b3AABB_Center( aabb );
		}
		default:
			return b3Vec3_zero;
	}
}

float b3GetShapeArea( const b3Shape* shape )
{
	// todo_erin fix these
	switch ( shape->type )
	{
		case b3_capsuleShape:
			return 2.0f * b3Length( b3Sub( shape->capsule.center1, shape->capsule.center2 ) ) +
				   2.0f * B3_PI * shape->capsule.radius;

		case b3_hullShape:
			return shape->hull->surfaceArea;

		case b3_sphereShape:
			return 2.0f * B3_PI * shape->sphere.radius;

		default:
			return 0.0f;
	}
}

// This projects the shape surface area onto a plane
float b3GetShapeProjectedArea( const b3Shape* shape, b3Vec3 planeNormal )
{
	switch ( shape->type )
	{
		case b3_capsuleShape:
		{
			float radius = shape->capsule.radius;
			b3Vec3 axis = b3Sub( shape->capsule.center2, shape->capsule.center1 );
			float projectedLength = b3Length( b3Cross( axis, planeNormal ) );
			float cylinderArea = 2.0f * radius * projectedLength;
			float sphereArea = B3_PI * radius * radius;
			return sphereArea + cylinderArea;
		}

		case b3_hullShape:
			return b3ComputeHullProjectedArea( shape->hull, planeNormal );

		case b3_sphereShape:
			return B3_PI * shape->sphere.radius * shape->sphere.radius;

		default:
			return 0.0f;
	}
}

b3MassData b3ComputeShapeMass( const b3Shape* shape )
{
	switch ( shape->type )
	{
		case b3_capsuleShape:
			return b3ComputeCapsuleMass( &shape->capsule, shape->density );

		case b3_hullShape:
			return b3ComputeHullMass( shape->hull, shape->density );

		case b3_sphereShape:
			return b3ComputeSphereMass( &shape->sphere, shape->density );

		default:
			return (b3MassData){ 0 };
	}
}

b3ShapeExtent b3ComputeShapeExtent( const b3Shape* shape, b3Vec3 localCenter )
{
	b3ShapeExtent extent = { 0 };

	switch ( shape->type )
	{
		case b3_capsuleShape:
		{
			float radius = shape->capsule.radius;
			extent.minExtent = radius;
			b3Vec3 c1 = b3Sub( shape->capsule.center1, localCenter );
			b3Vec3 c2 = b3Sub( shape->capsule.center2, localCenter );
			b3Vec3 r = { radius, radius, radius };
			extent.maxExtent = b3Add( b3Max( c1, c2 ), r );
		}
		break;

		case b3_compoundShape:
		{
			// This is shouldn't be needed but here for completeness
			b3AABB aabb = b3ComputeCompoundAABB( shape->compound, b3Transform_identity );
			float r1 = b3Length( b3Sub( aabb.lowerBound, localCenter ) );
			float r2 = b3Length( b3Sub( aabb.upperBound, localCenter ) );
			extent.minExtent = b3MinFloat( r1, r2 );
			b3Vec3 p = b3FarthestPointOnAABB( aabb, localCenter );
			extent.maxExtent = b3Abs( b3Sub( p, localCenter ) );
		}
		break;

		case b3_sphereShape:
		{
			float radius = shape->sphere.radius;
			extent.minExtent = radius;
			b3Vec3 r = { radius, radius, radius };
			b3Vec3 p = b3Add( b3Sub( shape->sphere.center, localCenter ), r );
			extent.maxExtent = b3Abs( b3Sub( p, localCenter ) );
		}
		break;

		case b3_hullShape:
			extent = b3ComputeHullExtent( shape->hull, localCenter );
			break;

		case b3_meshShape:
		{
			// This is needed for kinematic mesh sleeping
			b3AABB aabb = b3ComputeMeshAABB( shape->mesh.data, b3Transform_identity, shape->mesh.scale );
			float r1 = b3Length( b3Sub( aabb.lowerBound, localCenter ) );
			float r2 = b3Length( b3Sub( aabb.upperBound, localCenter ) );
			extent.minExtent = b3MinFloat( r1, r2 );
			b3Vec3 p = b3FarthestPointOnAABB( aabb, localCenter );
			extent.maxExtent = b3Abs( p );
		}
		break;

		default:
			break;
	}

	return extent;
}

b3CastOutput b3RayCastShape( const b3Shape* shape, b3Transform transform, const b3RayCastInput* input )
{
	b3RayCastInput localInput = *input;
	localInput.origin = b3InvTransformPoint( transform, input->origin );
	localInput.translation = b3InvRotateVector( transform.q, input->translation );

	b3CastOutput output = { 0 };
	switch ( shape->type )
	{
		case b3_capsuleShape:
			output = b3RayCastCapsule( &shape->capsule, &localInput );
			break;
		case b3_compoundShape:
			output = b3RayCastCompound( shape->compound, &localInput );
			break;
		case b3_sphereShape:
			output = b3RayCastSphere( &shape->sphere, &localInput );
			break;
		case b3_hullShape:
			output = b3RayCastHull( shape->hull, &localInput );
			break;
		case b3_meshShape:
			output = b3RayCastMesh( &shape->mesh, &localInput );
			break;
		case b3_heightShape:
			output = b3RayCastHeightField( shape->heightField, &localInput );
			break;
		default:
			return output;
	}

	output.point = b3TransformPoint( transform, output.point );
	output.normal = b3RotateVector( transform.q, output.normal );
	return output;
}

b3CastOutput b3ShapeCastShape( const b3Shape* shape, b3Transform transform, const b3ShapeCastInput* input )
{
	b3ShapeCastInput localInput = *input;
	b3Vec3 localPoints[B3_MAX_SHAPE_CAST_POINTS];

	localInput.proxy.count = b3MinInt( input->proxy.count, B3_MAX_SHAPE_CAST_POINTS );
	for ( int i = 0; i < localInput.proxy.count; ++i )
	{
		localPoints[i] = b3InvTransformPoint( transform, input->proxy.points[i] );
	}

	localInput.proxy.points = localPoints;
	localInput.translation = b3InvRotateVector( transform.q, input->translation );

	b3CastOutput output = { 0 };
	switch ( shape->type )
	{
		case b3_capsuleShape:
			output = b3ShapeCastCapsule( &shape->capsule, &localInput );
			break;

		case b3_compoundShape:
			output = b3ShapeCastCompound( shape->compound, &localInput );
			break;

		case b3_heightShape:
			output = b3ShapeCastHeightField( shape->heightField, &localInput );
			break;

		case b3_hullShape:
			output = b3ShapeCastHull( shape->hull, &localInput );
			break;

		case b3_meshShape:
			output = b3ShapeCastMesh( &shape->mesh, &localInput );
			break;

		case b3_sphereShape:
			output = b3ShapeCastSphere( &shape->sphere, &localInput );
			break;
		default:
			return output;
	}

	output.point = b3TransformPoint( transform, output.point );
	output.normal = b3RotateVector( transform.q, output.normal );
	return output;
}

bool b3OverlapShape( const b3Shape* shape, b3Transform transform, const b3ShapeProxy* proxy )
{
	b3ShapeType type = shape->type;
	switch ( type )
	{
		case b3_capsuleShape:
			return b3OverlapCapsule( &shape->capsule, transform, proxy );

		case b3_compoundShape:
			return b3OverlapCompound( shape->compound, transform, proxy );

		case b3_heightShape:
			return b3OverlapHeightField( shape->heightField, transform, proxy );

		case b3_hullShape:
			return b3OverlapHull( shape->hull, transform, proxy );

		case b3_meshShape:
			return b3OverlapMesh( &shape->mesh, transform, proxy );

		case b3_sphereShape:
			return b3OverlapSphere( &shape->sphere, transform, proxy );

		default:
			B3_ASSERT( false );
			return false;
	}

#if 0
	b3Vec3 localPoints[B3_MAX_SHAPE_CAST_POINTS];
	b3ShapeProxy localProxy;

	b3Transform invTransform = b3InvertTransform( transform );
	b3Matrix3 R = b3MakeMatrixFromQuat( invTransform.q );

	localProxy.count = b3MinInt( proxy->count, B3_MAX_SHAPE_CAST_POINTS );
	for ( int i = 0; i < localProxy.count; ++i )
	{
		localPoints[i] = b3Add( b3MulMV( R, proxy->points[i] ), invTransform.p );
	}

	localProxy.points = localPoints;
	localProxy.radius = proxy->radius;

	if ( type == b3_meshShape )
	{
		return b3OverlapMesh( &localProxy, shape->mesh.data, shape->mesh.scale );
	}

	B3_ASSERT( type == b3_heightShape );

	return b3OverlapHeightField( &localProxy, shape->heightField );
#endif
}

int b3CollideMover( b3PlaneResult* planes, int planeCapacity, const b3Shape* shape, b3Transform transform,
					const b3Capsule* mover )
{
	if ( planeCapacity == 0 )
	{
		return 0;
	}

	b3Capsule localMover;
	localMover.center1 = b3InvTransformPoint( transform, mover->center1 );
	localMover.center2 = b3InvTransformPoint( transform, mover->center2 );
	localMover.radius = mover->radius;

	int planeCount = 0;
	switch ( shape->type )
	{
		case b3_capsuleShape:
			planeCount = b3CollideMoverAndCapsule( planes, &shape->capsule, &localMover );
			break;

		case b3_compoundShape:
			planeCount = b3CollideMoverAndCompound( planes, planeCapacity, shape->compound, &localMover );
			break;

		case b3_sphereShape:
			planeCount = b3CollideMoverAndSphere( planes, &shape->sphere, &localMover );
			break;

		case b3_hullShape:
			planeCount = b3CollideMoverAndHull( planes, shape->hull, &localMover );
			break;

		case b3_meshShape:
			planeCount = b3CollideMoverAndMesh( planes, planeCapacity, &shape->mesh, &localMover );
			break;

		case b3_heightShape:
			planeCount = b3CollideMoverAndHeightField( planes, planeCapacity, shape->heightField, &localMover );
			break;

		default:
			B3_ASSERT( false );
			break;
	}

	for ( int i = 0; i < planeCount; ++i )
	{
		planes[i].plane.normal = b3RotateVector( transform.q, planes[i].plane.normal );
		planes[i].point = b3TransformPoint( transform, planes[i].point );
	}

	return planeCount;
}

void b3CreateShapeProxy( b3Shape* shape, b3BroadPhase* bp, b3BodyType type, b3WorldTransform transform, bool forcePairCreation )
{
	B3_ASSERT( shape->proxyKey == B3_NULL_INDEX );

	b3UpdateShapeAABBs( shape, transform, type );

	// Create proxies in the broad-phase.
	shape->proxyKey =
		b3BroadPhase_CreateProxy( bp, type, shape->fatAABB, shape->filter.categoryBits, shape->id, forcePairCreation );
	B3_ASSERT( B3_PROXY_TYPE( shape->proxyKey ) < b3_bodyTypeCount );
}

void b3DestroyShapeProxy( b3Shape* shape, b3BroadPhase* bp )
{
	if ( shape->proxyKey != B3_NULL_INDEX )
	{
		b3BroadPhase_DestroyProxy( bp, shape->proxyKey );
		shape->proxyKey = B3_NULL_INDEX;
	}
}

static void b3DestroyShapeAllocationForShapeChange( b3World* world, b3Shape* shape )
{
	b3ShapeType type = shape->type;
	switch ( type )
	{
		case b3_hullShape:
			b3RemoveHullFromDatabase( world, shape->hull );
			shape->hull = NULL;
			break;

		default:
			break;
	}

	if ( shape->userShape != NULL )
	{
		world->destroyDebugShape( shape->userShape, world->userDebugShapeContext );
		shape->userShape = NULL;
	}
}

void b3DestroyShapeAllocations( b3World* world, b3Shape* shape )
{
	b3DestroyShapeAllocationForShapeChange( world, shape );

	if ( shape->materials != NULL )
	{
		B3_ASSERT( shape->materialCount > 0 );
		b3Free( shape->materials, shape->materialCount * sizeof( b3SurfaceMaterial ) );
		shape->materials = NULL;
		shape->materialCount = 0;
	}

	// Name is stored inline. Sensor data is destroyed elsewhere
}

b3ShapeProxy b3MakeShapeProxy( const b3Shape* shape )
{
	switch ( shape->type )
	{
		case b3_capsuleShape:
			return (b3ShapeProxy){ &shape->capsule.center1, 2, shape->capsule.radius };

		case b3_sphereShape:
			return (b3ShapeProxy){ &shape->sphere.center, 1, shape->sphere.radius };

		case b3_hullShape:
		{
			const b3HullData* hull = shape->hull;
			const b3Vec3* points = b3GetHullPoints( hull );
			return (b3ShapeProxy){ points, hull->vertexCount, 0.0f };
		}

		default:
		{
			B3_ASSERT( false );
			return (b3ShapeProxy){ 0 };
		}
	}
}

b3ShapeProxy b3MakeLocalProxy( const b3ShapeProxy* proxy, b3Transform transform, b3Vec3* buffer )
{
	b3Transform invTransform = b3InvertTransform( transform );
	b3Matrix3 R = b3MakeMatrixFromQuat( invTransform.q );

	int count = b3MinInt( proxy->count, B3_MAX_SHAPE_CAST_POINTS );
	for ( int i = 0; i < count; ++i )
	{
		buffer[i] = b3Add( b3MulMV( R, proxy->points[i] ), invTransform.p );
	}

	return (b3ShapeProxy){
		.points = buffer,
		.count = count,
		.radius = proxy->radius,
	};
}

b3AABB b3ComputeProxyAABB( const b3ShapeProxy* proxy )
{
	const b3Vec3* points = proxy->points;
	b3AABB aabb = {
		.lowerBound = points[0],
		.upperBound = points[0],
	};

	for ( int i = 1; i < proxy->count; ++i )
	{
		aabb.lowerBound = b3Min( aabb.lowerBound, points[i] );
		aabb.upperBound = b3Max( aabb.upperBound, points[i] );
	}

	b3Vec3 r = { proxy->radius, proxy->radius, proxy->radius };
	aabb.lowerBound = b3Sub( aabb.lowerBound, r );
	aabb.upperBound = b3Add( aabb.upperBound, r );
	return aabb;
}




















































// todo no tests






























































































































































static void b3ResetProxy( b3World* world, b3Shape* shape, bool wakeBodies, bool destroyProxy )
{
	b3Body* body = b3Array_Get( world->bodies, shape->bodyId );

	int shapeId = shape->id;

	// destroy all contacts associated with this shape
	int contactKey = body->headContactKey;
	while ( contactKey != B3_NULL_INDEX )
	{
		int contactId = contactKey >> 1;
		int edgeIndex = contactKey & 1;

		b3Contact* contact = b3Array_Get( world->contacts, contactId );
		contactKey = contact->edges[edgeIndex].nextKey;

		if ( contact->shapeIdA == shapeId || contact->shapeIdB == shapeId )
		{
			b3DestroyContact( world, contact, wakeBodies );
		}
	}

	b3WorldTransform transform = b3GetBodyTransformQuick( world, body );
	if ( shape->proxyKey != B3_NULL_INDEX )
	{
		b3BodyType proxyType = B3_PROXY_TYPE( shape->proxyKey );
		b3UpdateShapeAABBs( shape, transform, proxyType );

		if ( destroyProxy )
		{
			b3BroadPhase_DestroyProxy( &world->broadPhase, shape->proxyKey );

			bool forcePairCreation = true;
			shape->proxyKey = b3BroadPhase_CreateProxy( &world->broadPhase, proxyType, shape->fatAABB, shape->filter.categoryBits,
														shapeId, forcePairCreation );
		}
		else
		{
			b3BroadPhase_MoveProxy( &world->broadPhase, shape->proxyKey, shape->fatAABB );
		}
	}
	else
	{
		b3BodyType proxyType = body->type;
		b3UpdateShapeAABBs( shape, transform, proxyType );
	}

	b3ValidateSolverSets( world );
}





























































































































































































































































































































































































































































































#define B3_DEBUG_WIND 0

// https://en.wikipedia.org/wiki/Density_of_air
// https://www.engineeringtoolbox.com/wind-load-d_1775.html
// force = 0.5 * air_density * velocity^2 * area
// https://en.wikipedia.org/wiki/Lift_(force)













































































































































































































typedef struct b3MeshImpactContext
{
	b3TOIInput toiInput;
	b3TOIOutput toiOutput;
	// Centroid of shape in body B local space
	b3Vec3 localCentroidB;
	// Centroid of shape at beginning and end of sweep in mesh local space. Used for early out.
	b3Vec3 meshLocalCentroidB1, meshLocalCentroidB2;
	float fallbackRadius;
	bool isSensor;

	int visitCount;
} b3MeshImpactContext;

static bool b3MeshTimeOfImpactFcn( b3Vec3 a, b3Vec3 b, b3Vec3 c, int triangleIndex, void* context )
{
	B3_UNUSED( triangleIndex );

	b3MeshImpactContext* toiContext = context;

	toiContext->visitCount += 1;

	// Early out for parallel movement
	b3Vec3 c1 = toiContext->meshLocalCentroidB1;
	b3Vec3 c2 = toiContext->meshLocalCentroidB2;

	b3Vec3 n = b3Normalize( b3Cross( b3Sub( b, a ), b3Sub( c, a ) ) );
	float offset1 = b3Dot( n, b3Sub( c1, a ) );
	float offset2 = b3Dot( n, b3Sub( c2, a ) );

	if ( offset1 < 0.0f )
	{
		// Started behind or finished in front
		return true;
	}

	if ( toiContext->isSensor == false && offset1 - offset2 < toiContext->fallbackRadius && offset2 > toiContext->fallbackRadius )
	{
		// Finished in front
		return true;
	}

	b3Vec3 triangle[3] = { a, b, c };
	toiContext->toiInput.proxyA.points = triangle;
	toiContext->toiInput.proxyA.count = 3;

	b3TOIOutput output = b3TimeOfImpact( &toiContext->toiInput );

	// It is possible for a hit at fraction == 0

	if ( 0.0f < output.fraction && output.fraction < toiContext->toiInput.maxFraction )
	{
		toiContext->toiOutput = output;
		toiContext->toiInput.maxFraction = output.fraction;
	}
	else if ( 0.0f == output.fraction )
	{
		// fallback to TOI of a small circle around the fast shape centroid
		b3TOIInput fallbackInput = toiContext->toiInput;
		fallbackInput.proxyB = (b3ShapeProxy){ &toiContext->localCentroidB, 1, toiContext->fallbackRadius + B3_LINEAR_SLOP };
		output = b3TimeOfImpact( &fallbackInput );

		if ( 0.0f < output.fraction && output.fraction < toiContext->toiInput.maxFraction )
		{
			toiContext->toiOutput = output;
			toiContext->toiInput.maxFraction = output.fraction;
			toiContext->toiOutput.usedFallback = true;
		}
	}

	// Continue the query
	return true;
}

typedef struct b3CompoundImpactContext
{
	b3TOIInput toiInput;
	b3TOIOutput toiOutput;
	b3Transform compoundTransform;

	// Bounds local to compound
	b3AABB localSweepBoundsB;

	// Centroid of shape in body B local space
	b3Vec3 localCentroidB;
	float fallbackRadius;
} b3CompoundImpactContext;

// Implements b3CompoundQueryFcn
static bool b3CompoundTimeOfImpactFcn( const b3CompoundData* compound, int childIndex, void* context )
{
	b3CompoundImpactContext* toiContext = (b3CompoundImpactContext*)context;

	b3ChildShape child = b3GetCompoundChild( compound, childIndex );

	b3TOIOutput output = { 0 };
	toiContext->toiInput.sweepA = b3MakeCompoundChildSweep( toiContext->compoundTransform, child.transform );

	switch ( child.type )
	{
		case b3_capsuleShape:
		{
			toiContext->toiInput.proxyA.points = &child.capsule.center1;
			toiContext->toiInput.proxyA.count = 2;
			toiContext->toiInput.proxyA.radius = child.capsule.radius;
			output = b3TimeOfImpact( &toiContext->toiInput );
		}
		break;

		case b3_hullShape:
		{
			toiContext->toiInput.proxyA.points = b3GetHullPoints( child.hull );
			toiContext->toiInput.proxyA.count = child.hull->vertexCount;
			toiContext->toiInput.proxyA.radius = 0.0f;
			output = b3TimeOfImpact( &toiContext->toiInput );
		}
		break;

		case b3_meshShape:
		{
			b3MeshImpactContext meshContext = { 0 };
			meshContext.toiInput = toiContext->toiInput;
			meshContext.isSensor = false;
			meshContext.localCentroidB = toiContext->localCentroidB;
			meshContext.fallbackRadius = toiContext->fallbackRadius;

			b3Transform meshWorldTransform = b3MulTransforms( toiContext->compoundTransform, child.transform );

			const b3Sweep* sweepB = &toiContext->toiInput.sweepB;
			b3Transform xfB1 = {
				.p = b3Sub( sweepB->c1, b3RotateVector( sweepB->q1, sweepB->localCenter ) ),
				.q = sweepB->q1,
			};

			b3Transform xfB2 = {
				.p = b3Sub( sweepB->c2, b3RotateVector( sweepB->q2, sweepB->localCenter ) ),
				.q = sweepB->q2,
			};

			meshContext.meshLocalCentroidB1 =
				b3InvTransformPoint( meshWorldTransform, b3TransformPoint( xfB1, meshContext.localCentroidB ) );
			meshContext.meshLocalCentroidB2 =
				b3InvTransformPoint( meshWorldTransform, b3TransformPoint( xfB2, meshContext.localCentroidB ) );

			// Bounds local to mesh
			b3AABB localBounds = b3AABB_Transform( b3InvertTransform( child.transform ), toiContext->localSweepBoundsB );

			b3QueryMesh( &child.mesh, localBounds, b3MeshTimeOfImpactFcn, &meshContext );

			output = meshContext.toiOutput;
		}
		break;

		case b3_sphereShape:
		{
			toiContext->toiInput.proxyA.points = &child.sphere.center;
			toiContext->toiInput.proxyA.count = 1;
			toiContext->toiInput.proxyA.radius = child.sphere.radius;
			output = b3TimeOfImpact( &toiContext->toiInput );
		}
		break;

		default:
			B3_ASSERT( false );
			break;
	}

	if ( 0.0f < output.fraction && output.fraction < toiContext->toiInput.maxFraction )
	{
		toiContext->toiOutput = output;
		toiContext->toiInput.maxFraction = output.fraction;
	}

	// Clear this to be safe
	toiContext->toiInput.proxyA = (b3ShapeProxy){ 0 };

	// Continue the query
	return true;
}

b3TOIOutput b3ShapeTimeOfImpact( b3Shape* shapeA, b3Shape* shapeB, b3Sweep* sweepA, b3Sweep* sweepB, float maxFraction )
{
	bool isSensor = shapeA->sensorIndex != B3_NULL_INDEX;

	b3ShapeType typeA = shapeA->type;
	if ( typeA == b3_compoundShape )
	{
		// todo implement b3CompoundTimeOfImpact
		b3CompoundImpactContext context = { 0 };
		context.toiInput.proxyB = b3MakeShapeProxy( shapeB );
		context.toiInput.sweepB = *sweepB;
		context.toiInput.maxFraction = maxFraction;

		context.compoundTransform = (b3Transform){
			.p = sweepA->c1,
			.q = sweepA->q1,
		};

		b3Vec3 localCentroidB = b3GetShapeCentroid( shapeB );
		context.localCentroidB = localCentroidB;

		b3ShapeExtent extents = b3ComputeShapeExtent( shapeB, context.localCentroidB );
		context.fallbackRadius = b3MaxFloat( 0.75f * extents.minExtent, B3_SPECULATIVE_DISTANCE );

		// Swept bounds of shapeB
		b3AABB bounds = b3ComputeSweptShapeAABB( shapeB, sweepB, maxFraction );

		// Bounds local to mesh
		b3AABB localBounds = b3AABB_Transform( b3InvertTransform( context.compoundTransform ), bounds );
		context.localSweepBoundsB = localBounds;

		b3QueryCompound( shapeA->compound, localBounds, b3CompoundTimeOfImpactFcn, &context );

		return context.toiOutput;
	}

	if ( typeA == b3_heightShape || typeA == b3_meshShape )
	{
		// todo implement b3MeshTimeOfImpact and b3HeightFieldTimeOfImpact
		// Note: assuming mesh is static

		uint64_t ticks = b3GetTicks();

		b3MeshImpactContext context = { 0 };
		context.toiInput.sweepA = *sweepA;
		context.toiInput.proxyA.count = 3;
		context.toiInput.proxyB = b3MakeShapeProxy( shapeB );
		context.toiInput.sweepB = *sweepB;
		context.toiInput.maxFraction = maxFraction;
		context.isSensor = isSensor;

		b3Vec3 localCentroidB = b3GetShapeCentroid( shapeB );
		context.localCentroidB = localCentroidB;

		// Assume mesh is static
		b3Transform xfA = {
			.p = b3Sub( sweepA->c1, b3RotateVector( sweepA->q1, sweepA->localCenter ) ),
			.q = sweepA->q1,
		};

		b3Transform xfB1 = {
			.p = b3Sub( sweepB->c1, b3RotateVector( sweepB->q1, sweepB->localCenter ) ),
			.q = sweepB->q1,
		};

		b3Transform xfB2 = {
			.p = b3Sub( sweepB->c2, b3RotateVector( sweepB->q2, sweepB->localCenter ) ),
			.q = sweepB->q2,
		};

		context.meshLocalCentroidB1 = b3InvTransformPoint( xfA, b3TransformPoint( xfB1, localCentroidB ) );
		context.meshLocalCentroidB2 = b3InvTransformPoint( xfA, b3TransformPoint( xfB2, localCentroidB ) );

		b3ShapeExtent extents = b3ComputeShapeExtent( shapeB, context.localCentroidB );
		context.fallbackRadius = b3MaxFloat( 0.5f * extents.minExtent, B3_LINEAR_SLOP );

		// Swept bounds of shapeB
		// todo pass in xfA to get local bounds directly
		b3AABB bounds = b3ComputeSweptShapeAABB( shapeB, sweepB, maxFraction );

		// Bounds local to mesh
		b3AABB localBounds = b3AABB_Transform( b3InvertTransform( xfA ), bounds );

		if ( typeA == b3_meshShape )
		{
			b3QueryMesh( &shapeA->mesh, localBounds, b3MeshTimeOfImpactFcn, &context );
		}
		else if ( typeA == b3_heightShape )
		{
			b3QueryHeightField( shapeA->heightField, localBounds, b3MeshTimeOfImpactFcn, &context );
		}

		float ms = b3GetMilliseconds( ticks );
		if ( ms > 1000.0f * b3GetStallThreshold() )
		{
			b3Log( "CCD stall: visited %d triangles", context.visitCount );
		}

		return context.toiOutput;
	}

	B3_ASSERT( shapeB->type != b3_compoundShape && shapeB->type != b3_meshShape && shapeB->type != b3_heightShape );

	b3TOIInput input;
	input.proxyA = b3MakeShapeProxy( shapeA );
	input.proxyB = b3MakeShapeProxy( shapeB );
	input.sweepA = *sweepA;
	input.sweepB = *sweepB;
	input.maxFraction = maxFraction;

	b3TOIOutput output = b3TimeOfImpact( &input );

#if 0
	// todo I'm not sure this is worth it for convex vs convex.
	if (0.0f < output.fraction && output.fraction < maxFraction)
	{
		return output;
	}

	if (0.0f == output.fraction)
	{
		// fallback to TOI of a small circle around the fast shape centroid
		b3Vec3 centroid = b3GetShapeCentroid( shapeB );
		input.proxyB = ( b3ShapeProxy ){ &centroid, 1, B3_SPECULATIVE_DISTANCE };
		output = b3TimeOfImpact( &input );
		return output;
	}
#endif

	return output;
}

// Resolve the user material id for a hit point on the given shape. Mesh/heightfield shapes
// use the manifold-point triangleIndex to pick a per-triangle material. Compound shapes use
// the contact's childIndex to find the participating child, then for a mesh child apply the
// child's materialIndices indirection on top of the per-triangle index. Convex shapes fall
// back to materials[0]. childIndex is unused for non-compound shapes.
uint64_t b3GetShapeUserMaterialId( const b3Shape* shape, int childIndex, int triangleIndex )
{
	if ( shape->materialCount == 0 )
	{
		return 0;
	}

	int materialIndex = 0;
	if ( shape->type == b3_meshShape )
	{
		const uint8_t* indices = b3GetMeshMaterialIndices( shape->mesh.data );
		if ( indices != NULL )
		{
			materialIndex = indices[triangleIndex];
		}
	}
	else if ( shape->type == b3_heightShape )
	{
		materialIndex = b3GetHeightFieldMaterial( shape->heightField, triangleIndex );
	}
	else if ( shape->type == b3_compoundShape )
	{
		b3ChildShape child = b3GetCompoundChild( shape->compound, childIndex );
		if ( child.type == b3_meshShape )
		{
			const uint8_t* indices = b3GetMeshMaterialIndices( child.mesh.data );
			int meshMaterialIndex = indices != NULL ? indices[triangleIndex] : 0;
			meshMaterialIndex = b3ClampInt( meshMaterialIndex, 0, B3_MAX_COMPOUND_MESH_MATERIALS - 1 );
			materialIndex = child.materialIndices[meshMaterialIndex];
		}
		else
		{
			materialIndex = child.materialIndices[0];
		}
	}

	materialIndex = b3ClampInt( materialIndex, 0, shape->materialCount - 1 );
	return b3GetShapeMaterials( shape )[materialIndex].userMaterialId;
}
