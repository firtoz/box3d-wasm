// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#include "physics_world.h"

#include "arena_allocator.h"
#include "bitset.h"
#include "body.h"
#include "broad_phase.h"
#include "constraint_graph.h"
#include "contact.h"
#include "core.h"
#include "ctz.h"
#include "hull.h"
#include "island.h"
#include "joint.h"
#include "parallel_for.h"
#include "platform.h"
#include "recording.h"
#include "scheduler.h"
#include "sensor.h"
#include "shape.h"
#include "solver.h"
#include "solver_set.h"

#include "box3d/box3d.h"
#include "box3d/constants.h"

#include <float.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

_Static_assert( B3_MAX_WORLDS > 0, "must be 1 or more" );
_Static_assert( B3_MAX_WORLDS < UINT16_MAX, "B3_MAX_WORLDS limit exceeded" );
b3World b3_worlds[B3_MAX_WORLDS];
b3AtomicInt b3_worldCount;
int b3_maxWorldCount;

const b3HullData* b3AddHullToDatabase( b3World* world, const b3HullData* src )
{
	b3HullMap* database = world->hullDatabase;

	// Compare by content to de-duplicate. Not trusting the hash.
	b3HullMap_itr itr = b3HullMap_get( database, src );
	if ( b3HullMap_is_end( itr ) == false )
	{
		// Bump reference count.
		itr.data->val += 1;
		return itr.data->key;
	}

	b3HullData* clone = b3CloneHull( src );
	B3_ASSERT( clone != NULL );

	// Start with reference count of 1.
	b3HullMap_insert( database, clone, 1 );
	return clone;
}

const b3HullData* b3AddOwnedHullToDatabase( b3World* world, b3HullData* owned )
{
	b3HullMap* database = world->hullDatabase;

	b3HullMap_itr itr = b3HullMap_get( database, owned );
	if ( b3HullMap_is_end( itr ) == false )
	{
		itr.data->val += 1;
		b3DestroyHull( owned );
		return itr.data->key;
	}

	// Take ownership of input hull.
	b3HullMap_insert( database, owned, 1 );
	return owned;
}

void b3RemoveHullFromDatabase( b3World* world, const b3HullData* data )
{
	b3HullMap* database = world->hullDatabase;

	b3HullMap_itr itr = b3HullMap_get( database, data );
	B3_ASSERT( b3HullMap_is_end( itr ) == false );

	if ( --itr.data->val == 0 )
	{
		// Erase through the iterator we already have so the lookup runs once.
		b3HullData* owned = (b3HullData*)itr.data->key;
		b3HullMap_erase_itr( database, itr );
		b3DestroyHull( owned );
	}
}

b3World* b3GetUnlockedWorldFromId( b3WorldId id )
{
	B3_ASSERT( 1 <= id.index1 && id.index1 <= B3_MAX_WORLDS );
	b3World* world = b3_worlds + ( id.index1 - 1 );
	B3_ASSERT( id.index1 == world->worldId + 1 );
	B3_ASSERT( id.generation == world->generation );

	// A world accessed from an id should not be locked
	if ( world->locked )
	{
		B3_ASSERT( false );
		return NULL;
	}
	return world;
}

b3World* b3GetWorldFromId( b3WorldId id )
{
	B3_ASSERT( 1 <= id.index1 && id.index1 <= B3_MAX_WORLDS );
	b3World* world = b3_worlds + ( id.index1 - 1 );
	B3_ASSERT( id.index1 == world->worldId + 1 );
	B3_ASSERT( id.generation == world->generation );
	return world;
}

b3World* b3GetWorld( int index )
{
	B3_ASSERT( 0 <= index && index < B3_MAX_WORLDS );
	b3World* world = b3_worlds + index;
	B3_ASSERT( world->worldId == index );
	return world;
}

b3World* b3GetUnlockedWorld( int index )
{
	B3_ASSERT( 0 <= index && index < B3_MAX_WORLDS );
	b3World* world = b3_worlds + index;
	B3_ASSERT( world->worldId == index );
	if ( world->locked )
	{
		B3_ASSERT( false );
		return NULL;
	}

	return world;
}

static void* b3DefaultAddTaskFcn( b3TaskCallback* task, void* taskContext, void* userContext, const char* name )
{
	B3_UNUSED( userContext, name );
	task( taskContext );
	return NULL;
}

static void b3DefaultFinishTaskFcn( void* userTask, void* userContext )
{
	B3_UNUSED( userTask );
	B3_UNUSED( userContext );
}

static float b3DefaultFrictionCallback( float frictionA, uint64_t materialA, float frictionB, uint64_t materialB )
{
	B3_UNUSED( materialA, materialB );
	return sqrtf( frictionA * frictionB );
}

static float b3DefaultRestitutionCallback( float restitutionA, uint64_t materialA, float restitutionB, uint64_t materialB )
{
	B3_UNUSED( materialA, materialB );
	return b3MaxFloat( restitutionA, restitutionB );
}

static void b3CreateWorkerContexts( b3World* world )
{
	b3Array_Resize( world->taskContexts, world->workerCount );
	b3Array_MemZero( world->taskContexts );

	b3Array_Resize( world->sensorTaskContexts, world->workerCount );
	b3Array_MemZero( world->sensorTaskContexts );

	for ( int i = 0; i < world->workerCount; ++i )
	{
		world->taskContexts.data[i].arena = b3CreateArena( 128 * 1024 );
		b3Array_Reserve( world->taskContexts.data[i].sensorHits, 8 );
		world->taskContexts.data[i].contactStateBitSet = b3CreateBitSet( 1024 );
		world->taskContexts.data[i].hitEventBitSet = b3CreateBitSet( 1024 );
		world->taskContexts.data[i].hasHitEvents = false;
		world->taskContexts.data[i].jointStateBitSet = b3CreateBitSet( 1024 );
		world->taskContexts.data[i].enlargedSimBitSet = b3CreateBitSet( 256 );
		world->taskContexts.data[i].awakeIslandBitSet = b3CreateBitSet( 256 );
		world->taskContexts.data[i].splitIslandId = B3_NULL_INDEX;

		world->sensorTaskContexts.data[i].eventBits = b3CreateBitSet( 128 );
	}
}

static void b3DestroyWorkerContexts( b3World* world )
{
	for ( int i = 0; i < world->workerCount; ++i )
	{
		b3DestroyArena( &world->taskContexts.data[i].arena );
		b3Array_Destroy( world->taskContexts.data[i].sensorHits );
		b3DestroyBitSet( &world->taskContexts.data[i].contactStateBitSet );
		b3DestroyBitSet( &world->taskContexts.data[i].hitEventBitSet );
		b3DestroyBitSet( &world->taskContexts.data[i].jointStateBitSet );
		b3DestroyBitSet( &world->taskContexts.data[i].enlargedSimBitSet );
		b3DestroyBitSet( &world->taskContexts.data[i].awakeIslandBitSet );

		b3DestroyBitSet( &world->sensorTaskContexts.data[i].eventBits );
	}

	b3Array_Destroy( world->taskContexts );
	b3Array_Destroy( world->sensorTaskContexts );
}


















































































































































































































































































































































// Issues T0 prefetches across the cache lines of a b3Contact (216 B / 4 lines).
// Used to hide the random-access latency of contact lookups while we work on an
// earlier index.
static inline void b3PrefetchContact( const b3Contact* contact )
{
	const char* p = (const char*)contact;
	b3Prefetch( p );
	b3Prefetch( p + 64 );
	b3Prefetch( p + 128 );
	b3Prefetch( p + 192 );
}

static void b3CollideTask( int startIndex, int endIndex, int workerIndex, void* context )
{
	b3TracyCZoneNC( collide_task, "Collide Task", b3_colorDodgerBlue, true );

	b3StepContext* stepContext = (b3StepContext*)context;
	b3World* world = stepContext->world;
	b3ConstraintGraph* graph = &world->constraintGraph;
	b3TaskContext* taskContext = world->taskContexts.data + workerIndex;
	int* contactIndices = stepContext->awakeContactIndices;
	b3Contact* contacts = world->contacts.data;
	b3Shape* shapes = world->shapes.data;
	b3Body* bodies = world->bodies.data;
	b3BodySim* awakeSims = world->solverSets.data[b3_awakeSet].bodySims.data;
	b3BodySim* staticSims = world->solverSets.data[b3_staticSet].bodySims.data;

	B3_ASSERT( startIndex < endIndex );

	float recycleDistance = world->contactRecycleDistance;
	float speculativeDistance = B3_SPECULATIVE_DISTANCE;
	float recycleDistanceNonTouching = b3MinFloat( recycleDistance, speculativeDistance );

	// Prefetch contact[i + contactPrefetchDistance] each iteration so the random
	// 216 B contact load lands in L1 by the time we reach it. Distance picked to
	// cover ~200 cycles of memory latency without overshooting the L1 working set.
	const int contactPrefetchDistance = 4;
	int prefetchEnd = endIndex - contactPrefetchDistance;

	for ( int i = startIndex; i < endIndex; ++i )
	{
		if ( i < prefetchEnd )
		{
			b3PrefetchContact( contacts + contactIndices[i + contactPrefetchDistance] );
		}

		int contactIndex = contactIndices[i];
		B3_ASSERT( contactIndex < world->contacts.count );

		b3Contact* contact = contacts + contactIndex;
		B3_VALIDATE( contact->contactId == contactIndex );

		b3Shape* shapeA = shapes + contact->shapeIdA;
		b3Shape* shapeB = shapes + contact->shapeIdB;

		// Do proxies still overlap?
		bool overlap = b3AABB_Overlaps( shapeA->fatAABB, shapeB->fatAABB );
		if ( overlap == false )
		{
			// This contact will be destroyed
			contact->flags |= b3_simDisjoint;
			contact->flags &= ~b3_simTouchingFlag;
			b3SetBit( &taskContext->contactStateBitSet, contactIndex );
			continue;
		}

		// Update contact respecting shape/body order (A,B). Bodies behind awake-set
		// contacts are always either awake or static - inline b3GetBodySim with that
		// invariant to skip the cross-TU call and per-call solverSets indirection.
		b3Body* bodyA = bodies + shapeA->bodyId;
		b3Body* bodyB = bodies + shapeB->bodyId;
		bool isStaticA = bodyA->type == b3_staticBody;
		bool isStaticB = bodyB->type == b3_staticBody;
		bool wasTouching = ( contact->flags & b3_simTouchingFlag );
		bool isMeshContact = ( contact->flags & b3_simMeshContact );
		b3BodySim* bodySimA;
		b3BodySim* bodySimB;
		if ( wasTouching )
		{
			B3_ASSERT( bodyA->setIndex == b3_awakeSet || bodyA->setIndex == b3_staticSet );
			B3_ASSERT( bodyB->setIndex == b3_awakeSet || bodyB->setIndex == b3_staticSet );
			bodySimA = ( isStaticA ? staticSims : awakeSims ) + bodyA->localIndex;
			bodySimB = ( isStaticB ? staticSims : awakeSims ) + bodyB->localIndex;
		}
		else
		{
			// There can be non-touching contacts between awake bodies and sleeping bodies.
			{
				b3SolverSet* set = b3Array_Get( world->solverSets, bodyA->setIndex );
				bodySimA = b3Array_Get( set->bodySims, bodyA->localIndex );
			}
			{
				b3SolverSet* set = b3Array_Get( world->solverSets, bodyB->setIndex );
				bodySimB = b3Array_Get( set->bodySims, bodyB->localIndex );
			}
		}

		b3WorldTransform transformA = bodySimA->transform;
		b3WorldTransform transformB = bodySimB->transform;

		bool isFast = ( bodySimA->flags & b3_isFast ) || ( bodySimB->flags & b3_isFast );

		// These are used by the contact solver. If the contact is between an awake body
		// and a sleeping body and the contact begins to touch, the these will be invalid
		// but fixed when linked in the constraint graph.
		contact->bodySimIndexA = isStaticA ? B3_NULL_INDEX : bodyA->localIndex;
		contact->bodySimIndexB = isStaticB ? B3_NULL_INDEX : bodyB->localIndex;
		float recycleTolerance = wasTouching ? recycleDistance : recycleDistanceNonTouching;

		// Contact recycling optimization. Please cite this library if you use this optimization.
		// This is inspired by persistent contact manifolds used in some physics engines, such as PhysX.
		// However, this allows larger relative motion and has fewer tuning parameters (just one).
		if ( ( isFast == false || isMeshContact == false ) && recycleDistance > 0.0f &&
			 ( contact->flags & b3_relativeTransformValid ) && ( contact->flags & b3_contactRecycleFlag ) )
		{
			// The scalar part of b3InvMulQuat is just the quaternion dot product.
			// cos(relative_angle/2) = scalar(conj(q1) * q2) = dot(q1, q2)
			// A small relative angle means this value is close to 1. Need to use abs or square
			// due to double cover.
			float angleA = b3DotQuat( transformA.q, contact->cachedRotationA );
			float angleB = b3DotQuat( transformB.q, contact->cachedRotationB );
			float angularDistance = b3MinFloat( angleA * angleA, angleB * angleB );

			b3Transform xf = b3InvMulWorldTransforms( transformA, transformB );
			b3Transform xfc = contact->cachedRelativePose;
			b3Vec3 maxExtentA = isStaticA ? b3Vec3_zero : bodySimA->maxExtent;
			b3Vec3 maxExtentB = isStaticB ? b3Vec3_zero : bodySimB->maxExtent;
			b3Vec3 maxExtent = b3Max( maxExtentA, maxExtentB );

			// Variation of Conservative Advancement
			// distance + 2 * length(modified_cross(|qr.v|, maxExtent)) < recycleTolerance.
			// 2*|qr.v| == 2*|sin(theta/2)| ~= theta for small angles.
			float distSquared = b3DistanceSquared( xf.p, xfc.p );

			if ( angularDistance > B3_CONTACT_RECYCLE_ANGULAR_DISTANCE && distSquared < recycleTolerance * recycleTolerance )
			{
				float distance = sqrtf( distSquared );
				float slack = recycleTolerance - distance;

				// qr = inv( inv(qA0) * qB0 ) * inv(qA) * qB
				//    = inv(qB0) * qA0 * inv(qA) * qB
				// Suppose A is static
				// qr = inv(qB0) * qA0 * inv(qA0) * qB
				//    = inv(qB0) * qB
				// qB = qB0 * qr
				// Therefore qr is associated with the local angular velocity of body B when A is static.
				b3Quat qr = b3InvMulQuat( xfc.q, xf.q );
				b3Vec3 arc = b3ModifiedCross( b3Abs( qr.v ), maxExtent );

				float arcSq = 4.0f * b3LengthSquared( arc );
				if ( arcSq < slack * slack )
				{
					b3Quat dqA = b3MulQuat( transformA.q, b3Conjugate( contact->cachedRotationA ) );
					b3Quat dqB = b3MulQuat( transformB.q, b3Conjugate( contact->cachedRotationB ) );
					b3Matrix3 matrixA = b3MakeMatrixFromQuat( dqA );
					b3Matrix3 matrixB = b3MakeMatrixFromQuat( dqB );

					// Minimize round-off
					b3Vec3 dc = b3SubPos( bodySimB->center, bodySimA->center );

					int manifoldCount = contact->manifoldCount;
					for ( int manifoldIndex = 0; manifoldIndex < manifoldCount; ++manifoldIndex )
					{
						b3Manifold* manifold = contact->manifolds + manifoldIndex;
						b3Vec3 normal = manifold->normal;

						int pointCount = manifold->pointCount;
						for ( int pointIndex = 0; pointIndex < pointCount; ++pointIndex )
						{
							// Keep anchors but update separation, same as sub-stepping. This eliminates jitter.
							b3ManifoldPoint* mp = manifold->points + pointIndex;
							b3Vec3 rA = b3MulMV( matrixA, mp->anchorA );
							b3Vec3 rB = b3MulMV( matrixB, mp->anchorB );
							b3Vec3 dp = b3Add( dc, b3Sub( rB, rA ) );
							mp->separation = mp->baseSeparation + b3Dot( dp, normal );
							mp->persisted = true;
						}
					}

					// Diagnostics
					taskContext->recycledContactCount += 1;
					int bucketIndex = b3MinInt( manifoldCount, B3_CONTACT_MANIFOLD_COUNT_BUCKETS - 1 );
					if ( bucketIndex > 0 )
					{
						taskContext->manifoldCounts[bucketIndex - 1] += 1;
					}

					// Contact is recycled. This also skips updating other aspects of the contact
					// such as material parameters.
					continue;
				}
			}
		}

		// Caching for contact recycling.
		contact->cachedRotationA = transformA.q;
		contact->cachedRotationB = transformB.q;
		contact->cachedRelativePose = b3InvMulWorldTransforms( transformA, transformB );
		contact->flags |= b3_relativeTransformValid;

		// This updates solid contacts
		bool touching = b3UpdateContact( world, workerIndex, contact, shapeA, bodySimA->localCenter, transformA, shapeB,
										 bodySimB->localCenter, transformB, isFast, taskContext->arena );

		int bucketIndex = b3MinInt( contact->manifoldCount, B3_CONTACT_MANIFOLD_COUNT_BUCKETS - 1 );
		if ( bucketIndex > 0 )
		{
			taskContext->manifoldCounts[bucketIndex - 1] += 1;
		}

		// Update the mesh contact spec
		if ( touching == true && wasTouching == true && ( contact->flags & b3_simMeshContact ) )
		{
			B3_ASSERT( contact->colorIndex != B3_NULL_INDEX );
			B3_ASSERT( 0 <= contact->colorIndex && contact->colorIndex < B3_GRAPH_COLOR_COUNT );
			b3GraphColor* color = graph->colors + contact->colorIndex;
			b3ContactSpec* spec = b3Array_Get( color->contacts, contact->localIndex );
			spec->manifoldCount = (uint16_t)contact->manifoldCount;
		}

		// State changes that affect island connectivity. Also affects contact events.
		if ( touching == true && wasTouching == false )
		{
			contact->flags |= b3_simStartedTouching;
			b3SetBit( &taskContext->contactStateBitSet, contactIndex );
		}
		else if ( touching == false && wasTouching == true )
		{
			contact->flags |= b3_simStoppedTouching;
			b3SetBit( &taskContext->contactStateBitSet, contactIndex );
		}

		for ( int manifoldIndex = 0; manifoldIndex < contact->manifoldCount; ++manifoldIndex )
		{
			b3Manifold* manifold = contact->manifolds + manifoldIndex;
			for ( int pointIndex = 0; pointIndex < manifold->pointCount; ++pointIndex )
			{
				// Cache separation
				b3ManifoldPoint* mp = manifold->points + pointIndex;
				mp->baseSeparation = mp->separation;
			}
		}
	}

	b3TracyCZoneEnd( collide_task );
}

static void b3AddNonTouchingContact( b3World* world, b3Contact* contact )
{
	B3_ASSERT( contact->setIndex == b3_awakeSet );
	b3SolverSet* set = b3Array_Get( world->solverSets, b3_awakeSet );
	contact->colorIndex = B3_NULL_INDEX;
	contact->localIndex = set->contactIndices.count;
	contact->bodySimIndexA = B3_NULL_INDEX;
	contact->bodySimIndexB = B3_NULL_INDEX;
	b3Array_Push( set->contactIndices, contact->contactId );
}

static void b3RemoveNonTouchingContact( b3World* world, int setIndex, int localIndex )
{
	b3SolverSet* set = b3Array_Get( world->solverSets, setIndex );
	int movedIndex = b3Array_RemoveSwap( set->contactIndices, localIndex );
	if ( movedIndex != B3_NULL_INDEX )
	{
		int movedContactIndex = set->contactIndices.data[localIndex];
		b3Contact* movedContact = b3Array_Get( world->contacts, movedContactIndex );
		B3_ASSERT( movedContact->setIndex == setIndex );
		B3_ASSERT( movedContact->colorIndex == B3_NULL_INDEX );
		B3_ASSERT( movedContact->localIndex == movedIndex );
		movedContact->localIndex = localIndex;
	}
}

// Narrow-phase collision
static void b3Collide( b3StepContext* context )
{
	b3World* world = context->world;

	B3_ASSERT( world->workerCount > 0 );

	b3TracyCZoneNC( collide, "Collide", b3_colorDarkOrchid, true );

	// Gather contacts from all the graph colors into a single array for easier parallel-for
	int touchingCount = 0;

	b3GraphColor* graphColors = world->constraintGraph.colors;
	for ( int i = 0; i < B3_GRAPH_COLOR_COUNT; ++i )
	{
		touchingCount += graphColors[i].convexContacts.count + graphColors[i].contacts.count;
	}

	b3SolverSet* awakeSet = b3Array_Get( world->solverSets, b3_awakeSet );
	int nonTouchingCount = awakeSet->contactIndices.count;

	int contactCount = touchingCount + nonTouchingCount;

	if ( contactCount == 0 )
	{
		b3TracyCZoneEnd( collide );
		return;
	}

	int* contactIndices = (int*)b3StackAlloc( &world->stack, contactCount * sizeof( int ), "contact indices" );

	int contactIndex = 0;
	for ( int i = 0; i < B3_GRAPH_COLOR_COUNT; ++i )
	{
		b3GraphColor* color = graphColors + i;
		int count = color->convexContacts.count;
		for ( int j = 0; j < count; ++j )
		{
			contactIndices[contactIndex] = color->convexContacts.data[j];
			contactIndex += 1;
		}

		count = color->contacts.count;
		for ( int j = 0; j < count; ++j )
		{
			contactIndices[contactIndex] = color->contacts.data[j].contactId;
			contactIndex += 1;
		}
	}

	B3_ASSERT( contactIndex == touchingCount );

	if ( nonTouchingCount > 0 )
	{
		int* nonTouchingIndices = awakeSet->contactIndices.data;
		memcpy( contactIndices + touchingCount, nonTouchingIndices, nonTouchingCount * sizeof( int ) );
	}

	context->awakeContactIndices = contactIndices;

	// Contact bit set on ids because contact pointers are unstable as they move between touching and not touching.
	int contactIdCapacity = b3GetIdCapacity( &world->contactIdPool );
	for ( int i = 0; i < world->workerCount; ++i )
	{
		b3TaskContext* taskContext = world->taskContexts.data + i;
		b3SetBitCountAndClear( &taskContext->contactStateBitSet, contactIdCapacity );
		taskContext->satCallCount = 0;
		taskContext->satCacheHitCount = 0;
		taskContext->recycledContactCount = 0;
		memset( taskContext->manifoldCounts, 0, sizeof( taskContext->manifoldCounts ) );
	}

	// Task should take at least 40us on a 4GHz CPU (10K cycles)
	int minRange = 20;
	b3ParallelFor( world, b3CollideTask, contactCount, minRange, context, "collide" );

	b3StackFree( &world->stack, contactIndices );
	context->awakeContactIndices = NULL;
	contactIndices = NULL;

	// Serially update contact state
	// todo bring this zone together with island merge
	b3TracyCZoneNC( contact_state, "Contact State", b3_colorLightSlateGray, true );

	int satMultiplier = context->dt > 0.0f ? 1 : 0;

	// Bitwise OR all contact bits
	b3BitSet* bitSet = &world->taskContexts.data[0].contactStateBitSet;
	world->satCallCount = satMultiplier * world->taskContexts.data[0].satCallCount;
	world->satCacheHitCount = satMultiplier * world->taskContexts.data[0].satCacheHitCount;
	memcpy( world->manifoldCounts, world->taskContexts.data[0].manifoldCounts,
			B3_CONTACT_MANIFOLD_COUNT_BUCKETS * sizeof( int ) );

	for ( int i = 1; i < world->workerCount; ++i )
	{
		b3InPlaceUnion( bitSet, &world->taskContexts.data[i].contactStateBitSet );
		world->satCallCount += world->taskContexts.data[i].satCallCount;
		world->satCacheHitCount += world->taskContexts.data[i].satCacheHitCount;
		for ( int j = 0; j < B3_CONTACT_MANIFOLD_COUNT_BUCKETS; ++j )
		{
			world->manifoldCounts[j] += world->taskContexts.data[i].manifoldCounts[j];
		}
	}

	// Release per-step overflow blocks and grow the backing capacity if last
	// step's demand exceeded it. Contact processing is the only consumer of
	// these arenas and is finished by this point.
	for ( int i = 0; i < world->workerCount; ++i )
	{
		b3ArenaSync( &world->taskContexts.data[i].arena );
	}

	int endEventArrayIndex = world->endEventArrayIndex;

	const b3Shape* shapes = world->shapes.data;
	uint16_t worldId = world->worldId;

	// Process contact state changes. Iterate over set bits
	for ( uint32_t k = 0; k < bitSet->blockCount; ++k )
	{
		uint64_t bits = bitSet->bits[k];
		while ( bits != 0 )
		{
			uint32_t ctz = b3CTZ64( bits );
			int contactId = (int)( 64 * k + ctz );

			b3Contact* contact = b3Array_Get( world->contacts, contactId );
			B3_ASSERT( contact->setIndex == b3_awakeSet );

			const b3Shape* shapeA = shapes + contact->shapeIdA;
			const b3Shape* shapeB = shapes + contact->shapeIdB;
			b3ShapeId shapeIdA = { shapeA->id + 1, worldId, shapeA->generation };
			b3ShapeId shapeIdB = { shapeB->id + 1, worldId, shapeB->generation };
			b3ContactId contactFullId = {
				.index1 = contactId + 1,
				.world0 = worldId,
				.padding = 0,
				.generation = contact->generation,
			};
			uint32_t flags = contact->flags;

			if ( flags & b3_simDisjoint )
			{
				// Bounding boxes no longer overlap
				b3DestroyContact( world, contact, false );
				contact = NULL;
			}
			else if ( flags & b3_simStartedTouching )
			{
				B3_ASSERT( contact->islandId == B3_NULL_INDEX );

				if ( flags & b3_contactEnableContactEvents )
				{
					b3ContactBeginTouchEvent event = { shapeIdA, shapeIdB, contactFullId };
					b3Array_Push( world->contactBeginEvents, event );
				}

				B3_ASSERT( contact->manifoldCount > 0 );
				B3_ASSERT( contact->setIndex == b3_awakeSet );

				// Link first because this wakes colliding bodies and ensures the body sims
				// are in the correct place.
				contact->flags &= ~b3_simStartedTouching;
				contact->flags |= b3_contactTouchingFlag;
				b3LinkContact( world, contact );

				// Make sure these didn't change
				B3_ASSERT( contact->colorIndex == B3_NULL_INDEX );

				// Contact sim pointer may have become orphaned due to awake set growth,
				// so I just need to refresh it.

				int oldLocalIndex = contact->localIndex;

				b3AddContactToGraph( world, contact );
				b3RemoveNonTouchingContact( world, b3_awakeSet, oldLocalIndex );
			}
			else if ( flags & b3_simStoppedTouching )
			{
				contact->flags &= ~b3_simStoppedTouching;
				contact->flags &= ~b3_contactTouchingFlag;

				if ( contact->flags & b3_contactEnableContactEvents )
				{
					b3ContactEndTouchEvent event = { shapeIdA, shapeIdB, contactFullId };
					b3Array_Push( world->contactEndEvents[endEventArrayIndex], event );
				}

				B3_ASSERT( contact->manifoldCount == 0 );

				// Cache these here for the remove below
				int colorIndex = contact->colorIndex;
				int localIndex = contact->localIndex;

				b3UnlinkContact( world, contact );
				int bodyIdA = contact->edges[0].bodyId;
				int bodyIdB = contact->edges[1].bodyId;

				b3AddNonTouchingContact( world, contact );

				bool isMeshContact = contact->flags & b3_simMeshContact;
				b3RemoveContactFromGraph( world, bodyIdA, bodyIdB, colorIndex, localIndex, isMeshContact );
				contact = NULL;
			}

			// Clear the smallest set bit
			bits = bits & ( bits - 1 );
		}
	}

	b3ValidateSolverSets( world );
	b3ValidateContacts( world );

	b3TracyCZoneEnd( contact_state );
	b3TracyCZoneEnd( collide );
}














































































































































































typedef struct DrawContext
{
	b3World* world;
	b3DebugDraw* draw;
} DrawContext;

static bool DrawQueryCallback( int proxyId, uint64_t userData, void* context )
{
	int shapeId = (int)userData;

	B3_UNUSED( proxyId );

	struct DrawContext* drawContext = (DrawContext*)context;
	b3World* world = drawContext->world;
	b3DebugDraw* draw = drawContext->draw;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );
	B3_ASSERT( shape->id == shapeId );

	b3SetBit( &world->debugBodySet, shape->bodyId );

	if ( draw->drawShapes )
	{
		b3Body* body = b3Array_Get( world->bodies, shape->bodyId );
		b3BodySim* bodySim = b3GetBodySim( world, body );

		b3HexColor color;

		const b3SurfaceMaterial* surfaceMaterial = b3GetShapeMaterials( shape );
		if ( surfaceMaterial[0].customColor != 0 )
		{
			// May already carry a packed material preset, pass through unchanged
			color = (b3HexColor)surfaceMaterial[0].customColor;
		}
		else
		{
			// Hue carries the state, material carries its energy. Calm matte for the
			// resting masses, glossy for fast bodies, metallic for the driven kinematic.
			// Diagnostic states keep a saturated hue and the default material so they pop.
			b3HexColor rgb;
			b3DebugMaterial material = b3_debugMaterialDefault;

			if ( body->type == b3_dynamicBody && body->mass == 0.0f )
			{
				// Bad body
				rgb = b3_colorRed;
			}
			else if ( body->setIndex == b3_disabledSet )
			{
				rgb = b3_colorSlateGray;
			}
			else if ( shape->sensorIndex != B3_NULL_INDEX )
			{
				rgb = b3_colorWheat;
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

			color = (b3HexColor)b3MakeDebugColor( rgb, material );
		}

		if ( shape->userShape == NULL && world->createDebugShape != NULL )
		{
			b3DebugShape debugShape = { 0 };
			debugShape.shapeId = (b3ShapeId){
				.world0 = world->worldId,
				.index1 = shapeId + 1,
				.generation = shape->generation,
			};
			debugShape.type = shape->type;

			switch ( shape->type )
			{
				case b3_capsuleShape:
					debugShape.capsule = &shape->capsule;
					shape->userShape = world->createDebugShape( &debugShape, world->userDebugShapeContext );
					break;
				case b3_compoundShape:
					debugShape.compound = shape->compound;
					shape->userShape = world->createDebugShape( &debugShape, world->userDebugShapeContext );
					break;
				case b3_heightShape:
					debugShape.heightField = shape->heightField;
					shape->userShape = world->createDebugShape( &debugShape, world->userDebugShapeContext );
					break;
				case b3_hullShape:
					debugShape.hull = shape->hull;
					shape->userShape = world->createDebugShape( &debugShape, world->userDebugShapeContext );
					break;
				case b3_meshShape:
					debugShape.mesh = &shape->mesh;
					shape->userShape = world->createDebugShape( &debugShape, world->userDebugShapeContext );
					break;
				case b3_sphereShape:
					debugShape.sphere = &shape->sphere;
					shape->userShape = world->createDebugShape( &debugShape, world->userDebugShapeContext );
					break;
				default:
					B3_ASSERT( false );
					break;
			}
		}

		if ( shape->userShape != NULL )
		{
			draw->DrawShapeFcn( shape->userShape, bodySim->transform, color, draw->context );
		}
	}

	if ( draw->drawBounds )
	{
		draw->DrawBoundsFcn( shape->fatAABB, b3_colorGold, draw->context );
	}

	return true;
}
















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































typedef struct WorldQueryContext
{
	b3World* world;
	b3OverlapResultFcn* fcn;
	b3QueryFilter filter;
	void* userContext;
} WorldQueryContext;

static bool TreeQueryCallback( int proxyId, uint64_t userData, void* context )
{
	B3_UNUSED( proxyId );

	int shapeId = (int)userData;
	WorldQueryContext* worldContext = (WorldQueryContext*)context;
	b3World* world = worldContext->world;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );

	b3Filter shapeFilter = shape->filter;
	b3QueryFilter queryFilter = worldContext->filter;

	if ( b3ShouldQueryCollide( &shapeFilter, &queryFilter ) == false )
	{
		return true;
	}

	b3ShapeId id = { shapeId + 1, world->worldId, shape->generation };
	bool result = worldContext->fcn( id, worldContext->userContext );
	return result;
}















































typedef struct WorldOverlapContext
{
	b3World* world;
	b3OverlapResultFcn* fcn;
	b3QueryFilter filter;
	b3ShapeProxy proxy;
	b3Pos origin;
	void* userContext;
} WorldOverlapContext;

static bool b3TreeOverlapCallback( int proxyId, uint64_t userData, void* context )
{
	B3_UNUSED( proxyId );

	int shapeId = (int)userData;
	WorldOverlapContext* worldContext = (WorldOverlapContext*)context;
	b3World* world = worldContext->world;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );

	b3Filter shapeFilter = shape->filter;
	b3QueryFilter queryFilter = worldContext->filter;

	if ( b3ShouldQueryCollide( &shapeFilter, &queryFilter ) == false )
	{
		return true;
	}

	// Re-center on the query origin so the overlap test stays in float precision far from the origin
	b3Body* body = b3Array_Get( world->bodies, shape->bodyId );
	b3Transform transform = b3ToRelativeTransform( b3GetBodyTransformQuick( world, body ), worldContext->origin );

	bool overlapping = b3OverlapShape( shape, transform, &worldContext->proxy );
	if ( overlapping == false )
	{
		return true;
	}

	b3ShapeId id = { shape->id + 1, world->worldId, shape->generation };
	bool result = worldContext->fcn( id, worldContext->userContext );
	return result;
}





















































typedef struct WorldMoverContext
{
	b3World* world;
	b3PlaneResultFcn* fcn;
	b3QueryFilter filter;
	b3Capsule mover;
	b3Pos origin;
	void* userContext;
} WorldMoverContext;

static bool TreeCollideCallback( int proxyId, uint64_t userData, void* context )
{
	B3_UNUSED( proxyId );

	int shapeId = (int)userData;
	WorldMoverContext* worldContext = (WorldMoverContext*)context;
	b3World* world = worldContext->world;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );

	b3Filter shapeFilter = shape->filter;
	b3QueryFilter queryFilter = worldContext->filter;

	if ( b3ShouldQueryCollide( &shapeFilter, &queryFilter ) == false )
	{
		return true;
	}

	// Re-center on the query origin, the mover and the resulting planes are origin relative
	b3Body* body = b3Array_Get( world->bodies, shape->bodyId );
	b3WorldTransform bodyTransform = b3GetBodyTransformQuick( world, body );
	b3Transform transform = b3ToRelativeTransform( bodyTransform, worldContext->origin );

	b3PlaneResult buffer[64];
	int count = b3CollideMover( buffer, 64, shape, transform, &worldContext->mover );

	if ( count > 0 )
	{
		b3ShapeId id = { shape->id + 1, world->worldId, shape->generation };
		return worldContext->fcn( id, buffer, count, worldContext->userContext );
	}

	return true;
}

// It is tempting to use a shape proxy for the mover, but this makes handling deep overlap difficult and the generality may
// not be worth it.


















































typedef struct WorldRayCastContext
{
	b3World* world;
	b3CastResultFcn* fcn;
	b3QueryFilter filter;
	float fraction;
	b3Pos origin;
	void* userContext;
} WorldRayCastContext;

static float RayCastCallback( const b3RayCastInput* input, int proxyId, uint64_t userData, void* context )
{
	B3_UNUSED( proxyId );

	int shapeId = (int)userData;
	WorldRayCastContext* worldContext = (WorldRayCastContext*)context;
	b3World* world = worldContext->world;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );
	b3Filter shapeFilter = shape->filter;
	b3QueryFilter queryFilter = worldContext->filter;

	if ( b3ShouldQueryCollide( &shapeFilter, &queryFilter ) == false )
	{
		return input->maxFraction;
	}

	b3Body* body = b3Array_Get( world->bodies, shape->bodyId );
	b3WorldTransform bodyTransform = b3GetBodyTransformQuick( world, body );
	b3Transform transform = b3ToRelativeTransform( bodyTransform, worldContext->origin );

	b3RayCastInput localInput = *input;
	localInput.origin = b3Vec3_zero;
	b3CastOutput output = b3RayCastShape( shape, transform, &localInput );

	if ( output.hit )
	{
		B3_ASSERT( output.fraction <= input->maxFraction );

		b3ShapeId id = { shapeId + 1, world->worldId, shape->generation };
		b3Pos point = b3OffsetPos( worldContext->origin, output.point );
		int materialIndex = b3ClampInt( output.materialIndex, 0, shape->materialCount - 1 );
		uint64_t userMaterialId = b3GetShapeMaterials( shape )[materialIndex].userMaterialId;

		int triangleIndex = output.triangleIndex;
		int childIndex = output.childIndex;
		float fraction = worldContext->fcn( id, point, output.normal, output.fraction, userMaterialId, triangleIndex, childIndex,
											worldContext->userContext );

		// The user may return -1 to skip this shape
		if ( 0.0f <= fraction && fraction <= 1.0f )
		{
			worldContext->fraction = fraction;
		}

		return fraction;
	}

	return input->maxFraction;
}




























































// This callback finds the closest hit. This is the most common callback used in games.
static float b3RayCastClosestFcn( b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t userMaterialId,
								  int triangleIndex, int childIndex, void* context )
{
	// Ignore initial overlap
	if ( fraction == 0.0f )
	{
		return -1.0f;
	}

	b3RayResult* rayResult = (b3RayResult*)context;
	rayResult->shapeId = shapeId;
	rayResult->point = point;
	rayResult->normal = normal;
	rayResult->fraction = fraction;
	rayResult->userMaterialId = userMaterialId;
	rayResult->triangleIndex = triangleIndex;
	rayResult->childIndex = childIndex;
	rayResult->hit = true;
	return fraction;
}

























































typedef struct WorldShapeCastContext
{
	b3World* world;
	b3CastResultFcn* fcn;
	b3QueryFilter filter;
	float fraction;
	b3Pos origin;
	// origin relative input
	b3ShapeCastInput input;
	void* userContext;
} WorldShapeCastContext;

static float b3ShapeCastCallback( const b3BoxCastInput* input, int proxyId, uint64_t userData, void* context )
{
	B3_UNUSED( proxyId );

	int shapeId = (int)userData;
	WorldShapeCastContext* worldContext = (WorldShapeCastContext*)context;
	b3World* world = worldContext->world;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );
	b3Filter shapeFilter = shape->filter;
	b3QueryFilter queryFilter = worldContext->filter;

	if ( b3ShouldQueryCollide( &shapeFilter, &queryFilter ) == false )
	{
		return input->maxFraction;
	}

	// Rebuild from the origin relative input, taking only the advancing fraction from the tree.
	// The tree box is world float and would lose the cast far from the origin.
	b3ShapeCastInput localInput = worldContext->input;
	localInput.maxFraction = input->maxFraction;

	// Re-center on the query origin so the per-shape cast stays in float precision far from the origin
	b3Body* body = b3Array_Get( world->bodies, shape->bodyId );
	b3Transform transform = b3ToRelativeTransform( b3GetBodyTransformQuick( world, body ), worldContext->origin );

	b3CastOutput output = b3ShapeCastShape( shape, transform, &localInput );

	if ( output.hit )
	{
		b3ShapeId id = { shapeId + 1, world->worldId, shape->generation };
		int materialIndex = b3ClampInt( output.materialIndex, 0, shape->materialCount - 1 );
		uint64_t userMaterialId = b3GetShapeMaterials( shape )[materialIndex].userMaterialId;

		int triangleIndex = output.triangleIndex;
		int childIndex = output.childIndex;
		float fraction = worldContext->fcn( id, b3OffsetPos( worldContext->origin, output.point ), output.normal, output.fraction,
											userMaterialId, triangleIndex, childIndex, worldContext->userContext );

		// The user may return -1 to skip this shape
		if ( 0.0f <= fraction && fraction <= 1.0f )
		{
			worldContext->fraction = fraction;
		}

		return fraction;
	}

	return input->maxFraction;
}









































































typedef struct WorldMoverCastContext
{
	b3World* world;
	b3MoverFilterFcn* fcn;
	b3QueryFilter filter;
	float fraction;
	b3Pos origin;
	// origin relative input
	b3ShapeCastInput input;
	void* userContext;
} WorldMoverCastContext;

static float MoverCastCallback( const b3BoxCastInput* input, int proxyId, uint64_t userData, void* context )
{
	B3_UNUSED( proxyId );

	int shapeId = (int)userData;
	WorldMoverCastContext* worldContext = (WorldMoverCastContext*)context;
	b3World* world = worldContext->world;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );
	b3Filter shapeFilter = shape->filter;
	b3QueryFilter queryFilter = worldContext->filter;

	if ( b3ShouldQueryCollide( &shapeFilter, &queryFilter ) == false )
	{
		return worldContext->fraction;
	}

	if ( worldContext->fcn != NULL )
	{
		b3ShapeId id = { shapeId + 1, world->worldId, shape->generation };
		bool shouldCollide = worldContext->fcn( id, worldContext->userContext );
		if ( shouldCollide == false )
		{
			return worldContext->fraction;
		}
	}

	// Rebuild from the origin relative input, taking only the advancing fraction from the tree
	b3ShapeCastInput localInput = worldContext->input;
	localInput.maxFraction = input->maxFraction;

	// Re-center on the query origin so the per-shape cast stays in float precision far from the origin
	b3Body* body = b3Array_Get( world->bodies, shape->bodyId );
	b3Transform transform = b3ToRelativeTransform( b3GetBodyTransformQuick( world, body ), worldContext->origin );

	b3CastOutput output = b3ShapeCastShape( shape, transform, &localInput );
	if ( output.fraction == 0.0f )
	{
		// Ignore overlapping shapes
		return worldContext->fraction;
	}

	worldContext->fraction = output.fraction;
	return output.fraction;
}














































































































typedef struct ExplosionContext
{
	b3World* world;
	b3Pos position;
	float radius;
	float falloff;
	float impulsePerArea;
} ExplosionContext;

static bool ExplosionCallback( int proxyId, uint64_t userData, void* context )
{
	B3_UNUSED( proxyId );

	int shapeId = (int)userData;
	ExplosionContext* explosionContext = (ExplosionContext*)context;
	b3World* world = explosionContext->world;

	b3Shape* shape = b3Array_Get( world->shapes, shapeId );
	if ( shape->explosionScale == 0.0f )
	{
		return true;
	}

	b3Body* body = b3Array_Get( world->bodies, shape->bodyId );
	B3_ASSERT( body->type == b3_dynamicBody );

	b3WorldTransform xf = b3GetBodyTransformQuick( world, body );

	// Re-center the explosion into the shape local frame so distance and direction stay precise
	// far from the origin. Everything below runs in that near-origin frame.
	b3Vec3 localPosition = b3InvTransformWorldPoint( xf, explosionContext->position );

	b3DistanceInput input;
	input.proxyA = b3MakeShapeProxy( shape );
	input.proxyB = (b3ShapeProxy){ &localPosition, 1, 0.0f };
	input.transform = b3Transform_identity;
	input.useRadii = true;

	b3SimplexCache cache = { 0 };
	b3DistanceOutput output = b3ShapeDistance( &input, &cache, NULL, 0 );

	float radius = explosionContext->radius;
	float falloff = explosionContext->falloff;
	if ( output.distance > radius + falloff )
	{
		return true;
	}

	b3WakeBody( world, body );

	if ( body->setIndex != b3_awakeSet )
	{
		return true;
	}

	// Witness point is already in the body local query frame
	b3Vec3 closestPoint = output.pointA;
	if ( output.distance == 0.0f )
	{
		closestPoint = b3GetShapeCentroid( shape );
	}

	b3Vec3 direction = b3Sub( closestPoint, localPosition );
	if ( b3LengthSquared( direction ) > 100.0f * FLT_EPSILON * FLT_EPSILON )
	{
		direction = b3Normalize( direction );
	}
	else
	{
		direction = (b3Vec3){ 1.0f, 0.0f, 0.0f };
	}

	float area = b3GetShapeProjectedArea( shape, direction );
	float scale = 1.0f;
	if ( output.distance > radius && falloff > 0.0f )
	{
		scale = b3ClampFloat( ( radius + falloff - output.distance ) / falloff, 0.0f, 1.0f );
	}

	float magnitude = explosionContext->impulsePerArea * area * scale * shape->explosionScale;
	b3Vec3 impulse = b3MulSV( magnitude, b3RotateVector( xf.q, direction ) );

	int localIndex = body->localIndex;
	b3SolverSet* set = b3Array_Get( world->solverSets, b3_awakeSet );
	b3BodyState* state = b3Array_Get( set->bodyStates, localIndex );
	b3BodySim* bodySim = b3Array_Get( set->bodySims, localIndex );
	state->linearVelocity = b3MulAdd( state->linearVelocity, bodySim->invMass, impulse );

	// Lever arm from the center of mass to the closest point, rotated to world
	b3Vec3 r = b3RotateVector( xf.q, b3Sub( closestPoint, bodySim->localCenter ) );
	state->angularVelocity = b3Add( state->angularVelocity, b3MulMV( bodySim->invInertiaWorld, b3Cross( r, impulse ) ) );

	return true;
}
































































#if B3_ENABLE_VALIDATION
// This validates island graph connectivity for each body
void b3ValidateConnectivity( b3World* world )
{
	b3Body* bodies = world->bodies.data;
	int bodyCapacity = world->bodies.count;

	for ( int bodyIndex = 0; bodyIndex < bodyCapacity; ++bodyIndex )
	{
		b3Body* body = bodies + bodyIndex;
		if ( body->id == B3_NULL_INDEX )
		{
			b3ValidateFreeId( &world->bodyIdPool, bodyIndex );
			continue;
		}

		B3_ASSERT( bodyIndex == body->id );

		// Need to get the root island because islands are not merged until the next time step
		int bodyIslandId = body->islandId;
		int bodySetIndex = body->setIndex;

		int contactKey = body->headContactKey;
		while ( contactKey != B3_NULL_INDEX )
		{
			int contactId = contactKey >> 1;
			int edgeIndex = contactKey & 1;

			b3Contact* contact = b3Array_Get( world->contacts, contactId );

			bool touching = ( contact->flags & b3_contactTouchingFlag ) != 0;
			if ( touching )
			{
				if ( bodySetIndex != b3_staticSet )
				{
					int contactIslandId = contact->islandId;
					B3_ASSERT( contactIslandId == bodyIslandId );
				}
			}
			else
			{
				B3_ASSERT( contact->islandId == B3_NULL_INDEX );
			}

			contactKey = contact->edges[edgeIndex].nextKey;
		}

		int jointKey = body->headJointKey;
		while ( jointKey != B3_NULL_INDEX )
		{
			int jointId = jointKey >> 1;
			int edgeIndex = jointKey & 1;

			b3Joint* joint = b3Array_Get( world->joints, jointId );

			int otherEdgeIndex = edgeIndex ^ 1;

			b3Body* otherBody = b3Array_Get( world->bodies, joint->edges[otherEdgeIndex].bodyId );

			if ( bodySetIndex == b3_disabledSet || otherBody->setIndex == b3_disabledSet )
			{
				B3_ASSERT( joint->islandId == B3_NULL_INDEX );
			}
			else if ( bodySetIndex == b3_staticSet )
			{
				// Intentional nesting
				if ( otherBody->setIndex == b3_staticSet )
				{
					B3_ASSERT( joint->islandId == B3_NULL_INDEX );
				}
			}
			else if ( body->type != b3_dynamicBody && otherBody->type != b3_dynamicBody )
			{
				B3_ASSERT( joint->islandId == B3_NULL_INDEX );
			}
			else
			{
				int jointIslandId = joint->islandId;
				B3_ASSERT( jointIslandId == bodyIslandId );
			}

			jointKey = joint->edges[edgeIndex].nextKey;
		}
	}
}

// Validates solver sets, but not island connectivity
void b3ValidateSolverSets( b3World* world )
{
	B3_ASSERT( b3GetIdCapacity( &world->bodyIdPool ) == world->bodies.count );
	B3_ASSERT( b3GetIdCapacity( &world->contactIdPool ) == world->contacts.count );
	B3_ASSERT( b3GetIdCapacity( &world->jointIdPool ) == world->joints.count );
	B3_ASSERT( b3GetIdCapacity( &world->islandIdPool ) == world->islands.count );
	B3_ASSERT( b3GetIdCapacity( &world->solverSetIdPool ) == world->solverSets.count );

	int activeSetCount = 0;
	int totalBodyCount = 0;
	int totalJointCount = 0;
	int totalContactCount = 0;
	int totalIslandCount = 0;

	// Validate all solver sets
	int setCount = world->solverSets.count;
	for ( int setIndex = 0; setIndex < setCount; ++setIndex )
	{
		b3SolverSet* set = world->solverSets.data + setIndex;
		if ( set->setIndex != B3_NULL_INDEX )
		{
			activeSetCount += 1;

			if ( setIndex == b3_staticSet )
			{
				B3_ASSERT( set->contactIndices.count == 0 );
				B3_ASSERT( set->islandSims.count == 0 );
				B3_ASSERT( set->bodyStates.count == 0 );
			}
			else if ( setIndex == b3_disabledSet )
			{
				B3_ASSERT( set->islandSims.count == 0 );
				B3_ASSERT( set->bodyStates.count == 0 );
			}
			else if ( setIndex == b3_awakeSet )
			{
				B3_ASSERT( set->bodySims.count == set->bodyStates.count );
				B3_ASSERT( set->jointSims.count == 0 );
			}
			else
			{
				B3_ASSERT( set->bodyStates.count == 0 );
			}

			// Validate bodies
			{
				b3Body* bodies = world->bodies.data;
				B3_ASSERT( set->bodySims.count >= 0 );
				totalBodyCount += set->bodySims.count;
				for ( int i = 0; i < set->bodySims.count; ++i )
				{
					b3BodySim* bodySim = set->bodySims.data + i;

					int bodyId = bodySim->bodyId;
					B3_ASSERT( 0 <= bodyId && bodyId < world->bodies.count );
					b3Body* body = bodies + bodyId;
					B3_ASSERT( body->setIndex == setIndex );
					B3_ASSERT( body->localIndex == i );

					uint32_t syncedFlags = body->flags & ~b3_bodyTransientFlags;
					B3_ASSERT( ( bodySim->flags & syncedFlags ) == syncedFlags );

					b3BodyState* bodyState = b3GetBodyState( world, body );
					if ( bodyState != NULL )
					{
						B3_ASSERT( ( bodyState->flags & syncedFlags ) == syncedFlags );
					}

					if ( body->type == b3_dynamicBody )
					{
						B3_ASSERT( body->flags & b3_dynamicFlag );
					}

					if ( setIndex == b3_disabledSet )
					{
						B3_ASSERT( body->headContactKey == B3_NULL_INDEX );
					}

					// Validate body shapes
					int prevShapeId = B3_NULL_INDEX;
					int shapeId = body->headShapeId;
					while ( shapeId != B3_NULL_INDEX )
					{
						b3Shape* shape = b3Array_Get( world->shapes, shapeId );
						B3_ASSERT( shape->id == shapeId );
						B3_ASSERT( shape->prevShapeId == prevShapeId );

						if ( setIndex == b3_disabledSet )
						{
							B3_ASSERT( shape->proxyKey == B3_NULL_INDEX );
						}
						else if ( setIndex == b3_staticSet )
						{
							B3_ASSERT( B3_PROXY_TYPE( shape->proxyKey ) == b3_staticBody );
						}
						else
						{
							b3BodyType proxyType = B3_PROXY_TYPE( shape->proxyKey );
							B3_ASSERT( proxyType == b3_kinematicBody || proxyType == b3_dynamicBody );
						}

						prevShapeId = shapeId;
						shapeId = shape->nextShapeId;
					}

					// Validate body contacts
					int contactKey = body->headContactKey;
					while ( contactKey != B3_NULL_INDEX )
					{
						int contactId = contactKey >> 1;
						int edgeIndex = contactKey & 1;

						b3Contact* contact = b3Array_Get( world->contacts, contactId );
						B3_ASSERT( contact->setIndex != b3_staticSet );
						B3_ASSERT( contact->edges[0].bodyId == bodyId || contact->edges[1].bodyId == bodyId );
						contactKey = contact->edges[edgeIndex].nextKey;
					}

					// Validate body joints
					int jointKey = body->headJointKey;
					while ( jointKey != B3_NULL_INDEX )
					{
						int jointId = jointKey >> 1;
						int edgeIndex = jointKey & 1;

						b3Joint* joint = b3Array_Get( world->joints, jointId );

						int otherEdgeIndex = edgeIndex ^ 1;

						b3Body* otherBody = b3Array_Get( world->bodies, joint->edges[otherEdgeIndex].bodyId );

						if ( setIndex == b3_disabledSet || otherBody->setIndex == b3_disabledSet )
						{
							B3_ASSERT( joint->setIndex == b3_disabledSet );
						}
						else if ( setIndex == b3_staticSet && otherBody->setIndex == b3_staticSet )
						{
							B3_ASSERT( joint->setIndex == b3_staticSet );
						}
						else if ( body->type != b3_dynamicBody && otherBody->type != b3_dynamicBody )
						{
							B3_ASSERT( joint->setIndex == b3_staticSet );
						}
						else if ( setIndex == b3_awakeSet )
						{
							B3_ASSERT( joint->setIndex == b3_awakeSet );
						}
						else if ( setIndex >= b3_firstSleepingSet )
						{
							B3_ASSERT( joint->setIndex == setIndex );
						}

						b3JointSim* jointSim = b3GetJointSim( world, joint );
						B3_ASSERT( jointSim->jointId == jointId );
						B3_ASSERT( jointSim->bodyIdA == joint->edges[0].bodyId );
						B3_ASSERT( jointSim->bodyIdB == joint->edges[1].bodyId );

						jointKey = joint->edges[edgeIndex].nextKey;
					}
				}
			}

			// Validate contacts
			{
				B3_ASSERT( set->contactIndices.count >= 0 );
				totalContactCount += set->contactIndices.count;
				for ( int i = 0; i < set->contactIndices.count; ++i )
				{
					int contactIndex = set->contactIndices.data[i];
					b3Contact* contact = b3Array_Get( world->contacts, contactIndex );
					if ( setIndex == b3_awakeSet )
					{
						// contact should be non-touching if awake
						// or it could be this contact hasn't been transferred yet
						B3_ASSERT( contact->manifoldCount == 0 || ( contact->flags & b3_simStartedTouching ) != 0 );
					}
					B3_ASSERT( contact->setIndex == setIndex );
					B3_ASSERT( contact->colorIndex == B3_NULL_INDEX );
					B3_ASSERT( contact->localIndex == i );
				}
			}

			// Validate joints
			{
				B3_ASSERT( set->jointSims.count >= 0 );
				totalJointCount += set->jointSims.count;
				for ( int i = 0; i < set->jointSims.count; ++i )
				{
					b3JointSim* jointSim = set->jointSims.data + i;
					b3Joint* joint = b3Array_Get( world->joints, jointSim->jointId );
					B3_ASSERT( joint->setIndex == setIndex );
					B3_ASSERT( joint->colorIndex == B3_NULL_INDEX );
					B3_ASSERT( joint->localIndex == i );
				}
			}

			// Validate islands
			{
				B3_ASSERT( set->islandSims.count >= 0 );
				totalIslandCount += set->islandSims.count;
				for ( int i = 0; i < set->islandSims.count; ++i )
				{
					b3IslandSim* islandSim = set->islandSims.data + i;
					b3Island* island = b3Array_Get( world->islands, islandSim->islandId );
					B3_ASSERT( island->setIndex == setIndex );
					B3_ASSERT( island->localIndex == i );
				}
			}
		}
		else
		{
			B3_ASSERT( set->bodySims.count == 0 );
			B3_ASSERT( set->contactIndices.count == 0 );
			B3_ASSERT( set->jointSims.count == 0 );
			B3_ASSERT( set->islandSims.count == 0 );
			B3_ASSERT( set->bodyStates.count == 0 );
		}
	}

	int setIdCount = b3GetIdCount( &world->solverSetIdPool );
	B3_ASSERT( activeSetCount == setIdCount );

	int bodyIdCount = b3GetIdCount( &world->bodyIdPool );
	B3_ASSERT( totalBodyCount == bodyIdCount );

	int islandIdCount = b3GetIdCount( &world->islandIdPool );
	B3_ASSERT( totalIslandCount == islandIdCount );

	// Validate constraint graph
	for ( int colorIndex = 0; colorIndex < B3_GRAPH_COLOR_COUNT; ++colorIndex )
	{
		b3GraphColor* color = world->constraintGraph.colors + colorIndex;
		int bitCount = 0;

		B3_ASSERT( color->convexContacts.count >= 0 );
		totalContactCount += color->convexContacts.count;
		for ( int i = 0; i < color->convexContacts.count; ++i )
		{
			int contactId = color->convexContacts.data[i];
			b3Contact* contact = b3Array_Get( world->contacts, contactId );
			// contact should be touching in the constraint graph or awaiting transfer to non-touching
			B3_ASSERT( contact->manifoldCount > 0 || ( contact->flags & ( b3_simStoppedTouching | b3_simDisjoint ) ) != 0 );
			B3_ASSERT( contact->setIndex == b3_awakeSet );
			B3_ASSERT( contact->colorIndex == colorIndex );
			B3_ASSERT( contact->localIndex == i );

			int bodyIdA = contact->edges[0].bodyId;
			int bodyIdB = contact->edges[1].bodyId;

			if ( colorIndex < B3_OVERFLOW_INDEX )
			{
				b3Body* bodyA = b3Array_Get( world->bodies, bodyIdA );
				b3Body* bodyB = b3Array_Get( world->bodies, bodyIdB );
				B3_ASSERT( b3GetBit( &color->bodySet, bodyIdA ) == ( bodyA->type == b3_dynamicBody ) );
				B3_ASSERT( b3GetBit( &color->bodySet, bodyIdB ) == ( bodyB->type == b3_dynamicBody ) );

				bitCount += bodyA->type == b3_dynamicBody ? 1 : 0;
				bitCount += bodyB->type == b3_dynamicBody ? 1 : 0;
			}
		}

		totalContactCount += color->contacts.count;
		for ( int i = 0; i < color->contacts.count; ++i )
		{
			int contactId = color->contacts.data[i].contactId;
			b3Contact* contact = b3Array_Get( world->contacts, contactId );
			// contact should be touching in the constraint graph or awaiting transfer to non-touching
			B3_ASSERT( contact->manifoldCount > 0 || ( contact->flags & ( b3_simStoppedTouching | b3_simDisjoint ) ) != 0 );
			B3_ASSERT( contact->setIndex == b3_awakeSet );
			B3_ASSERT( contact->colorIndex == colorIndex );
			B3_ASSERT( contact->localIndex == i );

			int bodyIdA = contact->edges[0].bodyId;
			int bodyIdB = contact->edges[1].bodyId;

			if ( colorIndex < B3_OVERFLOW_INDEX )
			{
				b3Body* bodyA = b3Array_Get( world->bodies, bodyIdA );
				b3Body* bodyB = b3Array_Get( world->bodies, bodyIdB );
				B3_ASSERT( b3GetBit( &color->bodySet, bodyIdA ) == ( bodyA->type == b3_dynamicBody ) );
				B3_ASSERT( b3GetBit( &color->bodySet, bodyIdB ) == ( bodyB->type == b3_dynamicBody ) );

				bitCount += bodyA->type == b3_dynamicBody ? 1 : 0;
				bitCount += bodyB->type == b3_dynamicBody ? 1 : 0;
			}
		}

		B3_ASSERT( color->jointSims.count >= 0 );
		totalJointCount += color->jointSims.count;
		for ( int i = 0; i < color->jointSims.count; ++i )
		{
			b3JointSim* jointSim = color->jointSims.data + i;
			b3Joint* joint = b3Array_Get( world->joints, jointSim->jointId );
			B3_ASSERT( joint->setIndex == b3_awakeSet );
			B3_ASSERT( joint->colorIndex == colorIndex );
			B3_ASSERT( joint->localIndex == i );

			int bodyIdA = joint->edges[0].bodyId;
			int bodyIdB = joint->edges[1].bodyId;

			if ( colorIndex < B3_OVERFLOW_INDEX )
			{
				b3Body* bodyA = b3Array_Get( world->bodies, bodyIdA );
				b3Body* bodyB = b3Array_Get( world->bodies, bodyIdB );
				B3_ASSERT( b3GetBit( &color->bodySet, bodyIdA ) == ( bodyA->type == b3_dynamicBody ) );
				B3_ASSERT( b3GetBit( &color->bodySet, bodyIdB ) == ( bodyB->type == b3_dynamicBody ) );

				bitCount += bodyA->type == b3_dynamicBody ? 1 : 0;
				bitCount += bodyB->type == b3_dynamicBody ? 1 : 0;
			}
		}

		// Validate the bit population for this graph color
		B3_ASSERT( bitCount == b3CountSetBits( &color->bodySet ) );
	}

	int contactIdCount = b3GetIdCount( &world->contactIdPool );
	B3_ASSERT( totalContactCount == contactIdCount );
	B3_ASSERT( totalContactCount == (int)world->broadPhase.pairSet.count );

	int jointIdCount = b3GetIdCount( &world->jointIdPool );
	B3_ASSERT( totalJointCount == jointIdCount );

// Validate shapes
// This is very slow on compounds
#if 0
	int shapeCapacity = b3Array(world->shapeArray).count;
	for (int shapeIndex = 0; shapeIndex < shapeCapacity; shapeIndex += 1)
	{
		b3Shape* shape = world->shapeArray + shapeIndex;
		if (shape->id != shapeIndex)
		{
			continue;
		}

		B3_ASSERT(0 <= shape->bodyId && shape->bodyId < b3Array(world->bodyArray).count);

		b3Body* body = world->bodyArray + shape->bodyId;
		B3_ASSERT(0 <= body->setIndex && body->setIndex < b3Array(world->solverSetArray).count);

		b3SolverSet* set = world->solverSetArray + body->setIndex;
		B3_ASSERT(0 <= body->localIndex && body->localIndex < set->sims.count);

		b3BodySim* bodySim = set->sims.mData + body->localIndex;
		B3_ASSERT(bodySim->bodyId == shape->bodyId);

		bool found = false;
		int shapeCount = 0;
		int index = body->headShapeId;
		while (index != B3_NULL_INDEX)
		{
			b3CheckId(world->shapeArray, index);
			b3Shape* s = world->shapeArray + index;
			if (index == shapeIndex)
			{
				found = true;
			}

			index = s->nextShapeId;
			shapeCount += 1;
		}

		B3_ASSERT(found);
		B3_ASSERT(shapeCount == body->shapeCount);
	}
#endif
}

// Validate contact touching status.
void b3ValidateContacts( b3World* world )
{
	b3ConstraintGraph* graph = &world->constraintGraph;
	int contactCount = world->contacts.count;
	B3_ASSERT( contactCount == b3GetIdCapacity( &world->contactIdPool ) );
	int allocatedContactCount = 0;

	for ( int contactIndex = 0; contactIndex < contactCount; ++contactIndex )
	{
		b3Contact* contact = b3Array_Get( world->contacts, contactIndex );
		if ( contact->contactId == B3_NULL_INDEX )
		{
			continue;
		}

		B3_ASSERT( contact->contactId == contactIndex );

		allocatedContactCount += 1;

		bool touching = ( contact->flags & b3_contactTouchingFlag ) != 0;

		int setId = contact->setIndex;
		b3SolverSet* set = b3Array_Get( world->solverSets, setId );

		if ( setId == b3_awakeSet )
		{
			if ( touching )
			{
				B3_ASSERT( 0 <= contact->colorIndex && contact->colorIndex < B3_GRAPH_COLOR_COUNT );
				// Validate body sim indices
				b3Shape* shapeA = b3Array_Get( world->shapes, contact->shapeIdA );
				b3Shape* shapeB = b3Array_Get( world->shapes, contact->shapeIdB );

				b3Body* bodyA = b3Array_Get( world->bodies, shapeA->bodyId );
				b3Body* bodyB = b3Array_Get( world->bodies, shapeB->bodyId );

				if ( bodyA->type == b3_staticBody )
				{
					B3_ASSERT( contact->bodySimIndexA == B3_NULL_INDEX );
				}
				else
				{
					B3_ASSERT( contact->bodySimIndexA == bodyA->localIndex );
				}

				if ( bodyB->type == b3_staticBody )
				{
					B3_ASSERT( contact->bodySimIndexB == B3_NULL_INDEX );
				}
				else
				{
					B3_ASSERT( contact->bodySimIndexB == bodyB->localIndex );
				}

				if ( ( contact->flags & b3_simMeshContact ) != 0 || contact->colorIndex == B3_OVERFLOW_INDEX )
				{
					b3GraphColor* color = graph->colors + contact->colorIndex;
					int contactId = b3Array_Get( color->contacts, contact->localIndex )->contactId;
					B3_ASSERT( contactId == contactIndex );
				}
				else
				{
					b3GraphColor* color = graph->colors + contact->colorIndex;
					int contactId = *b3Array_Get( color->convexContacts, contact->localIndex );
					B3_ASSERT( contactId == contactIndex );
				}
			}
			else
			{
				B3_ASSERT( contact->colorIndex == B3_NULL_INDEX );
				B3_ASSERT( contact->manifolds == NULL );
				B3_ASSERT( contact->manifoldCount == 0 );

				int* index = b3Array_Get( set->contactIndices, contact->localIndex );
				B3_ASSERT( *index == contactIndex );
			}
		}
		else if ( setId >= b3_firstSleepingSet )
		{
			// Only touching contacts allowed in a sleeping set
			B3_ASSERT( touching == true );
			B3_ASSERT( contact->manifolds != NULL );
			B3_ASSERT( contact->manifoldCount > 0 );
			int* index = b3Array_Get( set->contactIndices, contact->localIndex );
			B3_ASSERT( *index == contactIndex );
		}
		else
		{
			// Sleeping and non-touching contacts belong in the disabled set
			B3_ASSERT( touching == false && setId == b3_disabledSet );
			B3_ASSERT( contact->manifolds == NULL );
			B3_ASSERT( contact->manifoldCount == 0 );
			int* index = b3Array_Get( set->contactIndices, contact->localIndex );
			B3_ASSERT( *index == contactIndex );
		}

		if ( contact->flags & b3_simMeshContact )
		{
			int cacheCount = contact->meshContact.triangleCache.count;
			if ( cacheCount > 0 )
			{
				B3_ASSERT( contact->meshContact.triangleCache.data != NULL );
				B3_ASSERT( contact->meshContact.triangleCache.capacity >= cacheCount );

				b3Shape* shapeA = b3Array_Get( world->shapes, contact->shapeIdA );
				if ( shapeA->type == b3_meshShape )
				{
					int triangleCount = shapeA->mesh.data->triangleCount;
					for ( int i = 0; i < cacheCount; ++i )
					{
						int triangleIndex = contact->meshContact.triangleCache.data[i].triangleIndex;
						B3_ASSERT( 0 <= triangleIndex && triangleIndex < triangleCount );
					}
				}
				else if ( shapeA->type == b3_heightShape )
				{
					int triangleCount = b3GetHeightFieldTriangleCount( shapeA->heightField );
					for ( int i = 0; i < cacheCount; ++i )
					{
						int triangleIndex = contact->meshContact.triangleCache.data[i].triangleIndex;
						B3_ASSERT( 0 <= triangleIndex && triangleIndex < triangleCount );
					}
				}
				else
				{
					B3_ASSERT( shapeA->type == b3_compoundShape );
					b3ChildShape child = b3GetCompoundChild( shapeA->compound, contact->childIndex );
					B3_ASSERT( child.type == b3_meshShape );

					int triangleCount = child.mesh.data->triangleCount;
					for ( int i = 0; i < cacheCount; ++i )
					{
						int triangleIndex = contact->meshContact.triangleCache.data[i].triangleIndex;
						B3_ASSERT( 0 <= triangleIndex && triangleIndex < triangleCount );
					}
				}
			}
		}
	}

	int contactIdCount = b3GetIdCount( &world->contactIdPool );
	B3_ASSERT( allocatedContactCount == contactIdCount );
}

#else

void b3ValidateConnectivity( b3World* world )
{
	B3_UNUSED( world );
}

void b3ValidateSolverSets( b3World* world )
{
	B3_UNUSED( world );
}

void b3ValidateContacts( b3World* world )
{
	B3_UNUSED( world );
}

#endif
