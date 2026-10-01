// SPDX-FileCopyrightText: 2026 Erin Catto
// SPDX-License-Identifier: MIT

// Dirk Gregorius contributed portions of this code

#include "hull.h"

#include "algorithm.h"
#include "math_internal.h"
#include "shape.h"

#include "box3d/collision.h"
#include "box3d/constants.h"
#include "box3d/math_functions.h"

#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define B3_AXIS_X 0
#define B3_AXIS_Y 1
#define B3_AXIS_Z 2

#define B3_MARK_VISIBLE 0
#define B3_MARK_DELETE 1

// Final hull is index-encoded with uint8_t, so vertex/edge/face counts are capped at 256.
#define B3_HULL_MAX_COUNT ( UINT8_MAX + 1 )

typedef struct b3QHListNode
{
	struct b3QHListNode* prev;
	struct b3QHListNode* next;
} b3QHListNode;

typedef struct b3QHFace b3QHFace;

typedef struct b3QHVertex
{
	// Intrusive list link. Must be first so (b3QHVertex*)nodePtr is valid.
	b3QHListNode link;

	b3QHFace* conflictFace;
	b3Vec3 position;

	// Index in the finalized hull, stamped during emit. B3_NULL_INDEX until then.
	int finalIndex;
	bool reachable;
} b3QHVertex;

typedef struct b3QHHalfEdge
{
	// Edge ring (CCW) around the owning face. Not an external list.
	struct b3QHHalfEdge* prev;
	struct b3QHHalfEdge* next;

	b3QHVertex* origin;
	b3QHFace* face;
	struct b3QHHalfEdge* twin;

	// Index in the finalized hull, stamped during emit. B3_NULL_INDEX until then.
	int finalIndex;
} b3QHHalfEdge;

struct b3QHFace
{
	// Intrusive list link. Must be first so (b3QHFace*)nodePtr is valid.
	b3QHListNode link;

	b3QHHalfEdge* edge;

	int mark;
	float area;
	b3Plane plane;
	b3Vec3 centroid;
	float maxConflictDistance;

	// Sentinel head for this face's conflict list of b3QHVertex.
	b3QHVertex conflictListHead;

	// Cached farthest conflict vertex (above b3HullBuilder::minOutside).
	// NULL when no conflict above threshold; maxConflictDistance is then minOutside.
	b3QHVertex* maxConflict;

	// Index in the finalized hull, stamped during emit. B3_NULL_INDEX until then.
	int finalIndex;
	bool flipped;
};

// One frame of the iterative horizon DFS. Replaces a recursive call to b3HullBuilder_BuildHorizon.
typedef struct b3HorizonFrame
{
	b3QHFace* face;
	b3QHHalfEdge* startEdge; // ring termination sentinel
	b3QHHalfEdge* edge;		 // next edge to process
	bool started;			 // false until the first edge of this ring has been processed
} b3HorizonFrame;

// All working memory for one hull build, carved from a single b3Alloc block.
typedef struct b3HullBuilder
{
	float tolerance;
	float minRadius;
	float minOutside;

	b3Vec3 interiorPoint;

	// List sentinels. Only the link is meaningful; other fields are unused.
	b3QHVertex orphanedList;
	b3QHVertex vertexList;
	b3QHFace faceList;

	// Bump-allocated pools with pointer-based free lists for faces and edges.
	b3QHVertex* vertexBase;
	int vertexCapacity;
	int vertexCount;

	b3QHHalfEdge* edgeBase;
	int edgeCapacity;
	int edgeCount;

	// LIFO free list. Built with edge->next.
	b3QHHalfEdge* edgeFreeHead;

	b3QHFace* faceBase;
	int faceCapacity;
	int faceCount;

	// LIFO free list. Built with face->link.next.
	b3QHFace* faceFreeHead;

	// Reusable scratch buffers.
	b3QHHalfEdge** horizon;
	int horizonCapacity;
	int horizonCount;

	b3QHFace** cone;
	int coneCapacity;
	int coneCount;

	b3QHFace** mergedFaces;
	int mergedFacesCapacity;
	int mergedFacesCount;

	// DFS stack used by the iterative b3HullBuilder_BuildHorizon. Depth bounded by live faces.
	b3HorizonFrame* horizonStack;
	int horizonStackCapacity;

	// Final counts of the constructed hull (vertexList / faceList / half-edges around faces).
	int finalVertexCount;
	int finalHalfEdgeCount;
	int finalFaceCount;
} b3HullBuilder;

static inline void b3QHList_Init( b3QHListNode* head )
{
	head->prev = head;
	head->next = head;
}

#define B3_LIST_EMPTY( A ) ( ( A )->next == ( A ) )

static inline bool b3QHList_Contains( const b3QHListNode* node )
{
	return node->prev != NULL && node->next != NULL;
}

// Insert node before `where`.
static inline void b3QHList_Insert( b3QHListNode* node, b3QHListNode* where )
{
	B3_ASSERT( !b3QHList_Contains( node ) && b3QHList_Contains( where ) );

	node->prev = where->prev;
	node->next = where;

	node->prev->next = node;
	node->next->prev = node;
}

static inline void b3QHList_Remove( b3QHListNode* node )
{
	B3_ASSERT( b3QHList_Contains( node ) );

	node->prev->next = node->next;
	node->next->prev = node->prev;

	node->prev = NULL;
	node->next = NULL;
}

static inline void b3QHList_PushBack( b3QHListNode* head, b3QHListNode* node )
{
	b3QHList_Insert( node, head->prev );
}

static b3QHVertex* b3HullBuilder_NewVertex( b3HullBuilder* b, b3Vec3 position )
{
	B3_ASSERT( b->vertexCount < b->vertexCapacity );
	b3QHVertex* vertex = b->vertexBase + b->vertexCount++;

	vertex->link.prev = NULL;
	vertex->link.next = NULL;
	vertex->conflictFace = NULL;
	vertex->position = position;
	vertex->finalIndex = B3_NULL_INDEX;
	vertex->reachable = false;

	return vertex;
}

static b3QHHalfEdge* b3HullBuilder_NewEdge( b3HullBuilder* b )
{
	b3QHHalfEdge* edge;
	if ( b->edgeFreeHead != NULL )
	{
		edge = b->edgeFreeHead;
		b->edgeFreeHead = edge->next;
	}
	else
	{
		B3_ASSERT( b->edgeCount < b->edgeCapacity );
		edge = b->edgeBase + b->edgeCount++;
	}
	// All other fields (prev/next/origin/face/twin) are written by NewFace immediately after.
	edge->finalIndex = B3_NULL_INDEX;
	return edge;
}

static void b3HullBuilder_RetireEdge( b3HullBuilder* b, b3QHHalfEdge* edge )
{
	edge->next = b->edgeFreeHead;
	b->edgeFreeHead = edge;
}

static b3QHFace* b3HullBuilder_NewFace( b3HullBuilder* b, b3QHVertex* v1, b3QHVertex* v2, b3QHVertex* v3 )
{
	b3QHFace* face;
	if ( b->faceFreeHead != NULL )
	{
		face = b->faceFreeHead;
		// link.next was used as free-list pointer; recover next head before we clobber.
		b->faceFreeHead = (b3QHFace*)face->link.next;
	}
	else
	{
		B3_ASSERT( b->faceCount < b->faceCapacity );
		face = b->faceBase + b->faceCount++;
	}

	// link.prev: NULL on retired faces (cleared by Remove); NULL here so PushBack's
	// !b3QHList_Contains assert holds for fresh bump slots too.
	// link.next: was the free-list pointer on reused slots, now stale; PushBack overwrites.
	face->link.prev = NULL;
	face->link.next = NULL;
	face->maxConflict = NULL;
	face->maxConflictDistance = 0.0f;
	face->finalIndex = B3_NULL_INDEX;

	b3QHHalfEdge* edge1 = b3HullBuilder_NewEdge( b );
	b3QHHalfEdge* edge2 = b3HullBuilder_NewEdge( b );
	b3QHHalfEdge* edge3 = b3HullBuilder_NewEdge( b );

	b3Vec3 p1 = v1->position;
	b3Vec3 p2 = v2->position;
	b3Vec3 p3 = v3->position;

	b3Plane plane;
	plane.normal = b3Cross( b3Sub( p2, p1 ), b3Sub( p3, p1 ) );
	float length;
	plane.normal = b3GetLengthAndNormalize( &length, plane.normal );
	plane.offset = b3Dot( plane.normal, p1 );

	float area = 0.5f * length;

	face->edge = edge1;
	face->mark = B3_MARK_VISIBLE;
	face->area = area;
	face->centroid = b3MulSV( 1.0f / 3.0f, b3Add( v1->position, b3Add( v2->position, v3->position ) ) );
	face->plane = plane;
	face->flipped = b3PlaneSeparation( plane, b->interiorPoint ) > 0.0f;
	b3QHList_Init( &face->conflictListHead.link );

	edge1->prev = edge3;
	edge1->next = edge2;
	edge1->origin = v1;
	edge1->face = face;
	edge1->twin = NULL;

	edge2->prev = edge1;
	edge2->next = edge3;
	edge2->origin = v2;
	edge2->face = face;
	edge2->twin = NULL;

	edge3->prev = edge2;
	edge3->next = edge1;
	edge3->origin = v3;
	edge3->face = face;
	edge3->twin = NULL;

	return face;
}

// Remove face and add to free list.
static void b3HullBuilder_RetireFace( b3HullBuilder* b, b3QHFace* face )
{
	// Sometimes a cone face gets merged and never added to the list.
	if ( b3QHList_Contains( &face->link ) )
	{
		b3QHList_Remove( &face->link );
	}

	face->edge = NULL;
	// link.prev is already NULL after Remove (or was never set).
	B3_VALIDATE( face->link.prev == NULL );
	face->link.next = (b3QHListNode*)b->faceFreeHead;
	b->faceFreeHead = face;
}

static b3AABB b3BuildBounds( int vertexCount, const b3Vec3* vertices )
{
	b3AABB bounds = B3_BOUNDS3_EMPTY;
	for ( int i = 0; i < vertexCount; ++i )
	{
		bounds.lowerBound = b3Min( bounds.lowerBound, vertices[i] );
		bounds.upperBound = b3Max( bounds.upperBound, vertices[i] );
	}
	return bounds;
}

static void b3FindFarthestPointsAlongCardinalAxes( int* index1Out, int* index2Out, float tolerance, int vertexCount,
												   const b3Vec3* vertexBase )
{
	*index1Out = B3_NULL_INDEX;
	*index2Out = B3_NULL_INDEX;

	b3Vec3 v0 = vertexBase[0];
	b3Vec3 minPt[3] = { v0, v0, v0 };
	b3Vec3 maxPt[3] = { v0, v0, v0 };

	int minIndex[3] = { 0, 0, 0 };
	int maxIndex[3] = { 0, 0, 0 };

	for ( int i = 1; i < vertexCount; ++i )
	{
		b3Vec3 v = vertexBase[i];

		if ( v.x < minPt[B3_AXIS_X].x )
		{
			minPt[B3_AXIS_X] = v;
			minIndex[B3_AXIS_X] = i;
		}
		else if ( v.x > maxPt[B3_AXIS_X].x )
		{
			maxPt[B3_AXIS_X] = v;
			maxIndex[B3_AXIS_X] = i;
		}

		if ( v.y < minPt[B3_AXIS_Y].y )
		{
			minPt[B3_AXIS_Y] = v;
			minIndex[B3_AXIS_Y] = i;
		}
		else if ( v.y > maxPt[B3_AXIS_Y].y )
		{
			maxPt[B3_AXIS_Y] = v;
			maxIndex[B3_AXIS_Y] = i;
		}

		if ( v.z < minPt[B3_AXIS_Z].z )
		{
			minPt[B3_AXIS_Z] = v;
			minIndex[B3_AXIS_Z] = i;
		}
		else if ( v.z > maxPt[B3_AXIS_Z].z )
		{
			maxPt[B3_AXIS_Z] = v;
			maxIndex[B3_AXIS_Z] = i;
		}
	}

	b3Vec3 distance;
	distance.x = maxPt[B3_AXIS_X].x - minPt[B3_AXIS_X].x;
	distance.y = maxPt[B3_AXIS_Y].y - minPt[B3_AXIS_Y].y;
	distance.z = maxPt[B3_AXIS_Z].z - minPt[B3_AXIS_Z].z;

	float distanceArray[3] = { distance.x, distance.y, distance.z };
	int maxElement = b3MaxElementIndex( distance );

	if ( distanceArray[maxElement] > 2.0f * tolerance )
	{
		*index1Out = minIndex[maxElement];
		*index2Out = maxIndex[maxElement];
	}
}

static int b3FindFarthestPointFromLine( int index1, int index2, float tolerance, int vertexCount, const b3Vec3* vertexBase )
{
	b3Vec3 a = vertexBase[index1];
	b3Vec3 b = vertexBase[index2];

	// |ap x ab|^2 / |ab|^2 is the squared perpendicular distance from p to the line.
	// Compares against (2 * tolerance)^2
	b3Vec3 ab = b3Sub( b, a );
	float abLengthSqr = b3Dot( ab, ab );
	B3_ASSERT( abLengthSqr > 0.0f );

	float invAbLengthSqr = 1.0f / abLengthSqr;
	float maxDistanceSqr = 4.0f * tolerance * tolerance;
	int maxIndex = B3_NULL_INDEX;

	for ( int i = 0; i < vertexCount; ++i )
	{
		if ( i == index1 || i == index2 )
		{
			continue;
		}

		b3Vec3 ap = b3Sub( vertexBase[i], a );
		b3Vec3 cross = b3Cross( ap, ab );
		float distanceSqr = b3Dot( cross, cross ) * invAbLengthSqr;
		if ( distanceSqr > maxDistanceSqr )
		{
			maxDistanceSqr = distanceSqr;
			maxIndex = i;
		}
	}

	return maxIndex;
}

static int b3FindFarthestPointFromPlane( int index1, int index2, int index3, float tolerance, int vertexCount,
										 const b3Vec3* vertexBase )
{
	b3Vec3 a = vertexBase[index1];
	b3Vec3 b = vertexBase[index2];
	b3Vec3 c = vertexBase[index3];

	b3Plane plane = b3MakePlaneFromPoints( a, b, c );

	float maxDistance = 2.0f * tolerance;
	int maxIndex = B3_NULL_INDEX;

	for ( int i = 0; i < vertexCount; ++i )
	{
		if ( i == index1 || i == index2 || i == index3 )
		{
			continue;
		}

		float distance = b3AbsFloat( b3PlaneSeparation( plane, vertexBase[i] ) );
		if ( distance > maxDistance )
		{
			maxDistance = distance;
			maxIndex = i;
		}
	}

	return maxIndex;
}

static bool b3IsEdgeConvex( const b3QHHalfEdge* edge, float tolerance )
{
	float distance = b3PlaneSeparation( edge->face->plane, edge->twin->face->centroid );
	return distance < -tolerance;
}

static bool b3IsEdgeConcave( const b3QHHalfEdge* edge, float tolerance )
{
	float distance = b3PlaneSeparation( edge->face->plane, edge->twin->face->centroid );
	return distance > tolerance;
}

static int b3VertexCountOfFace( const b3QHFace* face )
{
	int count = 0;
	const b3QHHalfEdge* edge = face->edge;
	do
	{
		count++;
		edge = edge->next;
	}
	while ( edge != face->edge );

	return count;
}

static void b3LinkFace( b3QHFace* face, int index, b3QHHalfEdge* twin )
{
	B3_ASSERT( face != twin->face );

	b3QHHalfEdge* edge = face->edge;
	while ( index-- > 0 )
	{
		B3_ASSERT( edge->face == face );
		edge = edge->next;
	}

	B3_ASSERT( edge != twin );
	edge->twin = twin;
	twin->twin = edge;
}

static void b3LinkFaces( b3QHFace* face1, int index1, b3QHFace* face2, int index2 )
{
	B3_ASSERT( face1 != face2 );

	b3QHHalfEdge* edge1 = face1->edge;
	while ( index1-- > 0 )
	{
		edge1 = edge1->next;
	}

	b3QHHalfEdge* edge2 = face2->edge;
	while ( index2-- > 0 )
	{
		edge2 = edge2->next;
	}

	B3_ASSERT( edge1 != edge2 );
	edge1->twin = edge2;
	edge2->twin = edge1;
}

static void b3NewellPlane( b3QHFace* face )
{
	int count = 0;
	b3Vec3 centroid = b3Vec3_zero;
	b3Vec3 normal = b3Vec3_zero;

	b3QHHalfEdge* edge = face->edge;
	B3_ASSERT( edge->face == face );

	// Use the first vertex as the origin to reduce round-off
	b3Vec3 origin = edge->origin->position;

	do
	{
		b3QHHalfEdge* twin = edge->twin;
		B3_ASSERT( twin->twin == edge );

		b3Vec3 v1 = b3Sub( edge->origin->position, origin );
		b3Vec3 v2 = b3Sub( twin->origin->position, origin );

		count++;
		centroid = b3Add( centroid, v1 );
		normal.x += ( v1.y - v2.y ) * ( v1.z + v2.z );
		normal.y += ( v1.z - v2.z ) * ( v1.x + v2.x );
		normal.z += ( v1.x - v2.x ) * ( v1.y + v2.y );

		edge = edge->next;
	}
	while ( edge != face->edge );

	B3_ASSERT( count > 0 );
	centroid = b3MulSV( 1.0f / (float)count, centroid );
	centroid = b3Add( centroid, origin );

	float length = b3Length( normal );
	B3_VALIDATE( length > 0.0f );
	normal = b3MulSV( 1.0f / length, normal );

	face->centroid = centroid;
	face->plane = b3MakePlaneFromNormalAndPoint( normal, centroid );
	face->area = 0.5f * length;
}

#if B3_DEBUG
static bool b3CheckConsistency( const b3QHFace* face )
{
	if ( face->mark == B3_MARK_DELETE )
	{
		return false;
	}

	if ( b3VertexCountOfFace( face ) < 3 )
	{
		return false;
	}

	const b3QHHalfEdge* edge = face->edge;

	do
	{
		const b3QHHalfEdge* twin = edge->twin;

		if ( twin == NULL )
		{
			return false;
		}
		if ( twin->face == NULL )
		{
			return false;
		}
		if ( twin->face == face )
		{
			return false;
		}
		if ( twin->face->mark == B3_MARK_DELETE )
		{
			return false;
		}
		if ( twin->twin != edge )
		{
			return false;
		}
		if ( edge->next->origin != twin->origin )
		{
			return false;
		}
		if ( edge->origin != twin->next->origin )
		{
			return false;
		}
		if ( edge->face != face )
		{
			return false;
		}

		edge = edge->next;
	}
	while ( edge != face->edge );

	return true;
}
#endif

static void b3HullBuilder_ComputeTolerance( b3HullBuilder* b, int pointCount, const b3Vec3* points )
{
	b3AABB bounds = b3BuildBounds( pointCount, points );
	b3Vec3 maxAbs = b3Max( b3Abs( bounds.lowerBound ), b3Abs( bounds.upperBound ) );

	float maxSum = maxAbs.x + maxAbs.y + maxAbs.z;
	float maxCoord = b3MaxFloat( maxAbs.x, b3MaxFloat( maxAbs.y, maxAbs.z ) );
	float maxDistance = b3MinFloat( B3_SQRT3 * maxCoord, maxSum );

	float tolerance = ( 3.0f * maxDistance * 1.01f + maxCoord ) * FLT_EPSILON;

	b->tolerance = tolerance;
	b->minRadius = 4.0f * b->tolerance;
	b->minOutside = 2.0f * b->minRadius;
	B3_ASSERT( b->minRadius < b->minOutside + 3.0f * FLT_EPSILON );
}

static bool b3HullBuilder_BuildInitialHull( b3HullBuilder* b, int pointCount, const b3Vec3* points )
{
	int index1, index2;
	b3FindFarthestPointsAlongCardinalAxes( &index1, &index2, b->tolerance, pointCount, points );
	if ( index1 < 0 || index2 < 0 )
	{
		return false;
	}

	int index3 = b3FindFarthestPointFromLine( index1, index2, b->tolerance, pointCount, points );
	if ( index3 < 0 )
	{
		return false;
	}

	int index4 = b3FindFarthestPointFromPlane( index1, index2, index3, b->tolerance, pointCount, points );
	if ( index4 < 0 )
	{
		return false;
	}

	b3Vec3 v1 = b3Sub( points[index1], points[index4] );
	b3Vec3 v2 = b3Sub( points[index2], points[index4] );
	b3Vec3 v3 = b3Sub( points[index3], points[index4] );

	if ( b3ScalarTripleProduct( v1, v2, v3 ) < 0.0f )
	{
		int temp = index2;
		index2 = index3;
		index3 = temp;
	}

	b->interiorPoint = b3Vec3_zero;
	b->interiorPoint = b3Add( b->interiorPoint, points[index1] );
	b->interiorPoint = b3Add( b->interiorPoint, points[index2] );
	b->interiorPoint = b3Add( b->interiorPoint, points[index3] );
	b->interiorPoint = b3Add( b->interiorPoint, points[index4] );
	b->interiorPoint = b3MulSV( 0.25f, b->interiorPoint );

	b3QHVertex* vertex1 = b3HullBuilder_NewVertex( b, points[index1] );
	b3QHList_PushBack( &b->vertexList.link, &vertex1->link );
	b3QHVertex* vertex2 = b3HullBuilder_NewVertex( b, points[index2] );
	b3QHList_PushBack( &b->vertexList.link, &vertex2->link );
	b3QHVertex* vertex3 = b3HullBuilder_NewVertex( b, points[index3] );
	b3QHList_PushBack( &b->vertexList.link, &vertex3->link );
	b3QHVertex* vertex4 = b3HullBuilder_NewVertex( b, points[index4] );
	b3QHList_PushBack( &b->vertexList.link, &vertex4->link );

	b3QHFace* face1 = b3HullBuilder_NewFace( b, vertex1, vertex2, vertex3 );
	b3QHList_PushBack( &b->faceList.link, &face1->link );
	b3QHFace* face2 = b3HullBuilder_NewFace( b, vertex4, vertex2, vertex1 );
	b3QHList_PushBack( &b->faceList.link, &face2->link );
	b3QHFace* face3 = b3HullBuilder_NewFace( b, vertex4, vertex3, vertex2 );
	b3QHList_PushBack( &b->faceList.link, &face3->link );
	b3QHFace* face4 = b3HullBuilder_NewFace( b, vertex4, vertex1, vertex3 );
	b3QHList_PushBack( &b->faceList.link, &face4->link );

	b3LinkFaces( face1, 0, face2, 1 );
	b3LinkFaces( face1, 1, face3, 1 );
	b3LinkFaces( face1, 2, face4, 1 );

	b3LinkFaces( face2, 0, face3, 2 );
	b3LinkFaces( face3, 0, face4, 2 );
	b3LinkFaces( face4, 0, face2, 2 );

#if B3_DEBUG
	B3_ASSERT( b3CheckConsistency( face1 ) );
	B3_ASSERT( b3CheckConsistency( face2 ) );
	B3_ASSERT( b3CheckConsistency( face3 ) );
	B3_ASSERT( b3CheckConsistency( face4 ) );
#endif

	for ( int index = 0; index < pointCount; ++index )
	{
		if ( index == index1 || index == index2 || index == index3 || index == index4 )
		{
			continue;
		}

		b3Vec3 point = points[index];

		float maxDistance = b->minOutside;
		b3QHFace* maxFace = NULL;

		for ( b3QHListNode* node = b->faceList.link.next; node != &b->faceList.link; node = node->next )
		{
			b3QHFace* face = (b3QHFace*)node;
			float distance = b3PlaneSeparation( face->plane, point );
			if ( distance > maxDistance )
			{
				maxDistance = distance;
				maxFace = face;
			}
		}

		if ( maxFace != NULL )
		{
			b3QHVertex* vertex = b3HullBuilder_NewVertex( b, point );
			vertex->conflictFace = maxFace;
			b3QHList_PushBack( &maxFace->conflictListHead.link, &vertex->link );
			if ( maxDistance > maxFace->maxConflictDistance )
			{
				maxFace->maxConflictDistance = maxDistance;
				maxFace->maxConflict = vertex;
			}
		}
	}

	return true;
}

// Recompute the farthest-conflict cache after a face's plane changes.
// Walks the existing conflict list once; cost is bounded by that list, not the global pool.
static void b3HullBuilder_RecacheConflicts( b3QHFace* face, float minOutside )
{
	b3QHVertex* maxVertex = NULL;
	float maxDistance = minOutside;

	for ( b3QHListNode* node = face->conflictListHead.link.next; node != &face->conflictListHead.link; node = node->next )
	{
		b3QHVertex* vertex = (b3QHVertex*)node;
		float distance = b3PlaneSeparation( face->plane, vertex->position );
		if ( distance > maxDistance )
		{
			maxDistance = distance;
			maxVertex = vertex;
		}
	}

	face->maxConflict = maxVertex;
	face->maxConflictDistance = maxDistance;
}

static b3QHVertex* b3HullBuilder_NextConflictVertex( const b3HullBuilder* b )
{
	b3QHVertex* maxVertex = NULL;
	float maxDistance = b->minOutside;

	for ( const b3QHListNode* faceNode = b->faceList.link.next; faceNode != &b->faceList.link; faceNode = faceNode->next )
	{
		const b3QHFace* face = (const b3QHFace*)faceNode;
		if ( face->maxConflict != NULL && face->maxConflictDistance > maxDistance )
		{
			maxDistance = face->maxConflictDistance;
			maxVertex = face->maxConflict;
		}
	}

	return maxVertex;
}

// Move every conflict vertex of `face` onto the orphaned list and clear their conflictFace.
static void b3HullBuilder_DrainConflictList( b3HullBuilder* b, b3QHFace* face )
{
	b3QHListNode* node = face->conflictListHead.link.next;
	while ( node != &face->conflictListHead.link )
	{
		b3QHVertex* orphan = (b3QHVertex*)node;
		node = node->next;

		orphan->conflictFace = NULL;
		b3QHList_Remove( &orphan->link );
		b3QHList_PushBack( &b->orphanedList.link, &orphan->link );
	}
	B3_ASSERT( B3_LIST_EMPTY( &face->conflictListHead.link ) );
}

// Mark a face for deletion, drain its conflict list, and populate a fresh DFS frame for it.
// `entryEdge` is the half-edge in `face` whose twin lies in the just-deleted parent face, or
// NULL for the seed. The frame skips `entryEdge` on recursive entries (it would be ignored
// anyway since the parent's mark is now DELETE, but skipping saves one iteration).
static void b3HullBuilder_EnterHorizonFace( b3HullBuilder* b, b3QHFace* face, b3QHHalfEdge* entryEdge, b3HorizonFrame* frameOut )
{
	face->mark = B3_MARK_DELETE;
	b3HullBuilder_DrainConflictList( b, face );

	frameOut->face = face;
	frameOut->started = false;
	if ( entryEdge != NULL )
	{
		frameOut->startEdge = entryEdge;
		frameOut->edge = entryEdge->next;
	}
	else
	{
		frameOut->startEdge = face->edge;
		frameOut->edge = face->edge;
	}
}

static void b3HullBuilder_BuildHorizon( b3HullBuilder* b, b3QHVertex* apex, b3QHFace* seed )
{
	b3HorizonFrame* stack = b->horizonStack;
	int top = 0;

	B3_ASSERT( top < b->horizonStackCapacity );
	b3HullBuilder_EnterHorizonFace( b, seed, NULL, &stack[top++] );

	while ( top > 0 )
	{
		b3HorizonFrame* f = &stack[top - 1];

		if ( f->started && f->edge == f->startEdge )
		{
			top--;
			continue;
		}
		f->started = true;

		b3QHHalfEdge* edge = f->edge;
		b3QHHalfEdge* twin = edge->twin;
		f->edge = edge->next;

		if ( twin->face->mark != B3_MARK_VISIBLE )
		{
			continue;
		}

		float distance = b3PlaneSeparation( twin->face->plane, apex->position );
		if ( distance > b->minRadius )
		{
			B3_ASSERT( top < b->horizonStackCapacity );
			b3HullBuilder_EnterHorizonFace( b, twin->face, twin, &stack[top++] );
		}
		else
		{
			B3_ASSERT( b->horizonCount < b->horizonCapacity );
			b->horizon[b->horizonCount++] = edge;
		}
	}
}

static void b3HullBuilder_BuildCone( b3HullBuilder* b, b3QHVertex* apex )
{
	for ( int i = 0; i < b->horizonCount; ++i )
	{
		b3QHHalfEdge* edge = b->horizon[i];
		B3_ASSERT( edge->twin->twin == edge );

		b3QHFace* face = b3HullBuilder_NewFace( b, apex, edge->origin, edge->twin->origin );
		B3_ASSERT( b->coneCount < b->coneCapacity );
		b->cone[b->coneCount++] = face;

		b3LinkFace( face, 1, edge->twin );
	}

	b3QHFace* face1 = b->cone[b->coneCount - 1];
	for ( int i = 0; i < b->coneCount; ++i )
	{
		b3QHFace* face2 = b->cone[i];
		b3LinkFaces( face1, 2, face2, 0 );
		face1 = face2;
	}
}

// Retire half-edges in the half-open ring range [begin, end) by pushing each onto edgeFreeHead.
// The caller has already detached these edges from the live face ring (rewire happens before
// destroy), so they are unreachable from the live hull.
static void b3HullBuilder_DestroyEdges( b3HullBuilder* b, b3QHHalfEdge* begin, b3QHHalfEdge* end )
{
	b3QHHalfEdge* edge = begin;
	while ( edge != end )
	{
		b3QHHalfEdge* next = edge->next;
		b3HullBuilder_RetireEdge( b, edge );
		edge = next;
	}
}

static void b3HullBuilder_ConnectEdges( b3HullBuilder* b, b3QHHalfEdge* prev, b3QHHalfEdge* next )
{
	B3_ASSERT( prev != next );
	B3_ASSERT( prev->face == next->face );

	// If both shared neighbors are the same face, prev and next together would orphan that face.
	if ( prev->twin->face == next->twin->face )
	{
		// next is redundant.
		if ( next->face->edge == next )
		{
			next->face->edge = prev;
		}

		b3QHHalfEdge* twin;
		if ( b3VertexCountOfFace( prev->twin->face ) == 3 )
		{
			// Capture all 3 half-edges of the dead triangle before the rewire overwrites prev->twin.
			b3QHHalfEdge* deadEdge0 = prev->twin;		// prev->twin (will be rewired below)
			b3QHHalfEdge* deadEdge1 = next->twin;		// next->twin
			b3QHHalfEdge* deadEdge2 = next->twin->prev; // third edge of the dead triangle

			twin = deadEdge2->twin;
			B3_ASSERT( twin->face->mark != B3_MARK_DELETE );

			b3QHFace* opposingFace = prev->twin->face;
			opposingFace->mark = B3_MARK_DELETE;
			B3_ASSERT( b->mergedFacesCount < b->mergedFacesCapacity );
			b->mergedFaces[b->mergedFacesCount++] = opposingFace;

			prev->next = next->next;
			prev->next->prev = prev;

			prev->twin = twin;
			twin->twin = prev;

			// Drop the redundant vertex (slot abandoned in the bump allocator).
			b3QHList_Remove( &next->origin->link );

			// Retire the 3 half-edges of the dead triangle now that the rewire is complete.
			b3HullBuilder_RetireEdge( b, deadEdge0 );
			b3HullBuilder_RetireEdge( b, deadEdge1 );
			b3HullBuilder_RetireEdge( b, deadEdge2 );
		}
		else
		{
			twin = next->twin;

			if ( twin->face->edge == prev->twin )
			{
				twin->face->edge = twin;
			}

			twin->next = prev->twin->next;
			twin->next->prev = twin;
			// prev->twin slot is retired to the edge free list.
			b3HullBuilder_RetireEdge( b, prev->twin );

			prev->next = next->next;
			prev->next->prev = prev;

			prev->twin = twin;
			twin->twin = prev;

			// Drop the redundant vertex (slot abandoned in the bump allocator).
			b3QHList_Remove( &next->origin->link );
		}

		// Twin->face changed shape; recompute its plane and refresh its cached max conflict.
		b3NewellPlane( twin->face );
		b3HullBuilder_RecacheConflicts( twin->face, b->minOutside );
	}
	else
	{
		prev->next = next;
		next->prev = prev;
	}
}

static void b3HullBuilder_AbsorbFaces( b3HullBuilder* b, b3QHFace* face )
{
	for ( int i = 0; i < b->mergedFacesCount; ++i )
	{
		B3_ASSERT( b->mergedFaces[i]->mark == B3_MARK_DELETE );
		b3QHListNode* head = &b->mergedFaces[i]->conflictListHead.link;

		b3QHListNode* node = head->next;
		while ( node != head )
		{
			b3QHVertex* vertex = (b3QHVertex*)node;
			node = node->next;

			b3QHList_Remove( &vertex->link );

			float distance = b3PlaneSeparation( face->plane, vertex->position );
			if ( distance > b->minOutside )
			{
				b3QHList_PushBack( &face->conflictListHead.link, &vertex->link );
				vertex->conflictFace = face;
				if ( distance > face->maxConflictDistance )
				{
					face->maxConflictDistance = distance;
					face->maxConflict = vertex;
				}
			}
			else
			{
				b3QHList_PushBack( &b->orphanedList.link, &vertex->link );
				vertex->conflictFace = NULL;
			}
		}

		B3_ASSERT( B3_LIST_EMPTY( head ) );

		// Conflict list is now drained. Retire this face to the free list.
		b3HullBuilder_RetireFace( b, b->mergedFaces[i] );
	}
}

static void b3HullBuilder_ConnectFaces( b3HullBuilder* b, b3QHHalfEdge* edge )
{
	b3QHFace* face = edge->face;

	b3QHHalfEdge* twin = edge->twin;

	b3QHHalfEdge* edgePrev = edge->prev;
	b3QHHalfEdge* edgeNext = edge->next;
	b3QHHalfEdge* twinPrev = twin->prev;
	b3QHHalfEdge* twinNext = twin->next;

	while ( edgePrev->twin->face == twin->face )
	{
		B3_ASSERT( edgePrev->twin == twinNext );
		B3_ASSERT( twinNext->twin == edgePrev );

		edgePrev = edgePrev->prev;
		twinNext = twinNext->next;
	}
	B3_ASSERT( edgePrev->face != twinNext->face );

	while ( edgeNext->twin->face == twin->face )
	{
		B3_ASSERT( edgeNext->twin == twinPrev );
		B3_ASSERT( twinPrev->twin == edgeNext );

		edgeNext = edgeNext->next;
		twinPrev = twinPrev->prev;
	}
	B3_ASSERT( edgeNext->face != twinPrev->face );

	face->edge = edgePrev;

	// Discard opposing face. mergedFaces is single-buffered: ConnectFaces does not nest.
	b->mergedFacesCount = 0;
	B3_ASSERT( b->mergedFacesCount < b->mergedFacesCapacity );
	b->mergedFaces[b->mergedFacesCount++] = twin->face;
	twin->face->mark = B3_MARK_DELETE;
	twin->face->edge = NULL;

	for ( b3QHHalfEdge* absorbed = twinNext; absorbed != twinPrev->next; absorbed = absorbed->next )
	{
		absorbed->face = face;
	}

	b3HullBuilder_DestroyEdges( b, edgePrev->next, edgeNext );
	b3HullBuilder_DestroyEdges( b, twinPrev->next, twinNext );

	b3HullBuilder_ConnectEdges( b, edgePrev, twinNext );
	b3HullBuilder_ConnectEdges( b, twinPrev, edgeNext );

	b3NewellPlane( face );
	// Existing conflicts now have stale distances under the new plane; AbsorbFaces will then
	// add more incrementally and update the cache as it goes.
	b3HullBuilder_RecacheConflicts( face, b->minOutside );
#if B3_DEBUG
	B3_ASSERT( b3CheckConsistency( face ) );
#endif

	b3HullBuilder_AbsorbFaces( b, face );
}

static bool b3HullBuilder_MergeConcave( b3HullBuilder* b, b3QHFace* face )
{
	b3QHHalfEdge* edge = face->edge;

	do
	{
		b3QHHalfEdge* twin = edge->twin;

		if ( b3IsEdgeConcave( edge, b->minRadius ) || b3IsEdgeConcave( twin, b->minRadius ) )
		{
			b3HullBuilder_ConnectFaces( b, edge );
			return true;
		}

		edge = edge->next;
	}
	while ( edge != face->edge );

	return false;
}

static bool b3HullBuilder_MergeCoplanar( b3HullBuilder* b, b3QHFace* face )
{
	b3QHHalfEdge* edge = face->edge;

	do
	{
		b3QHHalfEdge* twin = edge->twin;

		if ( !b3IsEdgeConvex( edge, b->minRadius ) || !b3IsEdgeConvex( twin, b->minRadius ) )
		{
			b3HullBuilder_ConnectFaces( b, edge );
			return true;
		}

		edge = edge->next;
	}
	while ( edge != face->edge );

	return false;
}

static void b3HullBuilder_MergeFaces( b3HullBuilder* b )
{
	for ( int i = 0; i < b->coneCount; ++i )
	{
		b3QHFace* face = b->cone[i];
		if ( face->mark == B3_MARK_VISIBLE && face->flipped )
		{
			face->flipped = false;

			float bestArea = 0;
			b3QHHalfEdge* bestEdge = NULL;

			b3QHHalfEdge* edge = face->edge;
			do
			{
				b3QHHalfEdge* twin = edge->twin;
				float area = twin->face->area;
				if ( area > bestArea )
				{
					bestArea = area;
					bestEdge = edge;
				}

				edge = edge->next;
			}
			while ( edge != face->edge );

			B3_ASSERT( bestEdge != NULL );
			b3HullBuilder_ConnectFaces( b, bestEdge );
		}
	}

	for ( int i = 0; i < b->coneCount; ++i )
	{
		b3QHFace* face = b->cone[i];
		if ( face->mark == B3_MARK_VISIBLE )
		{
			while ( b3HullBuilder_MergeConcave( b, face ) )
			{
			}
		}
	}

	for ( int i = 0; i < b->coneCount; ++i )
	{
		b3QHFace* face = b->cone[i];
		if ( face->mark == B3_MARK_VISIBLE )
		{
			while ( b3HullBuilder_MergeCoplanar( b, face ) )
			{
			}
		}
	}
}

static void b3HullBuilder_ResolveVertices( b3HullBuilder* b )
{
	b3QHListNode* node = b->orphanedList.link.next;
	while ( node != &b->orphanedList.link )
	{
		b3QHVertex* vertex = (b3QHVertex*)node;
		node = node->next;
		b3QHList_Remove( &vertex->link );

		float maxDistance = b->minOutside;
		b3QHFace* maxFace = NULL;

		for ( int i = 0; i < b->coneCount; ++i )
		{
			if ( b->cone[i]->mark == B3_MARK_VISIBLE )
			{
				float distance = b3PlaneSeparation( b->cone[i]->plane, vertex->position );
				if ( distance > maxDistance )
				{
					maxDistance = distance;
					maxFace = b->cone[i];
				}
			}
		}

		if ( maxFace != NULL )
		{
			B3_ASSERT( maxFace->mark == B3_MARK_VISIBLE );
			b3QHList_PushBack( &maxFace->conflictListHead.link, &vertex->link );
			vertex->conflictFace = maxFace;
			if ( maxDistance > maxFace->maxConflictDistance )
			{
				maxFace->maxConflictDistance = maxDistance;
				maxFace->maxConflict = vertex;
			}
		}
		// Otherwise: vertex is interior to the hull. Its slot in the bump pool is abandoned.
	}

	B3_ASSERT( B3_LIST_EMPTY( &b->orphanedList.link ) );
}

static void b3HullBuilder_ResolveFaces( b3HullBuilder* b )
{
	// Splice deleted faces out of the face list. Faces already retired by AbsorbFaces are no
	// longer on faceList, so guard with b3QHList_Contains before removing.
	b3QHListNode* node = b->faceList.link.next;
	while ( node != &b->faceList.link )
	{
		b3QHFace* face = (b3QHFace*)node;
		node = node->next;

		if ( face->mark == B3_MARK_DELETE && b3QHList_Contains( &face->link ) )
		{
			B3_ASSERT( B3_LIST_EMPTY( &face->conflictListHead.link ) );

			// Each half-edge is owned by exactly one face, so ring walks over the
			// dead region retire every interior edge exactly once. Merge deleted
			// faces are already off the face list.
			b3QHHalfEdge* start = face->edge;
			b3QHHalfEdge* edge = start;
			do
			{
				b3QHHalfEdge* next = edge->next;
				b3HullBuilder_RetireEdge( b, edge );
				edge = next;
			}
			while ( edge != start );

			b3HullBuilder_RetireFace( b, face );
		}
	}

	for ( int i = 0; i < b->coneCount; ++i )
	{
		b3QHFace* face = b->cone[i];
		if ( face->mark == B3_MARK_DELETE )
		{
			continue;
		}
		b3QHList_PushBack( &b->faceList.link, &face->link );
	}
}

static void b3HullBuilder_AddVertexToHull( b3HullBuilder* b, b3QHVertex* vertex )
{
	b3QHFace* face = vertex->conflictFace;
	vertex->conflictFace = NULL;
	b3QHList_Remove( &vertex->link );
	b3QHList_PushBack( &b->vertexList.link, &vertex->link );

	b->horizonCount = 0;
	b3HullBuilder_BuildHorizon( b, vertex, face );
	B3_ASSERT( b->horizonCount >= 3 );

	b->coneCount = 0;
	b3HullBuilder_BuildCone( b, vertex );
	B3_ASSERT( b->coneCount >= 3 );

	b3HullBuilder_MergeFaces( b );
	b3HullBuilder_ResolveVertices( b );
	b3HullBuilder_ResolveFaces( b );
}

static void b3HullBuilder_CleanHull( b3HullBuilder* b, b3Vec3 origin )
{
	int faceCount = 0;
	int halfEdgeCount = 0;

	for ( b3QHListNode* faceNode = b->faceList.link.next; faceNode != &b->faceList.link; faceNode = faceNode->next )
	{
		b3QHFace* face = (b3QHFace*)faceNode;
		b3QHHalfEdge* edge = face->edge;

		do
		{
			edge->origin->reachable = true;
			edge = edge->next;
			halfEdgeCount++;
		}
		while ( edge != face->edge );

		face->plane.offset += b3Dot( face->plane.normal, origin );
		face->centroid = b3Add( face->centroid, origin );
		faceCount++;
	}

	int vertexCount = 0;
	b3QHListNode* node = b->vertexList.link.next;
	while ( node != &b->vertexList.link )
	{
		b3QHVertex* vertex = (b3QHVertex*)node;
		node = node->next;

		if ( !vertex->reachable )
		{
			b3QHList_Remove( &vertex->link );
		}
		else
		{
			vertex->position = b3Add( vertex->position, origin );
			vertexCount++;
		}
	}

	b->interiorPoint = b3Add( b->interiorPoint, origin );

	b->finalVertexCount = vertexCount;
	b->finalHalfEdgeCount = halfEdgeCount;
	b->finalFaceCount = faceCount;
}

#if B3_DEBUG
static bool b3HullBuilder_IsConsistent( const b3HullBuilder* b )
{
	int v = b->finalVertexCount;
	int e = b->finalHalfEdgeCount / 2;
	int f = b->finalFaceCount;

	if ( v - e + f != 2 )
	{
		return false;
	}

	for ( const b3QHListNode* faceNode = b->faceList.link.next; faceNode != &b->faceList.link; faceNode = faceNode->next )
	{
		const b3QHFace* face = (const b3QHFace*)faceNode;
		if ( face->edge->face != face )
		{
			return false;
		}

		if ( !b3CheckConsistency( face ) )
		{
			return false;
		}

		if ( b3PlaneSeparation( face->plane, b->interiorPoint ) > 0 )
		{
			return false;
		}

		if ( face->mark != B3_MARK_VISIBLE )
		{
			return false;
		}

		const b3QHHalfEdge* edge = face->edge;

		do
		{
			if ( edge->next->origin != edge->twin->origin )
			{
				return false;
			}
			if ( edge->prev->next != edge )
			{
				return false;
			}
			if ( edge->next->prev != edge )
			{
				return false;
			}
			if ( edge->twin->twin != edge )
			{
				return false;
			}
			if ( edge->face != face )
			{
				return false;
			}
			if ( b3DistanceSquared( edge->origin->position, edge->twin->origin->position ) < 1000.0f * FLT_MIN )
			{
				return false;
			}

			edge = edge->next;
		}
		while ( edge != face->edge );
	}

	return true;
}
#endif

static bool b3HullBuilder_HasHull( const b3HullBuilder* b )
{
	int v = b->finalVertexCount;
	int e = b->finalHalfEdgeCount / 2;
	int f = b->finalFaceCount;
	return v - e + f == 2 && f >= 4;
}

// Build the entire hull. Returns true iff the result satisfies Euler's identity.
static bool b3HullBuilder_Construct( b3HullBuilder* b, const b3Vec3* points, int pointCount, int maxVertexCount, b3Vec3 origin,
									 b3Vec3* shiftedPoints )
{
	if ( pointCount < 4 )
	{
		return false;
	}

	for ( int i = 0; i < pointCount; ++i )
	{
		shiftedPoints[i] = b3Sub( points[i], origin );
	}

	b3HullBuilder_ComputeTolerance( b, pointCount, shiftedPoints );
	bool haveInitialHull = b3HullBuilder_BuildInitialHull( b, pointCount, shiftedPoints );
	if ( haveInitialHull == false )
	{
		return false;
	}

	int budget = b3ClampInt( maxVertexCount - 4, 0, B3_HULL_MAX_COUNT - 4 );

	b3QHVertex* vertex = b3HullBuilder_NextConflictVertex( b );
	while ( vertex && budget > 0 )
	{
		b3HullBuilder_AddVertexToHull( b, vertex );
		vertex = b3HullBuilder_NextConflictVertex( b );
		budget -= 1;
	}

	b3HullBuilder_CleanHull( b, origin );

#if B3_DEBUG
	B3_ASSERT( b3HullBuilder_IsConsistent( b ) );
#endif

	return b3HullBuilder_HasHull( b );
}

typedef struct b3HullWorkSizes
{
	// Input point count
	int N;
	// Output point limit
	int M;
	int vertexCapacity;
	int edgeCapacity;
	int faceCapacity;
	int horizonCapacity;
	int coneCapacity;
	int mergedFacesCapacity;
	int horizonStackCapacity;
	size_t totalBytes;

	size_t offsetVertex;
	size_t offsetEdge;
	size_t offsetFace;
	size_t offsetHorizon;
	size_t offsetCone;
	size_t offsetMergedFaces;
	size_t offsetHorizonStack;
	size_t offsetShiftedPoints;
} b3HullWorkSizes;

static b3HullWorkSizes b3ComputeHullWorkSizes( int pointCount, int clampedMaxCount )
{
	b3HullWorkSizes s;
	s.N = pointCount;
	s.M = clampedMaxCount;

	// Vertices: 4 initial hull vertices + at most one per remaining input point. No free list.
	s.vertexCapacity = pointCount + 4;

	// Edges and faces use free-list recycling; capacity is proportional to live hull size.
	// edgeCapacity: peak is ~twice live edges plus cone edges. Minimum 48.
	s.edgeCapacity = b3MaxInt( 48, 24 * s.M - 48 );

	// faceCapacity: peak intermediate state live faces (<=2*M-4) plus full cone (<=3*M-6). Minimum 16.
	s.faceCapacity = b3MaxInt( 16, 5 * s.M - 10 );

	// Horizon/cone bounded by current half-edge count. Merged faces by face count.
	s.horizonCapacity = b3MaxInt( 6, 3 * s.M - 6 );
	s.coneCapacity = s.horizonCapacity;
	s.mergedFacesCapacity = b3MaxInt( 4, 2 * s.M - 4 );

	// Horizon DFS depth is bounded by the number of live faces (Euler: <=2*M-4).
	s.horizonStackCapacity = b3MaxInt( 4, 2 * s.M - 4 );

	size_t offset = 0;

	s.offsetVertex = offset;
	offset = b3AlignUp8( offset + (size_t)s.vertexCapacity * sizeof( b3QHVertex ) );

	s.offsetEdge = offset;
	offset = b3AlignUp8( offset + (size_t)s.edgeCapacity * sizeof( b3QHHalfEdge ) );

	s.offsetFace = offset;
	offset = b3AlignUp8( offset + (size_t)s.faceCapacity * sizeof( b3QHFace ) );

	s.offsetHorizon = offset;
	offset = b3AlignUp8( offset + (size_t)s.horizonCapacity * sizeof( b3QHHalfEdge* ) );

	s.offsetCone = offset;
	offset = b3AlignUp8( offset + (size_t)s.coneCapacity * sizeof( b3QHFace* ) );

	s.offsetMergedFaces = offset;
	offset = b3AlignUp8( offset + (size_t)s.mergedFacesCapacity * sizeof( b3QHFace* ) );

	s.offsetHorizonStack = offset;
	offset = b3AlignUp8( offset + (size_t)s.horizonStackCapacity * sizeof( b3HorizonFrame ) );

	s.offsetShiftedPoints = offset;
	offset += (size_t)pointCount * sizeof( b3Vec3 );

	s.totalBytes = offset;
	return s;
}

static void b3HullBuilder_Init( b3HullBuilder* b, char* mem, const b3HullWorkSizes* s )
{
	memset( b, 0, sizeof( *b ) );
	b3QHList_Init( &b->orphanedList.link );
	b3QHList_Init( &b->vertexList.link );
	b3QHList_Init( &b->faceList.link );

	b->vertexBase = (b3QHVertex*)( mem + s->offsetVertex );
	b->vertexCapacity = s->vertexCapacity;

	b->edgeBase = (b3QHHalfEdge*)( mem + s->offsetEdge );
	b->edgeCapacity = s->edgeCapacity;

	b->faceBase = (b3QHFace*)( mem + s->offsetFace );
	b->faceCapacity = s->faceCapacity;

	b->horizon = (b3QHHalfEdge**)( mem + s->offsetHorizon );
	b->horizonCapacity = s->horizonCapacity;

	b->cone = (b3QHFace**)( mem + s->offsetCone );
	b->coneCapacity = s->coneCapacity;

	b->mergedFaces = (b3QHFace**)( mem + s->offsetMergedFaces );
	b->mergedFacesCapacity = s->mergedFacesCapacity;

	b->horizonStack = (b3HorizonFrame*)( mem + s->offsetHorizonStack );
	b->horizonStackCapacity = s->horizonStackCapacity;
}

static b3Vec3* b3GetHullPointsWrite( b3HullData* hull )
{
	if ( hull->pointOffset == 0 )
	{
		return NULL;
	}
	return (b3Vec3*)( (intptr_t)hull + hull->pointOffset );
}

static b3Plane* b3GetHullPlanesWrite( b3HullData* hull )
{
	if ( hull->planeOffset == 0 )
	{
		return NULL;
	}
	return (b3Plane*)( (intptr_t)hull + hull->planeOffset );
}

static b3HullVertex* b3GetHullVerticesWrite( b3HullData* hull )
{
	if ( hull->vertexOffset == 0 )
	{
		return NULL;
	}
	return (b3HullVertex*)( (intptr_t)hull + hull->vertexOffset );
}

static b3HullHalfEdge* b3GetHullEdgesWrite( b3HullData* hull )
{
	if ( hull->edgeOffset == 0 )
	{
		return NULL;
	}
	return (b3HullHalfEdge*)( (intptr_t)hull + hull->edgeOffset );
}

static float* b3GetHullSoaVerticesWrite( b3HullData* hull )
{
	if ( hull->soaVertexOffset == 0 )
	{
		return NULL;
	}
	return (float*)( (intptr_t)hull + hull->soaVertexOffset );
}

static float* b3GetHullSoaNormalsWrite( b3HullData* hull )
{
	if ( hull->soaNormalOffset == 0 )
	{
		return NULL;
	}
	return (float*)( (intptr_t)hull + hull->soaNormalOffset );
}













































#if B3_ENABLE_VALIDATION










































































































#else







#endif









































































































static void b3UpdateHullBounds( b3HullData* hull )
{
	const b3Vec3* points = b3GetHullPoints( hull );
	int vertexCount = hull->vertexCount;

	B3_ASSERT( vertexCount > 0 );
	b3AABB bounds;
	bounds.lowerBound = points[0];
	bounds.upperBound = points[0];

	for ( int i = 1; i < vertexCount; ++i )
	{
		b3Vec3 p = points[i];
		bounds.lowerBound = b3Min( bounds.lowerBound, p );
		bounds.upperBound = b3Max( bounds.upperBound, p );
	}

	hull->aabb = bounds;
}

// M. Kallay - "Computing the Moment of Inertia of a Solid Defined by a Triangle Mesh"
static bool b3UpdateHullBulkProperties( b3HullData* hull )
{
	const b3Vec3* points = b3GetHullPoints( hull );
	const b3HullFace* faces = b3GetHullFaces( hull );
	const b3HullHalfEdge* edges = b3GetHullEdges( hull );
	const b3Plane* planes = b3GetHullPlanes( hull );

	float area = 0.0f;
	float volume = 0.0f;
	b3Vec3 center = b3Vec3_zero;

	// Use the first vertex to reduce round-off errors.
	b3Vec3 origin = points[0];

	float xx = 0.0f;
	float xy = 0.0f;
	float yy = 0.0f;
	float xz = 0.0f;
	float zz = 0.0f;
	float yz = 0.0f;

	int faceCount = hull->faceCount;

	for ( int faceIndex = 0; faceIndex < faceCount; ++faceIndex )
	{
		const b3HullFace* face = faces + faceIndex;
		const b3HullHalfEdge* edge1 = edges + face->edge;
		const b3HullHalfEdge* edge2 = edges + edge1->next;
		const b3HullHalfEdge* edge3 = edges + edge2->next;

		B3_ASSERT( edge1 != edge3 );
		B3_ASSERT( edge1->origin < hull->vertexCount );

		b3Vec3 v1 = b3Sub( points[edge1->origin], origin );

		do
		{
			B3_ASSERT( edge2->origin < hull->vertexCount );
			B3_ASSERT( edge3->origin < hull->vertexCount );

			b3Vec3 v2 = b3Sub( points[edge2->origin], origin );
			b3Vec3 v3 = b3Sub( points[edge3->origin], origin );

			area += b3Length( b3Cross( b3Sub( v2, v1 ), b3Sub( v3, v1 ) ) );

			float det = b3ScalarTripleProduct( v1, v2, v3 );

			volume += det;

			b3Vec3 v4 = b3Add( v1, b3Add( v2, v3 ) );
			center = b3Add( center, b3MulSV( det, v4 ) );

			xx += det * ( v1.x * v1.x + v2.x * v2.x + v3.x * v3.x + v4.x * v4.x );
			yy += det * ( v1.y * v1.y + v2.y * v2.y + v3.y * v3.y + v4.y * v4.y );
			zz += det * ( v1.z * v1.z + v2.z * v2.z + v3.z * v3.z + v4.z * v4.z );
			xy += det * ( v1.x * v1.y + v2.x * v2.y + v3.x * v3.y + v4.x * v4.y );
			xz += det * ( v1.x * v1.z + v2.x * v2.z + v3.x * v3.z + v4.x * v4.z );
			yz += det * ( v1.y * v1.z + v2.y * v2.z + v3.y * v3.z + v4.y * v4.z );

			edge2 = edge3;
			edge3 = edges + edge3->next;
		}
		while ( edge1 != edge3 );
	}

	B3_VALIDATE( volume > 0.0f );

	b3Vec3 localCenter = volume > 0.0f ? b3MulSV( 0.25f / volume, center ) : b3Vec3_zero;
	center = b3Add( localCenter, origin );

	float radius = FLT_MAX;
	for ( int faceIndex = 0; faceIndex < faceCount; ++faceIndex )
	{
		b3Plane plane = planes[faceIndex];
		float distance = b3PlaneSeparation( plane, center );
		B3_VALIDATE( distance < 0.0f );

		radius = b3MinFloat( radius, -distance );
	}

	B3_VALIDATE( 0.0f < radius && radius < FLT_MAX );

	b3Matrix3 inertia;
	inertia.cx.x = yy + zz;
	inertia.cy.x = -xy;
	inertia.cz.x = -xz;
	inertia.cx.y = -xy;
	inertia.cy.y = xx + zz;
	inertia.cz.y = -yz;
	inertia.cx.z = -xz;
	inertia.cy.z = -yz;
	inertia.cz.z = xx + yy;

	float mass = volume / 6.0f;

	b3Matrix3 centralInertia = b3MulSM( 1.0f / 120.0f, inertia );
	centralInertia = b3SubMM( centralInertia, b3Steiner( mass, localCenter ) );

	hull->center = center;
	hull->centralInertia = centralInertia;
	hull->volume = mass;
	hull->surfaceArea = 0.5f * area;
	hull->innerRadius = radius;

	if ( mass <= 0.0f )
	{
		return false;
	}

	if ( volume <= 0.0f )
	{
		return false;
	}

	if ( area <= 0.0f )
	{
		return false;
	}

	if ( radius <= 0.0f )
	{
		return false;
	}

	return true;
}


































































































































































































































































// Hull identity covers every byte, so the structs carry explicit padding. These lock
// the layout, re-audit padding if a size changes.
_Static_assert( sizeof( b3HullData ) == 144, "unexpected hull data size" );
_Static_assert( sizeof( b3BoxHull ) == 640, "unexpected box hull size" );

// Implement b3HullMap.
#define NAME b3HullMap
#define KEY_TY const b3HullData*
#define VAL_TY int
#define HASH_FN b3HashHullData
#define CMPR_FN b3CompareHullData
#define MALLOC_FN b3Alloc
#define FREE_FN b3Free
#define IMPLEMENTATION_MODE
#include "verstable.h"

























































































































































































































































































































































































































// Constant template box (vertex/edge/face/topology). b3MakeTransformedBoxHull copies and
// fills in the runtime-dependent fields (boxPoints, boxPlanes, aabb, mass properties, hash).
static const b3BoxHull s_boxHull = {
	.base =
		{
			.version = B3_HULL_VERSION,
			.byteCount = sizeof( b3BoxHull ),
			.hash = 0,
			.vertexCount = 8,
			.edgeCount = 24,
			.faceCount = 6,
			.vertexOffset = offsetof( b3BoxHull, boxVertices ),
			.pointOffset = offsetof( b3BoxHull, boxPoints ),
			.edgeOffset = offsetof( b3BoxHull, boxEdges ),
			.planeOffset = offsetof( b3BoxHull, boxPlanes ),
			.faceOffset = offsetof( b3BoxHull, boxFaces ),
			.soaVertexOffset = offsetof( b3BoxHull, vx ),
			.soaNormalOffset = offsetof( b3BoxHull, nx ),
		},
	.boxVertices =
		{
			[0] = { .edge = 8 },
			[1] = { .edge = 1 },
			[2] = { .edge = 0 },
			[3] = { .edge = 9 },
			[4] = { .edge = 13 },
			[5] = { .edge = 3 },
			[6] = { .edge = 5 },
			[7] = { .edge = 11 },
		},
	.boxEdges =
		{
			[0] = { 2, 1, 2, 0 },	 [1] = { 17, 0, 1, 5 },	  [2] = { 4, 3, 1, 0 },	   [3] = { 20, 2, 5, 3 },
			[4] = { 6, 5, 5, 0 },	 [5] = { 23, 4, 6, 4 },	  [6] = { 0, 7, 6, 0 },	   [7] = { 18, 6, 2, 2 },
			[8] = { 10, 9, 0, 1 },	 [9] = { 21, 8, 3, 5 },	  [10] = { 12, 11, 3, 1 }, [11] = { 16, 10, 7, 2 },
			[12] = { 14, 13, 7, 1 }, [13] = { 19, 12, 4, 4 }, [14] = { 8, 15, 4, 1 },  [15] = { 22, 14, 0, 3 },
			[16] = { 7, 17, 3, 2 },	 [17] = { 9, 16, 2, 5 },  [18] = { 11, 19, 6, 2 }, [19] = { 5, 18, 7, 4 },
			[20] = { 15, 21, 1, 3 }, [21] = { 1, 20, 0, 5 },  [22] = { 3, 23, 4, 3 },  [23] = { 13, 22, 5, 4 },
		},
	.boxFaces =
		{
			[0] = { .edge = 0 },
			[1] = { .edge = 8 },
			[2] = { .edge = 16 },
			[3] = { .edge = 20 },
			[4] = { .edge = 19 },
			[5] = { .edge = 21 },
		},
};





































































































































































// todo use new hull scaling technique









