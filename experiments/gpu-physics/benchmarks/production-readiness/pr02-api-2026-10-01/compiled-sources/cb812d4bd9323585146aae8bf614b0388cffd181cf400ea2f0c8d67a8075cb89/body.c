// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#include "body.h"

#include "aabb.h"
#include "contact.h"
#include "core.h"
#include "id_pool.h"
#include "island.h"
#include "joint.h"
#include "physics_world.h"
#include "recording.h"
#include "sensor.h"
#include "shape.h"
#include "solver_set.h"

// needed for dll export
#include "box3d/box3d.h"
#include "box3d/id.h"

#include <stddef.h>

// Get a validated body from a world using an id.
b3Body* b3GetBodyFullId( b3World* world, b3BodyId bodyId )
{
	B3_ASSERT( b3Body_IsValid( bodyId ) );

	// id index starts at one so that zero can represent null
	// id index starts at one so that zero can represent null
	return b3Array_Get( world->bodies, bodyId.index1 - 1 );
}

b3WorldTransform b3GetBodyTransformQuick( b3World* world, b3Body* body )
{
	b3SolverSet* set = b3Array_Get( world->solverSets, body->setIndex );
	b3BodySim* bodySim = b3Array_Get( set->bodySims, body->localIndex );
	return bodySim->transform;
}

b3WorldTransform b3GetBodyTransform( b3World* world, int bodyId )
{
	b3Body* body = b3Array_Get( world->bodies, bodyId );
	return b3GetBodyTransformQuick( world, body );
}

// Create a b3BodyId from a raw id.
b3BodyId b3MakeBodyId( b3World* world, int bodyId )
{
	b3Body* body = b3Array_Get( world->bodies, bodyId );
	return (b3BodyId){ bodyId + 1, world->worldId, body->generation };
}

b3BodySim* b3GetBodySim( b3World* world, b3Body* body )
{
	b3SolverSet* set = b3Array_Get( world->solverSets, body->setIndex );
	b3BodySim* bodySim = b3Array_Get( set->bodySims, body->localIndex );
	return bodySim;
}

b3BodyState* b3GetBodyState( b3World* world, b3Body* body )
{
	if ( body->setIndex == b3_awakeSet )
	{
		b3SolverSet* set = b3Array_Get( world->solverSets, b3_awakeSet );
		return b3Array_Get( set->bodyStates, body->localIndex );
	}

	return NULL;
}

void b3SyncBodyFlags( b3World* world, b3Body* body )
{
	// Never sync transient flags
	uint32_t flags = body->flags & ~b3_bodyTransientFlags;

	b3BodySim* bodySim = b3GetBodySim( world, body );
	bodySim->flags = flags;

	b3BodyState* bodyState = b3GetBodyState( world, body );
	if ( bodyState != NULL )
	{
		bodyState->flags = flags;
	}
}

static void b3CreateIslandForBody( b3World* world, int setIndex, b3Body* body )
{
	B3_ASSERT( body->islandId == B3_NULL_INDEX );
	B3_ASSERT( setIndex != b3_disabledSet );

	b3Island* island = b3CreateIsland( world, setIndex );
	b3Array_Push( island->bodies, body->id );
	body->islandId = island->islandId;
	body->islandIndex = 0;

	b3ValidateIsland( world, island->islandId );
}

static void b3RemoveBodyFromIsland( b3World* world, b3Body* body )
{
	if ( body->islandId == B3_NULL_INDEX )
	{
		B3_ASSERT( body->islandIndex == B3_NULL_INDEX );
		return;
	}

	int islandId = body->islandId;
	b3Island* island = b3Array_Get( world->islands, islandId );
	{
		int localIndex = body->islandIndex;
		int movedBodyId = island->bodies.data[island->bodies.count - 1];
		island->bodies.data[localIndex] = movedBodyId;
		B3_VALIDATE( world->bodies.data[movedBodyId].islandIndex == island->bodies.count - 1 );
		world->bodies.data[movedBodyId].islandIndex = localIndex;
		island->bodies.count -= 1;
	}

	if ( island->bodies.count == 0 )
	{
		// Destroy empty island
		B3_ASSERT( island->contacts.count == 0 );
		B3_ASSERT( island->joints.count == 0 );

		// Free the island
		b3DestroyIsland( world, island->islandId );
	}
	else
	{
		b3ValidateIsland( world, islandId );
	}

	body->islandId = B3_NULL_INDEX;
	body->islandIndex = B3_NULL_INDEX;
}

static void b3DestroyBodyContacts( b3World* world, b3Body* body, bool wakeBodies )
{
	// Destroy the attached contacts
	int edgeKey = body->headContactKey;
	while ( edgeKey != B3_NULL_INDEX )
	{
		int contactId = edgeKey >> 1;
		int edgeIndex = edgeKey & 1;

		b3Contact* contact = b3Array_Get( world->contacts, contactId );
		edgeKey = contact->edges[edgeIndex].nextKey;
		b3DestroyContact( world, contact, wakeBodies );
	}

	b3ValidateSolverSets( world );
}




























































































































































bool b3IsBodyAwake( b3World* world, b3Body* body )
{
	B3_UNUSED( world );
	return body->setIndex == b3_awakeSet;
}

bool b3WakeBody( b3World* world, b3Body* body )
{
	if ( body->setIndex >= b3_firstSleepingSet )
	{
		b3WakeSolverSet( world, body->setIndex );
		b3ValidateSolverSets( world );
		return true;
	}

	return false;
}

bool b3WakeBodyWithLock( b3World* world, b3Body* body )
{
	B3_ASSERT( world->locked == false );
	world->locked = true;
	bool woke = b3WakeBody( world, body );
	world->locked = false;
	return woke;
}































































































































































































































































































































































































































































void b3UpdateBodyMassData( b3World* world, b3Body* body )
{
	b3BodySim* bodySim = b3GetBodySim( world, body );

	// Mass is no longer dirty
	body->flags &= ~b3_dirtyMass;
	b3SyncBodyFlags( world, body );

	// Compute mass data from shapes. Each shape has its own density.
	body->mass = 0.0f;
	body->inertia = b3Mat3_zero;

	bodySim->invMass = 0.0f;
	bodySim->invInertiaLocal = b3Mat3_zero;
	bodySim->invInertiaWorld = b3Mat3_zero;
	bodySim->localCenter = b3Vec3_zero;
	bodySim->minExtent = B3_HUGE;
	bodySim->maxExtent = b3Vec3_zero;

	if ( body->headShapeId == B3_NULL_INDEX )
	{
		return;
	}

	// Static and kinematic sims have zero mass.
	if ( body->type != b3_dynamicBody )
	{
		bodySim->center = bodySim->transform.p;
		bodySim->center0 = bodySim->center;

		// Need extents for kinematic bodies for sleeping to work correctly.
		if ( body->type == b3_kinematicBody )
		{
			int shapeId = body->headShapeId;
			while ( shapeId != B3_NULL_INDEX )
			{
				const b3Shape* s = b3Array_Get( world->shapes, shapeId );

				b3ShapeExtent extent = b3ComputeShapeExtent( s, b3Vec3_zero );
				bodySim->minExtent = b3MinFloat( bodySim->minExtent, extent.minExtent );
				bodySim->maxExtent = b3Max( bodySim->maxExtent, extent.maxExtent );

				shapeId = s->nextShapeId;
			}
		}

		return;
	}

	int shapeCount = body->shapeCount;
	b3MassData* masses = b3StackAlloc( &world->stack, shapeCount * sizeof( b3MassData ), "mass data" );

	// Accumulate mass over all shapes.
	b3Vec3 localCenter = b3Vec3_zero;
	int shapeId = body->headShapeId;
	int shapeIndex = 0;
	while ( shapeId != B3_NULL_INDEX )
	{
		const b3Shape* s = b3Array_Get( world->shapes, shapeId );
		shapeId = s->nextShapeId;

		if ( s->density == 0.0f )
		{
			masses[shapeIndex] = (b3MassData){ 0 };
			shapeIndex += 1;
			continue;
		}

		b3MassData massData = b3ComputeShapeMass( s );
		body->mass += massData.mass;
		localCenter = b3MulAdd( localCenter, massData.mass, massData.center );

		masses[shapeIndex] = massData;
		shapeIndex += 1;
	}

	// Compute center of mass.
	if ( body->mass > 0.0f )
	{
		bodySim->invMass = 1.0f / body->mass;
		localCenter = b3MulSV( bodySim->invMass, localCenter );
	}

	// Second loop to accumulate the rotational inertia about the center of mass
	for ( shapeIndex = 0; shapeIndex < shapeCount; ++shapeIndex )
	{
		b3MassData massData = masses[shapeIndex];
		if ( massData.mass == 0.0f )
		{
			continue;
		}

		// Shift to center of mass. This is safe because it can only increase.
		b3Vec3 offset = b3Sub( localCenter, massData.center );
		b3Matrix3 inertia = b3AddMM( massData.inertia, b3Steiner( massData.mass, offset ) );
		body->inertia = b3AddMM( body->inertia, inertia );
	}

	b3StackFree( &world->stack, masses );
	masses = NULL;

	float det = b3Det( body->inertia );
	B3_ASSERT( det >= 0.0f );

	if ( det > 0.0f )
	{
		// This call is faster than b3Invert
		bodySim->invInertiaLocal = b3InvertT( body->inertia );

		b3Matrix3 rotationMatrix = b3MakeMatrixFromQuat( bodySim->transform.q );
		bodySim->invInertiaWorld = b3MulMM( b3MulMM( rotationMatrix, bodySim->invInertiaLocal ), b3Transpose( rotationMatrix ) );
	}

	// Move center of mass.
	b3Pos oldCenter = bodySim->center;
	bodySim->localCenter = localCenter;
	bodySim->center = b3TransformWorldPoint( bodySim->transform, bodySim->localCenter );
	bodySim->center0 = bodySim->center;

	// Update center of mass velocity
	b3BodyState* state = b3GetBodyState( world, body );
	if ( state != NULL )
	{
		b3Vec3 deltaLinear = b3Cross( state->angularVelocity, b3SubPos( bodySim->center, oldCenter ) );
		state->linearVelocity = b3Add( state->linearVelocity, deltaLinear );
	}

	// Compute body extents relative to center of mass
	shapeId = body->headShapeId;
	while ( shapeId != B3_NULL_INDEX )
	{
		b3Shape* s = b3Array_Get( world->shapes, shapeId );

		b3ShapeExtent extent = b3ComputeShapeExtent( s, localCenter );
		bodySim->minExtent = b3MinFloat( bodySim->minExtent, extent.minExtent );
		bodySim->maxExtent = b3Max( bodySim->maxExtent, extent.maxExtent );

		shapeId = s->nextShapeId;
	}

	// Apply fixed rotation
	if ( ( bodySim->flags & b3_fixedRotation ) == b3_fixedRotation )
	{
		body->inertia = b3Mat3_zero;
		bodySim->invInertiaLocal = b3Mat3_zero;
		bodySim->invInertiaWorld = b3Mat3_zero;
	}
}




































































































































































































































































































































































































































































































// This should follow similar steps as you would get destroying and recreating the body, shapes, and joints.
// Contacts are difficult to preserve because the broad-phase pairs change, so I just destroy them.
// todo with a bit more effort I could support an option to let the body sleep
//
// Revised steps:
// 1 Skip disabled bodies
// 2 Destroy all contacts on the body
// 3 Wake the body
// 4 For all joints attached to the body
//  - wake attached bodies
//  - remove from island
//  - move to static set temporarily
// 5 Change the body type and transfer the body
// 6 If the body was static
//   - create an island for the body
//   Else if the body is becoming static
//   - remove it from the island
// 7 For all joints
//  - if either body is non-static
//    - link into island
//    - transfer to constraint graph
// 8 For all shapes
//  - Destroy proxy in old tree
//  - Create proxy in new tree
// Notes:
// - the implementation below tries to minimize the number of predicates, so some
//   operations may have no effect, such as transferring a joint to the same set































































































































































































































































































































































































































































































































































































// Disabling a body requires a lot of detailed bookkeeping, but it is a valuable feature.
// The most challenging aspect that joints may connect to bodies that are not disabled.













































































































































































































































































































































































































































bool b3ShouldBodiesCollide( b3World* world, b3Body* bodyA, b3Body* bodyB )
{
	if ( bodyA->type != b3_dynamicBody && bodyB->type != b3_dynamicBody )
	{
		return false;
	}

	int jointKey;
	int otherBodyId;
	if ( bodyA->jointCount < bodyB->jointCount )
	{
		jointKey = bodyA->headJointKey;
		otherBodyId = bodyB->id;
	}
	else
	{
		jointKey = bodyB->headJointKey;
		otherBodyId = bodyA->id;
	}

	while ( jointKey != B3_NULL_INDEX )
	{
		int jointId = jointKey >> 1;
		int edgeIndex = jointKey & 1;
		int otherEdgeIndex = edgeIndex ^ 1;

		b3Joint* joint = b3Array_Get( world->joints, jointId );
		if ( joint->collideConnected == false && joint->edges[otherEdgeIndex].bodyId == otherBodyId )
		{
			return false;
		}

		jointKey = joint->edges[edgeIndex].nextKey;
	}

	return true;
}
