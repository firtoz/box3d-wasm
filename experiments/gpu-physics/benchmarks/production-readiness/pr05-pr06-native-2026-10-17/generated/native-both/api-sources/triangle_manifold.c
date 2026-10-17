// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#include "algorithm.h"
#include "contact.h"
#include "core.h"
#include "manifold.h"
#include "shape.h"

#include "box3d/base.h"
#include "box3d/collision.h"
#include "box3d/constants.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct b3TriangleData
{
	b3Vec3 v1, v2, v3;
	b3Vec3 e1, e2, e3;
	b3Plane plane;
	int flags;
} b3TriangleData;

// Indexed by the 3-bit vertex mask
static const b3TriangleFeature s_triangleFeatures[8] = {
	b3_featureNone,			// 000  (unreachable)
	b3_featureVertex1,		// 001
	b3_featureVertex2,		// 010
	b3_featureEdge1,		// 011  v1,v2
	b3_featureVertex3,		// 100
	b3_featureEdge3,		// 101  v1,v3
	b3_featureEdge2,		// 110  v2,v3
	b3_featureTriangleFace, // 111
};

static b3TriangleFeature b3GetTriangleFeature( const b3SimplexCache* cache )
{
	int count = cache->count;
	B3_ASSERT( 0 < count && count < 4 );

	// Bit i set means triangle vertex i participates in the simplex.
	int mask = 0;
	for ( int i = 0; i < count; ++i )
	{
		B3_ASSERT( cache->indexA[i] < 3 );
		mask |= 1 << cache->indexA[i];
	}

	return s_triangleFeatures[mask];
}































































static bool b3ClipSegmentToTriangleFace( b3ClipVertex segment[2], const b3Vec3* points, b3Plane plane )
{
	b3Vec3 vertex1 = points[2];
	for ( int i = 0; i < 3; ++i )
	{
		b3Vec3 vertex2 = points[i];
		b3Vec3 tangent = b3Normalize( b3Sub( vertex2, vertex1 ) );
		b3Vec3 binormal = b3Cross( tangent, plane.normal );

		b3Plane clipPlane = b3MakePlaneFromNormalAndPoint( binormal, vertex1 );

		int vertexCount = 0;
		b3ClipVertex p1 = segment[0];
		b3ClipVertex p2 = segment[1];

		float distance1 = b3PlaneSeparation( clipPlane, p1.position );
		float distance2 = b3PlaneSeparation( clipPlane, p2.position );

		// If the points are behind the plane
		if ( distance1 <= 0.0f )
		{
			segment[vertexCount++] = p1;
		}
		if ( distance2 <= 0.0f )
		{
			segment[vertexCount++] = p2;
		}

		// If the points are on different sides of the plane
		if ( ( distance1 > 0.0f ) != ( distance2 > 0.0f ) )
		{
			// Find intersection point of edge and plane
			float t = distance1 / ( distance1 - distance2 );
			segment[vertexCount].position = b3Lerp( p1.position, p2.position, t );
			segment[vertexCount].pair = distance1 > 0.0f ? p1.pair : p2.pair;
			vertexCount++;
		}

		if ( vertexCount != 2 )
		{
			return false;
		}

		vertex1 = vertex2;
	}

	return true;
}

static b3SeparatingAxis b3QueryTriangleFaceAndCapsule( b3Plane plane, const b3Capsule* capsule )
{
	float separation1 = b3PlaneSeparation( plane, capsule->center1 );
	float separation2 = b3PlaneSeparation( plane, capsule->center2 );

	if ( separation1 < separation2 )
	{
		return (b3SeparatingAxis){
			.normal = plane.normal,
			.separation = separation1,
			.indexA = 0,
			.indexB = 0,
		};
	}

	return (b3SeparatingAxis){
		.normal = plane.normal,
		.separation = separation2,
		.indexA = 0,
		.indexB = 1,
	};
}

static b3SeparatingAxis b3QueryTriangleAndCapsuleEdges( const b3Vec3* vertices, b3Plane plane, const b3Capsule* capsule )
{
	// Work in the local space of the capsule
	b3Vec3 p1 = capsule->center1;
	b3Vec3 p2 = capsule->center2;
	b3Vec3 capsuleEdge = b3Sub( p2, p1 );

	// Find axis of minimum penetration
	b3Vec3 maxNormal = b3Vec3_zero;
	float maxSeparation = -FLT_MAX;
	int maxIndex1 = B3_NULL_INDEX;
	int maxIndex2 = B3_NULL_INDEX;
	float squaredTolerance = 0.005f * 0.005f;

	int edgeIndex = 2;
	float a = b3Dot( capsuleEdge, plane.normal );
	b3Vec3 v1 = vertices[2];
	for ( int index = 0; index < 3; ++index )
	{
		b3Vec3 v2 = vertices[index];
		b3Vec3 triangleEdge = b3Sub( v2, v1 );
		b3Vec3 sideNormal = b3Normalize( b3Cross( triangleEdge, plane.normal ) );

		// Pretend the triangle edge embeds a zero area face with a side normal. This
		// provides a way to find an edge-edge normal that points outward from
		// the triangle.
		float b = b3Dot( capsuleEdge, sideNormal );

		// Is the capsule edge parallel to the triangle edge? If so, face contact can handle it.
		if ( a * a + b * b < squaredTolerance * b3LengthSquared( capsuleEdge ) )
		{
			v1 = v2;
			edgeIndex = index;
			continue;
		}

		// Similar to hull vs hull (b3QueryEdgeDirections)
		b3Vec3 axis;
		if ( a * b <= 0.0f )
		{
			float t = b / ( b - a );
			axis = b3Lerp( sideNormal, plane.normal, t );
		}
		else
		{
			float t = b / ( a + b );
			axis = b3Lerp( sideNormal, b3Neg( plane.normal ), t );
		}

		B3_VALIDATE( b3LengthSquared( axis ) > 1000.0f * FLT_MIN );
		axis = b3Normalize( axis );
		float separation = b3Dot( axis, b3Sub( p1, v1 ) );

		if ( separation > maxSeparation )
		{
			// Note: We don't exit early if we find a separating axis here since we want to
			// find the best one for caching and account for the convex radius later.
			maxNormal = axis;
			maxSeparation = separation;
			maxIndex1 = edgeIndex;
			maxIndex2 = 0;
		}

		v1 = v2;
		edgeIndex = index;
	}

	// Save result
	return (b3SeparatingAxis){
		.normal = maxNormal,
		.separation = maxSeparation,
		.indexA = maxIndex1,
		.indexB = maxIndex2,
	};
}

static void b3BuildTriangleAndCapsuleFaceContact( b3LocalManifold* manifold, const b3Vec3* triangle, b3Plane plane,
												  const b3Capsule* capsule )
{
	B3_ASSERT( manifold->pointCount == 0 );

	b3ClipVertex segment[2];
	segment[0].position = capsule->center1;
	segment[0].separation = 0.0f;
	segment[0].pair = b3MakeFeaturePair( b3_featureShapeA, 0, b3_featureShapeA, 0 );
	segment[1].position = capsule->center2;
	segment[1].separation = 0.0f;
	segment[1].pair = b3MakeFeaturePair( b3_featureShapeA, 1, b3_featureShapeA, 1 );

	bool havePoints = b3ClipSegmentToTriangleFace( segment, triangle, plane );
	if ( havePoints == false )
	{
		return;
	}

	float radius = capsule->radius;
	float distance1 = b3PlaneSeparation( plane, segment[0].position );
	float distance2 = b3PlaneSeparation( plane, segment[1].position );

	float speculativeDistance = B3_SPECULATIVE_DISTANCE;
	if ( distance1 > speculativeDistance + radius && distance2 > speculativeDistance + radius )
	{
		return;
	}

	// Average points. Half-way between capsule bottom and triangle plane.
	b3Vec3 point1 = b3MulSub( segment[0].position, 0.5f * ( distance1 + capsule->radius ), plane.normal );
	b3Vec3 point2 = b3MulSub( segment[1].position, 0.5f * ( distance2 + capsule->radius ), plane.normal );

	manifold->normal = plane.normal;
	manifold->feature = b3_featureTriangleFace;
	manifold->pointCount = 2;

	b3LocalManifoldPoint* pt = manifold->points + 0;
	pt->point = point1;
	pt->separation = distance1 - capsule->radius;
	pt->pair = segment[0].pair;

	pt = manifold->points + 1;
	pt->point = point2;
	pt->separation = distance2 - capsule->radius;
	pt->pair = segment[1].pair;
}

static void b3BuildTriangleAndCapsuleEdgeContact( b3LocalManifold* manifold, const b3Vec3* triangle, b3Plane plane,
												  const b3Capsule* capsule, b3SeparatingAxis query )
{
	B3_ASSERT( 0 <= query.indexA && query.indexA < 3 );

	b3Vec3 p1 = capsule->center1;
	b3Vec3 p2 = capsule->center2;
	b3Vec3 capsuleEdge = b3Sub( p2, p1 );

	const b3Vec3* vs = triangle;

	b3Vec3 v1 = vs[query.indexA];
	b3Vec3 v2 = vs[( query.indexA + 1 ) % 3];
	b3Vec3 triangleEdge = b3Sub( v2, v1 );

	b3Vec3 sideNormal = b3Normalize( b3Cross( triangleEdge, plane.normal ) );

	// Pretend the triangle edge embeds a zero area face with a side normal. This
	// provides a way to find an edge-edge normal that points outward from
	// the triangle.
	float a = b3Dot( capsuleEdge, plane.normal );
	float b = b3Dot( capsuleEdge, sideNormal );

	// Is the capsule edge parallel to the triangle edge? If so, face contact can handle it.
	float squaredTolerance = 0.005f * 0.005f;
	if ( a * a + b * b < squaredTolerance * b3LengthSquared( capsuleEdge ) )
	{
		return;
	}

	// Similar to hull vs hull (b3QueryEdgeDirections)
	b3Vec3 normal = query.normal;
	b3SegmentDistanceResult result = b3LineDistance( v1, triangleEdge, p1, capsuleEdge );

	if ( result.fraction1 < 0.0f || 1.0f < result.fraction1 || result.fraction2 < 0.0f || 1.0f < result.fraction2 )
	{
		// closest point beyond end points
		return;
	}

	b3Vec3 point = b3Lerp( result.point1, b3MulSub( result.point2, capsule->radius, normal ), 0.5f );
	float separation = b3Dot( normal, b3Sub( p1, v1 ) );

	manifold->normal = normal;
	manifold->pointCount = 1;

	b3TriangleFeature edgesFeatures[] = { b3_featureEdge1, b3_featureEdge2, b3_featureEdge3 };
	manifold->feature = edgesFeatures[query.indexA];

	b3LocalManifoldPoint* pt = manifold->points + 0;
	pt->point = point;
	pt->separation = separation - capsule->radius;
	pt->pair = b3MakeFeaturePair( b3_featureShapeA, query.indexA, b3_featureShapeB, query.indexB );
}

















































































































































static inline int b3GetTriangleSupport( b3Vec3* points, b3Vec3 direction )
{
	int index = 0;
	float distance = b3Dot( points[0], direction );

	float d = b3Dot( points[1], direction );
	if ( d > distance )
	{
		distance = d;
		index = 1;
	}

	d = b3Dot( points[2], direction );
	if ( d > distance )
	{
		return 2;
	}

	return index;
}

static b3SeparatingAxis b3QueryTriangleFace( const b3TriangleData* triangle, const b3HullData* hull )
{
	const b3Vec3* hullPoints = b3GetHullPoints( hull );
	b3Plane plane = triangle->plane;
	b3Vec3 normal = b3Neg( plane.normal );
	int vertexIndex = b3FindHullSupportVertex( hull, normal );
	b3Vec3 support = hullPoints[vertexIndex];
	float separation = b3PlaneSeparation( plane, support );

	return (b3SeparatingAxis){
		.normal = plane.normal,
		.separation = separation,
		.indexA = 0,
		.indexB = vertexIndex,
		.type = b3_faceAxisA,
	};
}

static b3SeparatingAxis b3QueryHullFace( const b3TriangleData* triangle, const b3HullData* hull )
{
	const b3Plane* hullPlanes = b3GetHullPlanes( hull );
	int faceCount = hull->faceCount;

	b3Vec3 trianglePoints[] = { triangle->v1, triangle->v2, triangle->v3 };

	b3Vec3 maxNormal = b3Vec3_zero;
	float maxFaceSeparation = -INFINITY;
	int maxFaceIndex = B3_NULL_INDEX;
	int maxVertexIndex = B3_NULL_INDEX;

	for ( int faceIndex = 0; faceIndex < faceCount; ++faceIndex )
	{
		b3Plane plane = hullPlanes[faceIndex];

		int vertexIndex = b3GetTriangleSupport( trianglePoints, b3Neg( plane.normal ) );
		b3Vec3 support = trianglePoints[vertexIndex];
		float separation = b3PlaneSeparation( plane, support );
		if ( separation > maxFaceSeparation )
		{
			maxNormal = plane.normal;
			maxFaceSeparation = separation;
			maxFaceIndex = faceIndex;
			maxVertexIndex = vertexIndex;
		}
	}

	// Normal points from triangle to hull
	return (b3SeparatingAxis){
		.normal = b3Neg( maxNormal ),
		.separation = maxFaceSeparation,
		.indexA = maxVertexIndex,
		.indexB = maxFaceIndex,
		.type = b3_faceAxisB,
	};
}

// A: hull, B: triangle
static b3SeparatingAxis b3QueryTriangleAndHullEdges( const b3TriangleData* triangle, const b3HullData* hull )
{
	b3SeparatingAxis result = {
		.normal = b3Vec3_zero,
		.separation = -INFINITY,
		.indexA = B3_NULL_INDEX,
		.indexB = B3_NULL_INDEX,
		.type = b3_edgePairAxis,
	};

	b3Vec3 trianglePoints[] = { triangle->v1, triangle->v2, triangle->v3 };
	b3Vec3 triangleEdges[] = { triangle->e1, triangle->e2, triangle->e3 };
	// int edgeFlags[] = { b3_concaveEdge1, b3_concaveEdge1, b3_concaveEdge3 };

#if B3_FORCE_GHOST_COLLISIONS
	int triangleFlags = 0xFF;
#else
	int triangleFlags = triangle->flags;
#endif
	(void)triangleFlags;

	b3Vec3 triNormal = triangle->plane.normal;

	const b3HullHalfEdge* hullEdges = b3GetHullEdges( hull );
	const b3Vec3* hullPoints = b3GetHullPoints( hull );
	const b3Plane* hullPlanes = b3GetHullPlanes( hull );
	int edgeCount = hull->edgeCount;
	float squaredTolerance = 0.005f * 0.005f;

	for ( int i = 0; i < edgeCount; i += 2 )
	{
		const b3HullHalfEdge* edge = hullEdges + i;
		const b3HullHalfEdge* twin = hullEdges + i + 1;
		B3_ASSERT( edge->twin == i + 1 && twin->twin == i );

		b3Vec3 hullPoint = hullPoints[edge->origin];
		b3Vec3 hullEdge = b3Sub( hullPoints[twin->origin], hullPoint );

		b3Vec3 hullNormal1 = hullPlanes[edge->face].normal;
		b3Vec3 hullNormal2 = hullPlanes[twin->face].normal;

		for ( int j = 0; j < 3; ++j )
		{
			b3Vec3 triEdge = triangleEdges[j];

			float cab = b3Dot( hullNormal1, triEdge );
			float dab = b3Dot( hullNormal2, triEdge );
			float bcd = b3Dot( triNormal, hullEdge );
			if ( cab * dab >= 0.0f || cab * bcd <= 0.0f )
			{
				continue;
			}

			// Avoid nearly parallel edges that may lead to invalid separation values at the noise floor.
			if ( b3MaxFloat( cab * cab, dab * dab ) < squaredTolerance * b3LengthSquared( triEdge ) )
			{
				continue;
			}

			// Similar to hull vs hull (b3QueryEdgeDirections)
			// dot(hullNormal1 + t * (hullNormal2 - hullNormal1), triEdge) = 0
			// Normal points out of hull by construction.
			float t = cab / ( cab - dab );
			b3Vec3 axis = b3Lerp( hullNormal1, hullNormal2, t );
			B3_VALIDATE( b3LengthSquared( axis ) > 1000.0f * FLT_MIN );
			axis = b3Normalize( axis );
			float separation = b3Dot( axis, b3Sub( trianglePoints[j], hullPoint ) );

			// if ( separation > result.separation && ( edgeFlags[j] & triangleFlags ) == 0 )
			if ( separation > result.separation )
			{
				// Note: We don't exit early if we find a separating axis here since we want to
				// find the best one for caching.
				// Flip normal to point from triangle to hull.
				result.normal = b3Neg( axis );
				result.separation = separation;
				result.indexA = j;
				result.indexB = i;
			}
		}
	}

	return result;
}

static float b3CollideHullFace( b3LocalManifold* manifold, int pointCapacity, const b3TriangleData* triangle,
								const b3HullData* hull, b3SeparatingAxis query, b3SATCache* cache, bool enableSpeculative )
{
	B3_VALIDATE( query.type == b3_faceAxisB );
	B3_VALIDATE( 0 <= query.indexA && query.indexA < 3 );
	B3_VALIDATE( 0 <= query.indexB && query.indexB < hull->faceCount );

	manifold->pointCount = 0;

	const b3HullFace* hullFaces = b3GetHullFaces( hull );
	const b3HullHalfEdge* hullEdges = b3GetHullEdges( hull );
	const b3Plane* hullPlanes = b3GetHullPlanes( hull );
	const b3Vec3* hullPoints = b3GetHullPoints( hull );

	// Reference hull face
	b3Plane refPlane = hullPlanes[query.indexB];

	// Build clip polygon from triangle face (the incident face)
	b3ClipVertex buffer1[B3_MAX_CLIP_POINTS], buffer2[B3_MAX_CLIP_POINTS];

	b3Vec3 v1 = triangle->v1;
	b3Vec3 v2 = triangle->v2;
	b3Vec3 v3 = triangle->v3;
	buffer1[0].position = v1;
	buffer1[0].separation = b3PlaneSeparation( refPlane, v1 );
	buffer1[0].pair = b3MakeFeaturePair( b3_featureShapeB, 2, b3_featureShapeB, 0 );
	buffer1[1].position = v2;
	buffer1[1].separation = b3PlaneSeparation( refPlane, v2 );
	buffer1[1].pair = b3MakeFeaturePair( b3_featureShapeB, 0, b3_featureShapeB, 1 );
	buffer1[2].position = v3;
	buffer1[2].separation = b3PlaneSeparation( refPlane, v3 );
	buffer1[2].pair = b3MakeFeaturePair( b3_featureShapeB, 1, b3_featureShapeB, 2 );
	int pointCount = 3;

	// Clip triangle face against side planes of reference face
	b3ClipVertex* input = buffer1;
	b3ClipVertex* output = buffer2;

	const b3HullFace* face = hullFaces + query.indexB;
	int edgeIndex = face->edge;

	do
	{
		const b3HullHalfEdge* edge = hullEdges + edgeIndex;
		int nextEdgeIndex = edge->next;
		const b3HullHalfEdge* next = hullEdges + nextEdgeIndex;
		b3Vec3 vertex1 = hullPoints[edge->origin];
		b3Vec3 vertex2 = hullPoints[next->origin];
		b3Vec3 tangent = b3Normalize( b3Sub( vertex2, vertex1 ) );
		b3Vec3 binormal = b3Cross( tangent, refPlane.normal );

		b3Plane clipPlane = b3MakePlaneFromNormalAndPoint( binormal, vertex1 );

		pointCount = b3ClipPolygon( output, input, pointCount, clipPlane, edgeIndex, refPlane );
		B3_ASSERT( pointCount <= B3_MAX_CLIP_POINTS );

		if ( pointCount < 3 )
		{
			// Using a stale cache
			*cache = (b3SATCache){ 0 };
			return query.separation;
		}

		// Swap buffers, output becomes input for the next clipping plane
		B3_SWAP( output, input );
		edgeIndex = nextEdgeIndex;
	}
	while ( edgeIndex != face->edge );

	pointCount = b3MinInt( pointCount, pointCapacity );
	float minSeparation = FLT_MAX;
	int finalPointCount = 0;

	for ( int i = 0; i < pointCount; ++i )
	{
		b3ClipVertex* clipPoint = input + i;
		minSeparation = b3MinFloat( minSeparation, clipPoint->separation );

		if ( enableSpeculative == false && clipPoint->separation > 0.0f )
		{
			continue;
		}

		// Move point onto hull face improved culling
		b3Vec3 point = b3MulSub( clipPoint->position, clipPoint->separation, refPlane.normal );

		b3LocalManifoldPoint* pt = manifold->points + finalPointCount;
		pt->point = point;
		pt->separation = clipPoint->separation;
		pt->pair = b3FlipPair( clipPoint->pair );

		finalPointCount += 1;
	}

	float speculativeDistance = enableSpeculative ? B3_SPECULATIVE_DISTANCE : 0.0f;
	if ( minSeparation > speculativeDistance )
	{
		// This can occur with a stale SAT cache
		manifold->pointCount = 0;
		*cache = (b3SATCache){ 0 };
		return minSeparation;
	}

	manifold->pointCount = finalPointCount;
	manifold->normal = b3Neg( refPlane.normal );
	manifold->feature = b3_featureHullFace;

	// Save cache
	cache->separation = minSeparation;
	cache->type = b3_faceAxisB;
	cache->indexA = (uint8_t)query.indexA;
	cache->indexB = (uint8_t)query.indexB;
	return minSeparation;
}

static float b3CollideTriangleFace( b3LocalManifold* manifold, int pointCapacity, const b3TriangleData* triangle,
									const b3HullData* hull, b3SeparatingAxis query, b3SATCache* cache, bool enableSpeculative )
{
	B3_VALIDATE( query.type == b3_faceAxisA );
	B3_VALIDATE( query.indexA == 0 );
	B3_VALIDATE( 0 <= query.indexB && query.indexB < hull->vertexCount );
	B3_VALIDATE( manifold->pointCount == 0 );

	const b3HullFace* hullFaces = b3GetHullFaces( hull );
	const b3HullHalfEdge* hullEdges = b3GetHullEdges( hull );
	const b3Vec3* hullPoints = b3GetHullPoints( hull );

	// Find incident face
	b3Plane refPlane = triangle->plane;

	int incFace = b3FindIncidentFace( hull, refPlane.normal, query.indexB );

	// Build clip polygon from incident face
	b3ClipVertex buffer1[2 * B3_MAX_CLIP_POINTS], buffer2[2 * B3_MAX_CLIP_POINTS];
	int pointCount = 0;
	const b3HullFace* face = hullFaces + incFace;
	int hullEdgeIndex = face->edge;

	do
	{
		const b3HullHalfEdge* edge = hullEdges + hullEdgeIndex;

		int nextEdgeIndex = edge->next;
		const b3HullHalfEdge* next = hullEdges + nextEdgeIndex;

		b3Vec3 hullPoint = hullPoints[next->origin];
		buffer1[pointCount].position = hullPoint;
		buffer1[pointCount].separation = b3PlaneSeparation( refPlane, hullPoint );
		buffer1[pointCount].pair = b3MakeFeaturePair( b3_featureShapeB, hullEdgeIndex, b3_featureShapeB, nextEdgeIndex );

		pointCount += 1;
		hullEdgeIndex = nextEdgeIndex;
	}
	while ( hullEdgeIndex != face->edge && pointCount < 2 * B3_MAX_CLIP_POINTS );

	B3_ASSERT( pointCount >= 3 );

	// Clip incident face against side planes of reference face (triangle)
	b3ClipVertex* input = buffer1;
	b3ClipVertex* output = buffer2;

	b3Vec3 trianglePoints[] = { triangle->v1, triangle->v2, triangle->v3 };
	b3Vec3 triangleEdges[] = { triangle->e1, triangle->e2, triangle->e3 };

	for ( int i = 0; i < 3 && pointCount > 0; ++i )
	{
		b3Vec3 sideNormal = b3Cross( triangleEdges[i], refPlane.normal );
		sideNormal = b3Normalize( sideNormal );

		b3Plane clipPlane = b3MakePlaneFromNormalAndPoint( sideNormal, trianglePoints[i] );

		pointCount = b3ClipPolygon( output, input, pointCount, clipPlane, i, refPlane );
		B3_ASSERT( pointCount <= 2 * B3_MAX_CLIP_POINTS );

		B3_SWAP( output, input );
	}

	if ( pointCount == 0 )
	{
		// Triangle face clipped away. Invalidate cache.
		*cache = (b3SATCache){ 0 };
		return FLT_MAX;
	}

	pointCount = b3MinInt( pointCount, pointCapacity );

	float minSeparation = FLT_MAX;

	int finalPointCount = 0;
	for ( int i = 0; i < pointCount; ++i )
	{
		b3ClipVertex* clipPoint = input + i;
		minSeparation = b3MinFloat( minSeparation, clipPoint->separation );

		if ( enableSpeculative == false && clipPoint->separation > 0.0f )
		{
			continue;
		}

		// Move point onto triangle surface for improved culling
		// b3Vec3 point = b3MulSub( clipPoint->position, clipPoint->separation, refPlane.normal );
		b3Vec3 point = clipPoint->position;

		b3LocalManifoldPoint* pt = manifold->points + finalPointCount;
		pt->point = point;
		pt->separation = clipPoint->separation;
		pt->pair = clipPoint->pair;

		finalPointCount += 1;
	}

	float speculativeDistance = enableSpeculative ? B3_SPECULATIVE_DISTANCE : 0.0f;
	if ( minSeparation >= speculativeDistance )
	{
		// This can happens if the objects move a part while re-using a cached axis
		*cache = (b3SATCache){ 0 };
		return minSeparation;
	}

	manifold->pointCount = finalPointCount;
	manifold->normal = refPlane.normal;
	manifold->feature = b3_featureTriangleFace;

	// Save cache
	cache->separation = minSeparation;
	cache->type = b3_faceAxisA;
	cache->indexA = (uint8_t)query.indexA;
	cache->indexB = (uint8_t)query.indexB;
	return minSeparation;
}

static void b3CollideTriangleAndHullEdges( b3LocalManifold* manifold, int capacity, b3Vec3 trianglePoint, b3Vec3 triangleEdge,
										   const b3HullData* hull, b3SeparatingAxis query, b3SATCache* cache )
{
	B3_VALIDATE( query.type == b3_edgePairAxis );
	B3_VALIDATE( 0 <= query.indexA && query.indexA < 3 );
	B3_VALIDATE( 0 <= query.indexB && query.indexB < hull->edgeCount );
	B3_VALIDATE( query.separation <= 2.0f * B3_SPECULATIVE_DISTANCE );

	b3Vec3 pA = trianglePoint;
	b3Vec3 eA = triangleEdge;

	const b3HullHalfEdge* edgesB = b3GetHullEdges( hull );
	const b3Vec3* pointsB = b3GetHullPoints( hull );
	const b3HullHalfEdge* edgeB = edgesB + query.indexB;
	const b3HullHalfEdge* twinB = edgesB + edgeB->twin;
	b3Vec3 pB = pointsB[edgeB->origin];
	b3Vec3 qB = pointsB[twinB->origin];
	b3Vec3 eB = b3Sub( qB, pB );

	// Get the closest points between the infinite edge lines
	b3SegmentDistanceResult result = b3LineDistance( pA, eA, pB, eB );

	// Is one of the closest points outside of the associated edge segment?
	if ( capacity == 0 || result.fraction1 < 0.0f || 1.0f < result.fraction1 || result.fraction2 < 0.0f ||
		 1.0f < result.fraction2 )
	{
		// Invalid edge pair, no points generated
		B3_ASSERT( manifold->pointCount == 0 );
		*cache = (b3SATCache){ 0 };
		return;
	}

	float separation = b3Dot( query.normal, b3Sub( pB, pA ) );
	b3Vec3 point = b3MulSV( 0.5f, b3Add( result.point1, result.point2 ) );

	b3LocalManifoldPoint* pt = manifold->points + 0;
	pt->point = point;
	pt->separation = separation;
	pt->pair = b3MakeFeaturePair( b3_featureShapeA, query.indexA, b3_featureShapeB, query.indexB );

	// Save cache
	cache->separation = separation;
	cache->type = b3_edgePairAxis;
	cache->indexA = (uint8_t)query.indexA;
	cache->indexB = (uint8_t)query.indexB;

	manifold->normal = query.normal;
	manifold->pointCount = 1;

	b3TriangleFeature edgesFeatures[] = { b3_featureEdge1, b3_featureEdge2, b3_featureEdge3 };
	manifold->feature = edgesFeatures[query.indexA];
}

b3AtomicInt b3_triangleConvexCalls;
b3AtomicInt b3_triangleCacheHits;

// Triangle is in the local space of the hull for efficiency.






















































































































































































































































































































































































