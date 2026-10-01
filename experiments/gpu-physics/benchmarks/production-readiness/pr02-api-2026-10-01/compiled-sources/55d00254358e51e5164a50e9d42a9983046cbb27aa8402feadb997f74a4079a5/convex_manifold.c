// SPDX-FileCopyrightText: 2026 Erin Catto
// SPDX-License-Identifier: MIT

#include "algorithm.h"
#include "manifold.h"
#include "shape.h"
#include "simd.h"

#include "box3d/base.h"
#include "box3d/collision.h"
#include "box3d/constants.h"

#include <stdbool.h>
#include <stddef.h>

static int b3ClipSegment( b3ClipVertex segment[2], b3Plane plane )
{
	int vertexCount = 0;
	b3ClipVertex vertex1 = segment[0];
	b3ClipVertex vertex2 = segment[1];

	float distance1 = b3PlaneSeparation( plane, vertex1.position );
	float distance2 = b3PlaneSeparation( plane, vertex2.position );

	// If the points are behind the plane
	if ( distance1 <= 0.0f )
	{
		segment[vertexCount++] = vertex1;
	}
	if ( distance2 <= 0.0f )
	{
		segment[vertexCount++] = vertex2;
	}

	// If the points are on different sides of the plane
	if ( distance1 * distance2 < 0.0f )
	{
		// Find intersection point of edge and plane
		float t = distance1 / ( distance1 - distance2 );
		segment[vertexCount].position = b3Add( b3MulSV( 1.0f - t, vertex1.position ), b3MulSV( t, vertex2.position ) );
		segment[vertexCount].pair = distance1 > 0.0f ? vertex1.pair : vertex2.pair;
		vertexCount++;
	}

	return vertexCount;
}

static int b3ClipSegmentToHullFace( b3ClipVertex segment[2], const b3HullData* hull, int refFace )
{
	const b3HullFace* faces = b3GetHullFaces( hull );
	const b3Plane* planes = b3GetHullPlanes( hull );
	const b3HullHalfEdge* edges = b3GetHullEdges( hull );
	const b3Vec3* points = b3GetHullPoints( hull );

	b3Plane refPlane = planes[refFace];

	const b3HullFace* face = faces + refFace;

	int edgeIndex = face->edge;

	do
	{
		const b3HullHalfEdge* edge = edges + edgeIndex;
		int nextEdgeIndex = edge->next;
		const b3HullHalfEdge* next = edges + nextEdgeIndex;

		b3Vec3 vertex1 = points[edge->origin];
		b3Vec3 vertex2 = points[next->origin];
		b3Vec3 tangent = b3Normalize( b3Sub( vertex2, vertex1 ) );
		b3Vec3 binormal = b3Cross( tangent, refPlane.normal );

		int pointCount = b3ClipSegment( segment, b3MakePlaneFromNormalAndPoint( binormal, vertex1 ) );
		if ( pointCount < 2 )
		{
			return 0;
		}

		edgeIndex = nextEdgeIndex;
	}
	while ( edgeIndex != face->edge );

	return 2;
}

static b3SeparatingAxis b3QueryFaceDirectionHullAndCapsule( const b3HullData* hull, const b3Capsule* capsule,
															b3Transform capsuleTransform )
{
	int maxFaceIndex = -1;
	int maxVertexIndex = -1;
	float maxFaceSeparation = -FLT_MAX;
	const b3Plane* planes = b3GetHullPlanes( hull );

	b3Vec3 capsulePoints[2] = {
		b3TransformPoint( capsuleTransform, capsule->center1 ),
		b3TransformPoint( capsuleTransform, capsule->center2 ),
	};

	for ( int faceIndex = 0; faceIndex < hull->faceCount; ++faceIndex )
	{
		b3Plane plane = planes[faceIndex];

		int vertexIndex = b3GetPointSupport( capsulePoints, 2, b3Neg( plane.normal ) );
		b3Vec3 support = capsulePoints[vertexIndex];
		float separation = b3PlaneSeparation( plane, support );
		if ( separation > maxFaceSeparation )
		{
			maxVertexIndex = vertexIndex;
			maxFaceIndex = faceIndex;
			maxFaceSeparation = separation;
		}
	}

	return (b3SeparatingAxis){
		.normal = planes[maxFaceIndex].normal,
		.separation = maxFaceSeparation,
		.indexA = (uint8_t)maxFaceIndex,
		.indexB = (uint8_t)maxVertexIndex,
	};
}

static b3SeparatingAxis b3QueryEdgeDirectionHullAndCapsule( const b3HullData* hull, const b3Capsule* capsule,
															b3Transform capsuleTransform )
{
	// Find axis of minimum penetration
	b3Vec3 maxNormal = b3Vec3_zero;
	float maxSeparation = -FLT_MAX;
	int maxIndexA = B3_NULL_INDEX;
	int maxIndexB = B3_NULL_INDEX;

	// We perform all computations in local space of the hull
	b3Vec3 pA = b3TransformPoint( capsuleTransform, capsule->center1 );
	b3Vec3 qA = b3TransformPoint( capsuleTransform, capsule->center2 );
	b3Vec3 eA = b3Sub( qA, pA );

	const b3HullHalfEdge* edges = b3GetHullEdges( hull );
	const b3Vec3* points = b3GetHullPoints( hull );
	const b3Plane* planes = b3GetHullPlanes( hull );
	float squaredTolerance = B3_PARALLEL_EDGE_TOL * B3_PARALLEL_EDGE_TOL;

	for ( int index = 0; index < hull->edgeCount; index += 2 )
	{
		const b3HullHalfEdge* edge = edges + index;
		const b3HullHalfEdge* twin = edges + index + 1;
		B3_ASSERT( edge->twin == index + 1 && twin->twin == index );

		b3Vec3 qB = points[twin->origin];
		b3Vec3 uB = planes[edge->face].normal;
		b3Vec3 vB = planes[twin->face].normal;

		// An isolated edge (e.g. like in a capsule) defines a circle through the
		// origin on the Gauss map. So testing for overlap between this circle and
		// the arc AB simplifies to a plane test.
		float cba = b3Dot( uB, eA );
		float dba = b3Dot( vB, eA );

		if ( cba * dba < 0.0f )
		{
			// Avoid nearly parallel edges that may lead to invalid separation values at the noise floor.
			if ( b3MaxFloat( cba * cba, dba * dba ) < squaredTolerance * b3LengthSquared( eA ) )
			{
				continue;
			}

			// The intersection of the arcs on the Gauss map is the edge pair axis. Cast the
			// arc of hull B (from uB to vB) against the plane containing the arc of hull A:
			// dot(uB + t * (vB - uB), eA) == 0
			// then
			// t = cba / (cba - dba)
			//
			// The signs of cba and dba differ (Minkowski test), so the division is safe.
			//
			// The axis generated points from B to A by construction since it lands between
			// two face normals on B. This removes the need to orient the separation axis
			// using the hull centers.
			//
			// The axis is perpendicular to both edges so I can use qA and qB as arbitrary
			// points on edgeA and edgeB to measure the separation.

			float t = cba / ( cba - dba );
			b3Vec3 axis = b3Lerp( uB, vB, t );
			B3_VALIDATE( b3LengthSquared( axis ) > 1000.0f * FLT_MIN );
			axis = b3Normalize( axis );
			float separation = b3Dot( axis, b3Sub( qA, qB ) );

			if ( separation > maxSeparation )
			{
				// Note: We don't exit early if we find a separating axis here since we want to
				// find the best one for caching and account for the convex radius later.
				maxNormal = axis;
				maxSeparation = separation;
				maxIndexA = 0;
				maxIndexB = index;
			}
		}
	}

	// Save result
	return (b3SeparatingAxis){
		.normal = maxNormal,
		.separation = maxSeparation,
		.indexA = maxIndexA,
		.indexB = maxIndexB,
	};
}

// Reduce the manifold points to a maximum of 4 points.
// Note: this modifies the input point array to improve performance
static void b3ReduceManifoldPoints( b3LocalManifold* manifold, int capacity, b3LocalManifoldPoint* points, int count )
{
	if ( capacity < 4 )
	{
		return;
	}

	if ( count <= 4 )
	{
		for ( int i = 0; i < count; ++i )
		{
			manifold->points[i] = points[i];
		}

		manifold->pointCount = count;
		return;
	}

	b3Vec3 normal = manifold->normal;
	// float linearSlop = B3_LINEAR_SLOP;
	float speculativeDistance = B3_SPECULATIVE_DISTANCE;
	float tolSqr = speculativeDistance * speculativeDistance;

	// This bias is very important for contact point consistency across time steps.
	// It creates a pecking order to avoid flickering between candidates with similar scores.
	float bias = 0.95f;

	// Step 1: find extreme point that is touching
	int bestIndex = B3_NULL_INDEX;
	float bestScore = -FLT_MAX;

	// Arbitrary tangent direction
	// b3Vec3 perp1 = b3Perp( normal );
	// b3Vec3 perp2 = b3Cross( perp1, normal );
	// b3Vec3 searchDirection = -0.4535961214255773f * perp1 + 0.8912073600614354f * perp2;
	b3Vec3 searchDirection = b3ArbitraryPerp( normal );
	for ( int index = 0; index < count; ++index )
	{
		b3LocalManifoldPoint* pt = points + index;

		if ( pt->separation > speculativeDistance )
		{
			continue;
		}

		// The deeper the better
		float score = -pt->separation + b3Dot( searchDirection, pt->point );
		if ( bias * score > bestScore )
		{
			bestIndex = index;
			bestScore = score;
		}
	}

	B3_VALIDATE( 0 <= bestIndex && bestIndex < count );
	if ( bestIndex == B3_NULL_INDEX )
	{
		manifold->pointCount = 0;
		return;
	}

	manifold->points[0] = points[bestIndex];
	manifold->pointCount = 1;

	// Remove best point from array
	points[bestIndex] = points[count - 1];
	count -= 1;

	b3Vec3 a = manifold->points[0].point;

	// Step 2: Find farthest point in 2D
	bestScore = 0.0f;
	bestIndex = B3_NULL_INDEX;
	float maxDistanceSquared = 0.0f;

	for ( int index = 0; index < count; ++index )
	{
		b3Vec3 p = points[index].point;
		b3Vec3 d = b3Sub( p, a );
		b3Vec3 v = b3MulSub( d, b3Dot( d, normal ), normal );
		float distanceSquared = b3LengthSquared( v );
		maxDistanceSquared = b3MaxFloat( maxDistanceSquared, distanceSquared );
		float separation = b3MaxFloat( 0.0f, -points[index].separation );
		float score = distanceSquared + 4.0f * separation * separation;
		if ( bias * score > bestScore )
		{
			bestScore = score;
			bestIndex = index;
		}
	}

	if ( bestScore < tolSqr )
	{
		return;
	}

	B3_ASSERT( 0 <= bestIndex && bestIndex < count );
	manifold->points[1] = points[bestIndex];
	manifold->pointCount = 2;

	// Remove best point from array
	points[bestIndex] = points[count - 1];
	count -= 1;

	b3Vec3 b = manifold->points[1].point;

	// Step 3: Find the point with the maximum triangular area
	bestScore = tolSqr;
	bestIndex = B3_NULL_INDEX;
	float bestSignedArea = 0.0f;
	b3Vec3 ba = b3Sub( b, a );
	for ( int index = 0; index < count; ++index )
	{
		b3Vec3 p = points[index].point;
		float signedArea = b3Dot( normal, b3Cross( ba, b3Sub( p, a ) ) );
		float score = b3AbsFloat( signedArea );
		if ( bias * score >= bestScore )
		{
			bestScore = score;
			bestIndex = index;
			bestSignedArea = signedArea;
		}
	}

	if ( bestIndex == B3_NULL_INDEX )
	{
		return;
	}

	B3_ASSERT( bestIndex != B3_NULL_INDEX );

	manifold->points[2] = points[bestIndex];
	manifold->pointCount = 3;
	points[bestIndex] = points[count - 1];
	count -= 1;

	b3Vec3 c = manifold->points[2].point;

	// Step 4: get the point that adds the most area outside the current triangle
	bestScore = tolSqr;
	bestIndex = B3_NULL_INDEX;
	float sign = bestSignedArea < 0.0f ? -1.0f : 1.0f;
	for ( int index = 0; index < count; ++index )
	{
		b3Vec3 p = points[index].point;
		float u1 = sign * b3Dot( normal, b3Cross( b3Sub( p, a ), ba ) );
		float u2 = sign * b3Dot( normal, b3Cross( b3Sub( p, b ), b3Sub( c, b ) ) );
		float u3 = sign * b3Dot( normal, b3Cross( b3Sub( p, c ), b3Sub( a, c ) ) );
		float score = b3MaxFloat( u1, b3MaxFloat( u2, u3 ) );

		if ( bias * score > bestScore )
		{
			bestScore = score;
			bestIndex = index;
		}
	}

	if ( bestIndex != B3_NULL_INDEX )
	{
		manifold->points[manifold->pointCount] = points[bestIndex];
		manifold->pointCount += 1;
	}
}
















































































































































































































































































































































static bool b3BuildHullFaceAndCapsuleContact( b3LocalManifold* manifold, const b3HullData* hullA, const b3Capsule* capsuleB,
											  b3Transform transformBtoA, b3SeparatingAxis query )
{
	// Work in shapeA coordinates
	const b3Plane* planes = b3GetHullPlanes( hullA );

	// Clip the capsule edge against the side planes of the reference face
	int refFace = query.indexA;
	b3Plane refPlane = planes[refFace];

	b3ClipVertex segmentB[2];
	segmentB[0].position = b3TransformPoint( transformBtoA, capsuleB->center1 );
	segmentB[0].separation = 0.0f;
	segmentB[0].pair = b3MakeFeaturePair( b3_featureShapeA, 0, b3_featureShapeA, 0 );
	segmentB[1].position = b3TransformPoint( transformBtoA, capsuleB->center2 );
	segmentB[1].separation = 0.0f;
	segmentB[1].pair = b3MakeFeaturePair( b3_featureShapeA, 1, b3_featureShapeA, 1 );

	int pointCount = b3ClipSegmentToHullFace( segmentB, hullA, refFace );
	if ( pointCount < 2 )
	{
		return false;
	}

	float distance1 = b3PlaneSeparation( refPlane, segmentB[0].position );
	float distance2 = b3PlaneSeparation( refPlane, segmentB[1].position );
	const float speculativeDistance = B3_SPECULATIVE_DISTANCE;

	if ( distance1 <= speculativeDistance || distance2 <= speculativeDistance )
	{
		b3Vec3 normal = refPlane.normal;
		b3Vec3 point1 = b3MulSub( segmentB[0].position, 0.5f * ( distance1 + capsuleB->radius ), normal );
		b3Vec3 point2 = b3MulSub( segmentB[1].position, 0.5f * ( distance2 + capsuleB->radius ), normal );

		// Manifold in frame A
		manifold->normal = normal;
		manifold->pointCount = 2;

		b3LocalManifoldPoint* pt1 = manifold->points + 0;
		pt1->point = point1;
		pt1->separation = distance1 - capsuleB->radius;
		pt1->pair = segmentB[0].pair;

		b3LocalManifoldPoint* pt2 = manifold->points + 1;
		pt2->point = point2;
		pt2->separation = distance2 - capsuleB->radius;
		pt2->pair = segmentB[1].pair;

		return true;
	}

	return false;
}

static bool b3BuildHullAndCapsuleEdgeContact( b3LocalManifold* manifold, int capacity, const b3HullData* hullA,
											  const b3Capsule* capsuleB, b3Transform transformBtoA, b3SeparatingAxis query )
{
	if ( capacity < 1 )
	{
		return false;
	}

	// Work in shapeA coordinates

	b3Vec3 pc = b3TransformPoint( transformBtoA, capsuleB->center1 );
	b3Vec3 qc = b3TransformPoint( transformBtoA, capsuleB->center2 );
	b3Vec3 ec = b3Sub( qc, pc );

	const b3HullHalfEdge* edges = b3GetHullEdges( hullA );
	const b3Vec3* points = b3GetHullPoints( hullA );

	const b3HullHalfEdge* edge2 = edges + query.indexB;
	const b3HullHalfEdge* twin2 = edges + edge2->twin;
	b3Vec3 ph = points[edge2->origin];
	b3Vec3 qh = points[twin2->origin];
	b3Vec3 eh = b3Sub( qh, ph );

	b3Vec3 normal = query.normal;

	b3SegmentDistanceResult result = b3LineDistance( ph, eh, pc, ec );

	if ( b3IsWithinSegments( &result ) == false )
	{
		// closest point beyond end points
		return false;
	}

	b3Vec3 point = b3MulSV( 0.5f, b3Add( b3MulSub( result.point1, capsuleB->radius, normal ), result.point2 ) );
	float separation = b3Dot( normal, b3Sub( result.point2, result.point1 ) );
	B3_VALIDATE( b3AbsFloat( separation - query.separation ) < B3_LINEAR_SLOP );

	// Manifold in frame A
	manifold->normal = normal;
	manifold->pointCount = 1;

	b3LocalManifoldPoint* pt = manifold->points + 0;
	pt->point = point;
	pt->separation = separation - capsuleB->radius;
	pt->pair = b3MakeFeaturePair( b3_featureShapeA, query.indexA, b3_featureShapeB, query.indexB );
	return true;
}












































































































































static int b3BuildPolygon( b3ClipVertex* out, b3Transform transform, const b3HullData* hull, int incFace, b3Plane refPlane )
{
	const b3HullFace* faces = b3GetHullFaces( hull );
	const b3HullHalfEdge* edges = b3GetHullEdges( hull );
	const b3Vec3* points = b3GetHullPoints( hull );

	const b3HullFace* face = faces + incFace;
	int edgeIndex = face->edge;
	B3_ASSERT( edges[edgeIndex].face == incFace );

	int outCount = 0;

	b3Matrix3 matrix = b3MakeMatrixFromQuat( transform.q );

	do
	{
		const b3HullHalfEdge* edge = edges + edgeIndex;

		int nextEdgeIndex = edge->next;
		const b3HullHalfEdge* next = edges + nextEdgeIndex;

		b3ClipVertex vertex;
		vertex.position = b3Add( b3MulMV( matrix, points[next->origin] ), transform.p );
		vertex.separation = b3PlaneSeparation( refPlane, vertex.position );
		vertex.pair = b3MakeFeaturePair( b3_featureShapeB, edgeIndex, b3_featureShapeB, nextEdgeIndex );

		out[outCount] = vertex;
		outCount += 1;

		edgeIndex = nextEdgeIndex;
	}
	while ( edgeIndex != face->edge && outCount < B3_MAX_CLIP_POINTS );

	B3_VALIDATE( b3ValidatePolygon( out, outCount ) );

	return outCount;
}

static bool b3BuildFaceAContact( b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3HullData* hullB,
								 b3Transform transformBtoA, b3SeparatingAxis query, b3SATCache* cache )
{
	B3_VALIDATE( query.type == b3_faceAxisA );
	B3_VALIDATE( 0 <= query.indexA && query.indexA < hullA->faceCount );
	B3_VALIDATE( 0 <= query.indexB && query.indexB < hullB->vertexCount );

	const b3HullFace* facesA = b3GetHullFaces( hullA );
	const b3HullHalfEdge* edgesA = b3GetHullEdges( hullA );
	const b3Plane* planesA = b3GetHullPlanes( hullA );
	const b3Vec3* pointsA = b3GetHullPoints( hullA );

	// Reference face
	int refFace = query.indexA;
	b3Plane refPlane = planesA[refFace];

	// Find incident face
	b3Vec3 refNormalInB = b3InvRotateVector( transformBtoA.q, refPlane.normal );
	int incFace = b3FindIncidentFace( hullB, refNormalInB, query.indexB );

	// Build clip polygon from incident face in frame A
	b3ClipVertex buffer1[B3_MAX_CLIP_POINTS], buffer2[B3_MAX_CLIP_POINTS];
	int pointCount = b3BuildPolygon( buffer1, transformBtoA, hullB, incFace, refPlane );

	// Clip incident face against side planes of reference face
	b3ClipVertex* input = buffer1;
	b3ClipVertex* output = buffer2;

	const b3HullFace* face = facesA + refFace;
	int edgeIndex = face->edge;

	do
	{
		const b3HullHalfEdge* edge = edgesA + edgeIndex;
		int nextEdgeIndex = edge->next;
		const b3HullHalfEdge* next = edgesA + nextEdgeIndex;
		b3Vec3 vertex1 = pointsA[edge->origin];
		b3Vec3 vertex2 = pointsA[next->origin];
		b3Vec3 tangent = b3Normalize( b3Sub( vertex2, vertex1 ) );
		b3Vec3 binormal = b3Cross( tangent, refPlane.normal );

		b3Plane clipPlane = b3MakePlaneFromNormalAndPoint( binormal, vertex1 );

		pointCount = b3ClipPolygon( output, input, pointCount, clipPlane, edgeIndex, refPlane );
		B3_ASSERT( pointCount <= B3_MAX_CLIP_POINTS );

		B3_SWAP( output, input );

		if ( pointCount < 3 )
		{
			*cache = (b3SATCache){ 0 };
			return false;
		}

		edgeIndex = nextEdgeIndex;
	}
	while ( edgeIndex != face->edge );

	pointCount = b3MinInt( pointCount, B3_MAX_CLIP_POINTS );

	b3LocalManifoldPoint points[B3_MAX_CLIP_POINTS];
	float minSeparation = FLT_MAX;

	manifold->normal = refPlane.normal;

	for ( int i = 0; i < pointCount; ++i )
	{
		b3ClipVertex* clipPoint = input + i;
		b3LocalManifoldPoint* pt = points + i;
		*pt = (b3LocalManifoldPoint){ 0 };

		// Using the half-way point keeps the points in the same position when swapping reference face from A to B.
		b3Vec3 point = b3MulSub( clipPoint->position, 0.5f * clipPoint->separation, refPlane.normal );

		// Old way of pushing onto the reference face.
		// b3Vec3 point = clipPoint->position - clipPoint->separation * refPlane.normal;

		pt->point = point;
		pt->separation = clipPoint->separation;
		pt->pair = clipPoint->pair;

		minSeparation = b3MinFloat( minSeparation, clipPoint->separation );
	}

	if ( minSeparation >= B3_SPECULATIVE_DISTANCE )
	{
		*cache = (b3SATCache){ 0 };
		return false;
	}

	b3ReduceManifoldPoints( manifold, capacity, points, pointCount );

	// Save cache
	cache->separation = minSeparation;
	cache->type = (uint8_t)b3_faceAxisA;
	cache->indexA = (uint8_t)query.indexA;
	cache->indexB = (uint8_t)query.indexB;

	return true;
}

static bool b3BuildFaceBContact( b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3HullData* hullB,
								 b3Transform transformBtoA, b3SeparatingAxis query, b3SATCache* cache )
{
	B3_VALIDATE( query.type == b3_faceAxisB );

	b3Transform transformAtoB = b3InvertTransform( transformBtoA );
	b3SeparatingAxis flippedQuery = {
		.normal = b3Neg( query.normal ),
		.separation = query.separation,
		.indexA = query.indexB,
		.indexB = query.indexA,
		.type = b3_faceAxisA,
	};

	bool touching = b3BuildFaceAContact( manifold, capacity, hullB, hullA, transformAtoB, flippedQuery, cache );
	if ( touching == false )
	{
		*cache = (b3SATCache){ 0 };
		return false;
	}

	// Results are in frame B, need to transform them into frame A
	b3Matrix3 matrix = b3MakeMatrixFromQuat( transformBtoA.q );

	// Transform and flip normal so it points from A to B, even though the B has the reference face.
	manifold->normal = b3Neg( b3MulMV( matrix, manifold->normal ) );

	// Transform points from frame B to frame A.
	// Also flip the pairs to ensure correct matches.
	for ( int i = 0; i < manifold->pointCount; ++i )
	{
		b3LocalManifoldPoint* pt = manifold->points + i;
		pt->point = b3Add( b3MulMV( matrix, pt->point ), transformBtoA.p );
		pt->pair = b3FlipPair( pt->pair );
	}

	cache->type = (uint8_t)b3_faceAxisB;
	cache->indexA = (uint8_t)query.indexA;
	cache->indexB = (uint8_t)query.indexB;

	return true;
}

static bool b3BuildEdgeContact( b3LocalManifold* manifold, const b3HullData* hullA, const b3HullData* hullB,
								b3Transform transformBtoA, b3SeparatingAxis query, b3SATCache* cache )
{
	B3_VALIDATE( query.type == b3_edgePairAxis );
	B3_VALIDATE( 0 <= query.indexA && query.indexA < hullA->edgeCount );
	B3_VALIDATE( 0 <= query.indexB && query.indexB < hullB->edgeCount );

	// Work in shapeA coordinates
	const b3HullHalfEdge* edgesA = b3GetHullEdges( hullA );
	const b3Vec3* pointsA = b3GetHullPoints( hullA );

	const b3HullHalfEdge* edgesB = b3GetHullEdges( hullB );
	const b3Vec3* pointsB = b3GetHullPoints( hullB );

	// B3_VALIDATE( query.separation <= 2.0f * B3_SPECULATIVE_DISTANCE );

	const b3HullHalfEdge* edgeA = edgesA + query.indexA;
	const b3HullHalfEdge* twinA = edgesA + edgeA->twin;
	b3Vec3 pA = pointsA[edgeA->origin];
	b3Vec3 qA = pointsA[twinA->origin];
	b3Vec3 eA = b3Sub( qA, pA );

	const b3HullHalfEdge* edgeB = edgesB + query.indexB;
	const b3HullHalfEdge* twinB = edgesB + edgeB->twin;
	b3Vec3 pB = b3TransformPoint( transformBtoA, pointsB[edgeB->origin] );
	b3Vec3 qB = b3TransformPoint( transformBtoA, pointsB[twinB->origin] );
	b3Vec3 eB = b3Sub( qB, pB );

	b3Vec3 normal = query.normal;
	b3SegmentDistanceResult result = b3LineDistance( pA, eA, pB, eB );

	if ( b3IsWithinSegments( &result ) == false )
	{
		*cache = (b3SATCache){ 0 };
		return false;
	}

	// This can slide off the end from caching
	float separation = b3Dot( normal, b3Sub( result.point2, result.point1 ) );
	b3Vec3 point = b3MulSV( 0.5f, b3Add( result.point1, result.point2 ) );

	// Result in frame A
	manifold->normal = normal;
	manifold->pointCount = 1;

	b3LocalManifoldPoint* pt = manifold->points + 0;
	pt->point = point;
	pt->separation = separation;
	pt->pair = b3MakeFeaturePair( b3_featureShapeA, query.indexA, b3_featureShapeB, query.indexB );

	// Save cache
	cache->separation = separation;
	cache->type = (uint8_t)b3_edgePairAxis;
	cache->indexA = (uint8_t)query.indexA;
	cache->indexB = (uint8_t)query.indexB;

	return true;
}

// Transform a SoA point/normal stream (already split into X/Y/Z) by out = -(R*v (+t)).
// The inputs come straight from the hull's stored SoA arrays, so there's no transpose here.
// IsPoint is a template arg so the translation add is only emitted for points.
static inline void b3NegativeTransformFromSoA( b3Matrix3* R, b3Vec3 p, const float* inX, const float* inY, const float* inZ,
											   int n, float* outX, float* outY, float* outZ, bool isPoint )
{
	B3_VALIDATE( ( (uintptr_t)outX & 0xF ) == 0 );
	B3_VALIDATE( ( (uintptr_t)outY & 0xF ) == 0 );
	B3_VALIDATE( ( (uintptr_t)outZ & 0xF ) == 0 );

	// row-column
	b3FloatW r00 = b3SplatW( R->cx.x );
	b3FloatW r01 = b3SplatW( R->cy.x );
	b3FloatW r02 = b3SplatW( R->cz.x );
	b3FloatW r10 = b3SplatW( R->cx.y );
	b3FloatW r11 = b3SplatW( R->cy.y );
	b3FloatW r12 = b3SplatW( R->cz.y );
	b3FloatW r20 = b3SplatW( R->cx.z );
	b3FloatW r21 = b3SplatW( R->cy.z );
	b3FloatW r22 = b3SplatW( R->cz.z );

	b3FloatW tx = b3ZeroW();
	b3FloatW ty = b3ZeroW();
	b3FloatW tz = b3ZeroW();

	if ( isPoint )
	{
		tx = b3SplatW( p.x );
		ty = b3SplatW( p.y );
		tz = b3SplatW( p.z );
	}

	for ( int i = 0; i < n; i += 4 )
	{
		b3FloatW x = b3LoadW( inX + i );
		b3FloatW y = b3LoadW( inY + i );
		b3FloatW z = b3LoadW( inZ + i );

		// Rotate four vectors at a time
		b3FloatW ox = b3Dot3W( r00, r01, r02, x, y, z );
		b3FloatW oy = b3Dot3W( r10, r11, r12, x, y, z );
		b3FloatW oz = b3Dot3W( r20, r21, r22, x, y, z );

		if ( isPoint )
		{
			ox = b3AddW( ox, tx );
			oy = b3AddW( oy, ty );
			oz = b3AddW( oz, tz );
		}

		b3StoreW( outX + i, b3NegW( ox ) );
		b3StoreW( outY + i, b3NegW( oy ) );
		b3StoreW( outZ + i, b3NegW( oz ) );
	}
}

_Static_assert( B3_MAX_HULL_VERTICES == 128, "must be 128" );

#define B3_HULL_BIT_COUNT 7

// SIMD support point calculation using a SoA vertex array padded with repeats of the first vertex
// to a multiple of 4.
//
// This minimizes (bias - dot), where the caller is expected to provide a bias that makes this always positive.
// It can be direction dependent. The bias should be just big enough to ensure the value if positive because
// an excessive bias causes a precision loss in the support calculation.
//
// The vertex index is embedded in the low B3_HULL_BIT_COUNT mantissa bits of the value. By minimizing a value that
// is always positive, the minimum carries the smallest index so that padded SoA values will never win. This is
// purpose of using the bias instead of maximizing the dot directly.
//
// The support is then recomputed exactly as dot(normal, vertex), without the embedded index.
// todo consider using this for GJK
static inline void b3GetSupportWide( b3Vec3 normal, const float* vx, const float* vy, const float* vz, int n, float bias,
									 float* support, int* vertexIndex )
{
	const b3FloatW nx = b3SplatW( normal.x );
	const b3FloatW ny = b3SplatW( normal.y );
	const b3FloatW nz = b3SplatW( normal.z );
	const b3FloatW biasV = b3SplatW( bias );

	// Start the minimum at a large value.
	b3FloatW minValue = b3SplatW( B3_HUGE );

	// Tail lanes hold vertex 0 with index bits >= vertexCount, so they never become the min value.
	for ( int i = 0; i < n; i += 4 )
	{
		b3FloatW x = b3LoadW( vx + i );
		b3FloatW y = b3LoadW( vy + i );
		b3FloatW z = b3LoadW( vz + i );
		b3FloatW d = b3AddW( b3MulW( nz, z ), b3AddW( b3MulW( ny, y ), b3MulW( nx, x ) ) );

		// This is always positive.
		b3FloatW value = b3SubW( biasV, d );
		b3FloatW augmentedValue = b3EmbedIndexW( value, i, B3_HULL_BIT_COUNT );
		minValue = b3MinW( minValue, augmentedValue );
	}

	// One horizontal min, the winning lane's value and index bits ride through.
	int vi = b3MinIndexW( minValue, B3_HULL_BIT_COUNT );

	// Exact support for the chosen vertex.
	*vertexIndex = vi;

	// Dot product
	*support = normal.x * vx[vi] + normal.y * vy[vi] + normal.z * vz[vi];
}

#define NE ( B3_MAX_HULL_EDGES + 4 )
#define NF ( B3_MAX_HULL_FACES + 4 )
#define NV ( B3_MAX_HULL_VERTICES + 4 )

// SIMD separating axis test based on an implementation developed by Cairn Overturf.
// See his article: https://cairno.substack.com/p/improvements-to-the-separating-axis
b3AxisQuery b3ComputeSeparatingAxis( const b3HullData* hullA, const b3HullData* hullB, b3Transform xfB, bool earlyReturn )
{
	b3Matrix3 R = b3MakeMatrixFromQuat( xfB.q );
	b3Matrix3 invR = b3Transpose( R );

	float speculativeDistance = B3_SPECULATIVE_DISTANCE;

	b3AxisQuery res = {
		.faceA =
			{
				.normal = b3Vec3_zero,
				.separation = -INFINITY,
				.indexA = B3_NULL_INDEX,
				.indexB = B3_NULL_INDEX,
				.type = b3_faceAxisA,
			},
		.faceB =
			{
				.normal = b3Vec3_zero,
				.separation = -INFINITY,
				.indexA = B3_NULL_INDEX,
				.indexB = B3_NULL_INDEX,
				.type = b3_faceAxisB,
			},
		.edge =
			{
				.normal = b3Vec3_zero,
				.separation = -INFINITY,
				.indexA = B3_NULL_INDEX,
				.indexB = B3_NULL_INDEX,
				.type = b3_edgePairAxis,
			},
		.separatedFeature = b3_invalidAxis,
	};

	int faceCountA = hullA->faceCount;
	const b3Plane* planesA = b3GetHullPlanes( hullA );

	int soaVertexCountB = ( hullB->vertexCount + 3 ) & ~3;
	const float* vxB = b3GetHullSoaVertices( hullB );
	const float* vyB = vxB + soaVertexCountB;
	const float* vzB = vyB + soaVertexCountB;

	b3Vec3 cB = b3AABB_Center( hullB->aabb );
	b3Vec3 hB = b3AABB_Extents( hullB->aabb );

	// Test A's face planes against B's vertices.
	for ( int i = 0; i < faceCountA; ++i )
	{
		b3Plane plane = planesA[i];
		b3Vec3 direction = b3Neg( b3MulMV( invR, plane.normal ) );
		float planeSeparation = b3Dot( plane.normal, xfB.p ) - plane.offset;
		float biasB = b3Dot( direction, cB ) + 1.0625f * b3Dot( b3Abs( direction ), hB );
		float support;
		int vertexIndex;
		b3GetSupportWide( direction, vxB, vyB, vzB, soaVertexCountB, biasB, &support, &vertexIndex );
		float separation = planeSeparation - support;
		if ( separation > res.faceA.separation )
		{
			res.faceA.normal = plane.normal;
			res.faceA.separation = separation;
			res.faceA.indexA = i;
			res.faceA.indexB = vertexIndex;
			if ( separation > speculativeDistance && earlyReturn )
			{
				res.separatedFeature = b3_faceAxisA;
				return res;
			}
		}
	}

	int faceCountB = hullB->faceCount;
	const b3Plane* planesB = b3GetHullPlanes( hullB );

	int soaVertexCountA = ( hullA->vertexCount + 3 ) & ~3;
	const float* vxA = b3GetHullSoaVertices( hullA );
	const float* vyA = vxA + soaVertexCountA;
	const float* vzA = vyA + soaVertexCountA;

	b3Vec3 cA = b3AABB_Center( hullA->aabb );
	b3Vec3 hA = b3AABB_Extents( hullA->aabb );

	// Test B's face planes against A's vertices.
	for ( int i = 0; i < faceCountB; ++i )
	{
		b3Plane plane = planesB[i];
		b3Vec3 direction = b3Neg( b3MulMV( R, plane.normal ) );
		float planeSeparation = b3Dot( direction, xfB.p ) - plane.offset;
		float biasA = b3Dot( direction, cA ) + 1.0625f * b3Dot( b3Abs( direction ), hA );
		float support;
		int vertexIndex;
		b3GetSupportWide( direction, vxA, vyA, vzA, soaVertexCountA, biasA, &support, &vertexIndex );
		float separation = planeSeparation - support;
		if ( separation > res.faceB.separation )
		{
			res.faceB.normal = direction;
			res.faceB.separation = separation;
			res.faceB.indexA = vertexIndex;
			res.faceB.indexB = i;
			// This points from A to B and is in frame A
			if ( separation > speculativeDistance && earlyReturn )
			{
				res.separatedFeature = b3_faceAxisB;
				return res;
			}
		}
	}

	// Transform B into A's space once, into SoA arrays. Extra space so
	// tail can be set to zero in all cases.
	_Static_assert( ( B3_MAX_HULL_EDGES & 3 ) == 0, "must be multiple of 4" );
	_Static_assert( ( B3_MAX_HULL_FACES & 3 ) == 0, "must be multiple of 4" );
	_Static_assert( ( B3_MAX_HULL_VERTICES & 3 ) == 0, "must be multiple of 4" );

	// The alignments below are not necessary, but they don't hurt.

	// B face normals in A space, negated.
	_Alignas( 16 ) float bFNx[NF];
	_Alignas( 16 ) float bFNy[NF];
	_Alignas( 16 ) float bFNz[NF];

	// B vertices in A space, negated.
	_Alignas( 16 ) float bWx[NV];
	_Alignas( 16 ) float bWy[NV];
	_Alignas( 16 ) float bWz[NV];

	int soaFaceCountB = ( faceCountB + 3 ) & ~3;
	const float* nxB = b3GetHullSoaNormals( hullB );
	const float* nyB = nxB + soaFaceCountB;
	const float* nzB = nyB + soaFaceCountB;

	b3NegativeTransformFromSoA( &R, xfB.p, nxB, nyB, nzB, soaFaceCountB, bFNx, bFNy, bFNz, false );
	b3NegativeTransformFromSoA( &R, xfB.p, vxB, vyB, vzB, soaVertexCountB, bWx, bWy, bWz, true );

	// Per B edge data. C and D are the two face normals, v0 a vertex, DC the edge vector.
	_Alignas( 16 ) float bCx[NE];
	_Alignas( 16 ) float bCy[NE];
	_Alignas( 16 ) float bCz[NE];
	_Alignas( 16 ) float bDx[NE];
	_Alignas( 16 ) float bDy[NE];
	_Alignas( 16 ) float bDz[NE];
	_Alignas( 16 ) float bV0x[NE];
	_Alignas( 16 ) float bV0y[NE];
	_Alignas( 16 ) float bV0z[NE];
	_Alignas( 16 ) float bDCx[NE];
	_Alignas( 16 ) float bDCy[NE];
	_Alignas( 16 ) float bDCz[NE];

	int halfEdgeCountB = hullB->edgeCount;
	const b3HullHalfEdge* halfEdgesB = b3GetHullEdges( hullB );
	int nb = 0;
	for ( int i = 0; i < halfEdgeCountB; i += 2 )
	{
		const b3HullHalfEdge* edge = halfEdgesB + i;
		const b3HullHalfEdge* twin = edge + 1;
		int f0 = edge->face;
		int f1 = twin->face;
		int v0 = edge->origin;
		int v1 = twin->origin;

		bCx[nb] = bFNx[f0];
		bCy[nb] = bFNy[f0];
		bCz[nb] = bFNz[f0];
		bDx[nb] = bFNx[f1];
		bDy[nb] = bFNy[f1];
		bDz[nb] = bFNz[f1];
		bV0x[nb] = bWx[v0];
		bV0y[nb] = bWy[v0];
		bV0z[nb] = bWz[v0];
		bDCx[nb] = bWx[v1] - bWx[v0];
		bDCy[nb] = bWy[v1] - bWy[v0];
		bDCz[nb] = bWz[v1] - bWz[v0];
		nb += 1;
	}

	// Per A edge data, already in A's space so just gathered. n0 and n1 are the two face
	// normals, d the edge vector av1-av0, v0 the first vertex. Tol is the
	// parallel edge tolerance, scaled by the edge length.
	_Alignas( 16 ) float aN0x[NE];
	_Alignas( 16 ) float aN0y[NE];
	_Alignas( 16 ) float aN0z[NE];
	_Alignas( 16 ) float aN1x[NE];
	_Alignas( 16 ) float aN1y[NE];
	_Alignas( 16 ) float aN1z[NE];
	// dir = av1 - av0
	_Alignas( 16 ) float aDx[NE];
	_Alignas( 16 ) float aDy[NE];
	_Alignas( 16 ) float aDz[NE];
	_Alignas( 16 ) float aV0x[NE];
	_Alignas( 16 ) float aV0y[NE];
	_Alignas( 16 ) float aV0z[NE];
	_Alignas( 16 ) float aTol[NE];

	int halfEdgeCountA = hullA->edgeCount;
	const b3HullHalfEdge* halfEdgesA = b3GetHullEdges( hullA );
	int na = 0;

	float squaredTol = B3_PARALLEL_EDGE_TOL * B3_PARALLEL_EDGE_TOL;
	for ( int i = 0; i < halfEdgeCountA; i += 2 )
	{
		const b3HullHalfEdge* edge = halfEdgesA + i;
		const b3HullHalfEdge* twin = edge + 1;

		b3Vec3 A = planesA[edge->face].normal;
		b3Vec3 B = planesA[twin->face].normal;
		aN0x[na] = A.x;
		aN0y[na] = A.y;
		aN0z[na] = A.z;
		aN1x[na] = B.x;
		aN1y[na] = B.y;
		aN1z[na] = B.z;

		int v0 = edge->origin;
		int v1 = twin->origin;

		aDx[na] = vxA[v1] - vxA[v0];
		aDy[na] = vyA[v1] - vyA[v0];
		aDz[na] = vzA[v1] - vzA[v0];
		aV0x[na] = vxA[v0];
		aV0y[na] = vyA[v0];
		aV0z[na] = vzA[v0];

		aTol[na] = squaredTol * ( aDx[na] * aDx[na] + aDy[na] * aDy[na] + aDz[na] * aDz[na] );
		na += 1;
	}

	// Zero the tail lanes.
	b3FloatW zero = b3ZeroW();
	b3StoreW( aN0x + na, zero );
	b3StoreW( aN0y + na, zero );
	b3StoreW( aN0z + na, zero );
	b3StoreW( aN1x + na, zero );
	b3StoreW( aN1y + na, zero );
	b3StoreW( aN1z + na, zero );
	b3StoreW( aDx + na, zero );
	b3StoreW( aDy + na, zero );
	b3StoreW( aDz + na, zero );
	b3StoreW( aV0x + na, zero );
	b3StoreW( aV0y + na, zero );
	b3StoreW( aV0z + na, zero );
	b3StoreW( aTol + na, zero );

	int edgeCountB = halfEdgeCountB / 2;

#if defined( B3_SIMD_NONE )

	// The SIMD emulated version of this code is very slow. This is a purely scalar version
	// for platforms that don't have SIMD capability. It is much faster than SIMD emulation.
	// WARNING: this math needs to match the SIMD version for cross platform determinism.

	const float EPS = -0.0001f;

	for ( int j = 0; j < edgeCountB; ++j )
	{
		float Cx = bCx[j];
		float Cy = bCy[j];
		float Cz = bCz[j];
		float Dx = bDx[j];
		float Dy = bDy[j];
		float Dz = bDz[j];
		float DCx = bDCx[j];
		float DCy = bDCy[j];
		float DCz = bDCz[j];
		float bv0x = bV0x[j];
		float bv0y = bV0y[j];
		float bv0z = bV0z[j];

		for ( int i = 0; i < na; ++i )
		{
			// CBA = C.dir, DBA = D.dir, where dir = B_x_A
			float CBA = Cx * aDx[i] + ( Cy * aDy[i] + Cz * aDz[i] );
			float DBA = Dx * aDx[i] + ( Dy * aDy[i] + Dz * aDz[i] );
			if ( CBA * DBA >= EPS )
			{
				continue;
			}

			// ADC = n0.DC, BDC = n1.DC, where DC = D_x_C
			float ADC = aN0x[i] * DCx + ( aN0y[i] * DCy + aN0z[i] * DCz );
			float BDC = aN1x[i] * DCx + ( aN1y[i] * DCy + aN1z[i] * DCz );
			if ( ADC * BDC >= EPS || CBA * BDC >= EPS )
			{
				continue;
			}

			// Reject near parallel edges
			float maxCD = b3MaxFloat( CBA * CBA, DBA * DBA );
			if ( maxCD <= aTol[i] )
			{
				continue;
			}

			// t = -CBA / (DBA - CBA)
			float t = -CBA / ( DBA - CBA );

			// normal = lerp(t, C, D) = C + (D-C)*t
			float nx = Cx + t * ( Dx - Cx );
			float ny = Cy + t * ( Dy - Cy );
			float nz = Cz + t * ( Dz - Cz );
			float len2 = nx * nx + ( ny * ny + nz * nz );
			float inv = 1.0f / sqrtf( len2 );
			nx *= inv;
			ny *= inv;
			nz *= inv;

			// separation = -dot(normal, av0 + bv0)
			float sx = aV0x[i] + bv0x;
			float sy = aV0y[i] + bv0y;
			float sz = aV0z[i] + bv0z;

			float separation = -( sx * nx + ( sy * ny + sz * nz ) );
			if ( separation > res.edge.separation )
			{
				res.edge.normal = (b3Vec3){ nx, ny, nz };
				res.edge.separation = separation;

				// Half edge index
				res.edge.indexA = 2 * i;
				res.edge.indexB = 2 * j;

				if ( separation > speculativeDistance && earlyReturn )
				{
					res.separatedFeature = b3_edgePairAxis;
					return res;
				}
			}
		}
	}

#else

	// Edge phase, one B edge against four A edges at a time, no transforms in the loop.
	const b3FloatW EPS = b3SplatW( -0.0001f );
	const b3FloatW INF = b3SplatW( INFINITY );

	for ( int j = 0; j < edgeCountB; ++j )
	{
		const b3FloatW Cx = b3SplatW( bCx[j] );
		const b3FloatW Cy = b3SplatW( bCy[j] );
		const b3FloatW Cz = b3SplatW( bCz[j] );
		const b3FloatW Dx = b3SplatW( bDx[j] );
		const b3FloatW Dy = b3SplatW( bDy[j] );
		const b3FloatW Dz = b3SplatW( bDz[j] );
		const b3FloatW DCx = b3SplatW( bDCx[j] );
		const b3FloatW DCy = b3SplatW( bDCy[j] );
		const b3FloatW DCz = b3SplatW( bDCz[j] );
		const b3FloatW bv0x = b3SplatW( bV0x[j] );
		const b3FloatW bv0y = b3SplatW( bV0y[j] );
		const b3FloatW bv0z = b3SplatW( bV0z[j] );

		for ( int i = 0; i < na; i += 4 )
		{
			b3FloatW n0x = b3LoadW( aN0x + i );
			b3FloatW n0y = b3LoadW( aN0y + i );
			b3FloatW n0z = b3LoadW( aN0z + i );
			b3FloatW n1x = b3LoadW( aN1x + i );
			b3FloatW n1y = b3LoadW( aN1y + i );
			b3FloatW n1z = b3LoadW( aN1z + i );
			b3FloatW dx = b3LoadW( aDx + i );
			b3FloatW dy = b3LoadW( aDy + i );
			b3FloatW dz = b3LoadW( aDz + i );
			b3FloatW v0x = b3LoadW( aV0x + i );
			b3FloatW v0y = b3LoadW( aV0y + i );
			b3FloatW v0z = b3LoadW( aV0z + i );
			b3FloatW tol = b3LoadW( aTol + i );

			// CBA = C.dir, DBA = D.dir, where dir = B_x_A
			b3FloatW CBA = b3Dot3W( Cx, Cy, Cz, dx, dy, dz );
			b3FloatW DBA = b3Dot3W( Dx, Dy, Dz, dx, dy, dz );
			// ADC = n0.DC, BDC = n1.DC, where DC = D_x_C
			b3FloatW ADC = b3Dot3W( n0x, n0y, n0z, DCx, DCy, DCz );
			b3FloatW BDC = b3Dot3W( n1x, n1y, n1z, DCx, DCy, DCz );

			// Gauss map arc crossing test, CBA*DBA<eps and ADC*BDC<eps and CBA*BDC<eps
			b3FloatW m1 = b3LessThanW( b3MulW( CBA, DBA ), EPS );
			b3FloatW m2 = b3LessThanW( b3MulW( ADC, BDC ), EPS );
			b3FloatW m3 = b3LessThanW( b3MulW( CBA, BDC ), EPS );

			// Reject near parallel edges. The arc lerp is ill conditioned when both of B's normals are nearly
			// perpendicular to edge A, a scale invariant sine threshold relative to the edge length.
			b3FloatW maxCD = b3MaxW( b3MulW( CBA, CBA ), b3MulW( DBA, DBA ) );
			b3FloatW notParallel = b3GreaterThanW( maxCD, tol );
			b3FloatW mask = b3AndW( b3AndW( m1, m2 ), b3AndW( m3, notParallel ) );

			// Most A-edges fail the Gauss test, so skip the divide, sqrt and support work when no
			// lane passed.
			if ( b3AnyTrueW( mask ) == false )
			{
				continue;
			}

			// t = -CBA / (DBA - CBA)
			b3FloatW t = b3DivW( b3SubW( zero, CBA ), b3SubW( DBA, CBA ) );

			// normal = lerp(t, C, D) = C + (D-C)*t
			b3FloatW nx = b3MulAddW( Cx, t, b3SubW( Dx, Cx ) );
			b3FloatW ny = b3MulAddW( Cy, t, b3SubW( Dy, Cy ) );
			b3FloatW nz = b3MulAddW( Cz, t, b3SubW( Dz, Cz ) );

			// normalize
			b3FloatW len2 = b3Dot3W( nx, ny, nz, nx, ny, nz );
			b3FloatW inv = b3DivW( b3SplatW( 1.0f ), b3SqrtW( len2 ) );
			nx = b3MulW( nx, inv );
			ny = b3MulW( ny, inv );
			nz = b3MulW( nz, inv );

			// support = dot(normal, av0 + bv0)
			b3FloatW sx = b3AddW( v0x, bv0x );
			b3FloatW sy = b3AddW( v0y, bv0y );
			b3FloatW sz = b3AddW( v0z, bv0z );
			b3FloatW support = b3Dot3W( sx, sy, sz, nx, ny, nz );

			// Lanes that fail the Gauss test can never win.
			support = b3BlendW( INF, support, mask );
			b3FloatW separation = b3SubW( zero, support );

			// Test all 4 supports against the running best at once. If none beats it, skip the
			// store and scalar reduction. res->support only turns negative just before returning,
			// so this never skips a lane that would trigger the early out.
			b3FloatW improves = b3GreaterThanW( separation, b3SplatW( res.edge.separation ) );
			if ( b3AnyTrueW( improves ) == false )
			{
				continue;
			}

			_Alignas( 16 ) float sA[4];
			_Alignas( 16 ) float nxA[4];
			_Alignas( 16 ) float nyA[4];
			_Alignas( 16 ) float nzA[4];
			b3StoreW( sA, separation );
			b3StoreW( nxA, nx );
			b3StoreW( nyA, ny );
			b3StoreW( nzA, nz );

			// Reduce in lane order so ties keep the first edge and the early out takes the first
			// improving support below zero. Padded tail lanes carry +INF support, so they never
			// update or index a->edges out of range.
			for ( int lane = 0; lane < 4; lane++ )
			{
				int ei = i + lane;
				float s = sA[lane];
				if ( s > res.edge.separation )
				{
					res.edge.normal = (b3Vec3){ nxA[lane], nyA[lane], nzA[lane] };
					res.edge.separation = s;

					// Half edge index
					res.edge.indexA = 2 * ei;
					res.edge.indexB = 2 * j;

					if ( s > speculativeDistance && earlyReturn )
					{
						res.separatedFeature = b3_edgePairAxis;
						return res;
					}
				}
			}
		}
	}
#endif

	return res;
}

#undef NE
#undef NF
#undef NV

#define B3_SIMD_COLLIDE_HULLS 1

#if B3_SIMD_COLLIDE_HULLS == 1





































































































































































































































































































































#else

// todo this code has gone stale, will be deleted soon

typedef struct b3FaceQuery
{
	float separation;
	int faceIndex;
	int vertexIndex;
} b3FaceQuery;

typedef struct b3EdgeQuery
{
	b3Vec3 normal;
	float separation;
	int indexA;
	int indexB;
} b3EdgeQuery;

// Old non-SIMD version. Keeping this for testing and comparisons

static b3FaceQuery b3QueryFaceDirections( const b3HullData* hullA, const b3HullData* hullB, b3Transform relativeTransform )
{
	// We perform all computations in local space of the second hull
	b3Transform transform = b3InvertTransform( relativeTransform );
	const b3Plane* planesA = b3GetHullPlanes( hullA );
	const b3Vec3* pointsB = b3GetHullPoints( hullB );

	int maxFaceIndex = -1;
	int maxVertexIndex = -1;
	float maxFaceSeparation = -FLT_MAX;
	float speculativeDistance = B3_SPECULATIVE_DISTANCE;

	for ( int faceIndex = 0; faceIndex < hullA->faceCount; ++faceIndex )
	{
		b3Plane plane = b3TransformPlane( transform, planesA[faceIndex] );
		int vertexIndex = b3FindHullSupportVertex( hullB, b3Neg( plane.normal ) );
		b3Vec3 support = pointsB[vertexIndex];

		float separation = b3PlaneSeparation( plane, support );
		if ( separation > maxFaceSeparation )
		{
			maxFaceIndex = faceIndex;
			maxVertexIndex = vertexIndex;
			maxFaceSeparation = separation;

			if ( separation >= speculativeDistance )
			{
				return (b3FaceQuery){
					.separation = maxFaceSeparation,
					.faceIndex = (uint8_t)maxFaceIndex,
					.vertexIndex = (uint8_t)maxVertexIndex,
				};
			}
		}
	}

	return (b3FaceQuery){
		.separation = maxFaceSeparation,
		.faceIndex = (uint8_t)maxFaceIndex,
		.vertexIndex = (uint8_t)maxVertexIndex,
	};
}

static b3EdgeQuery b3QueryEdgeDirections( const b3HullData* hullA, const b3HullData* hullB, b3Transform transformBtoA )
{
	// Find axis of minimum penetration
	b3Vec3 maxNormal = b3Vec3_zero;
	float maxSeparation = -FLT_MAX;
	int maxIndexA = B3_NULL_INDEX;
	int maxIndexB = B3_NULL_INDEX;

	const b3HullHalfEdge* edgesA = b3GetHullEdges( hullA );
	const b3Vec3* pointsA = b3GetHullPoints( hullA );
	const b3Plane* planesA = b3GetHullPlanes( hullA );
	const b3HullHalfEdge* edgesB = b3GetHullEdges( hullB );
	const b3Vec3* pointsB = b3GetHullPoints( hullB );
	const b3Plane* planesB = b3GetHullPlanes( hullB );

	// Work in frame A
	b3Matrix3 matrix = b3MakeMatrixFromQuat( transformBtoA.q );
	float speculativeDistance = B3_SPECULATIVE_DISTANCE;
	float squaredTolerance = B3_PARALLEL_EDGE_TOL * B3_PARALLEL_EDGE_TOL;

	// Arranged to minimize transform operations
	for ( int indexB = 0; indexB < hullB->edgeCount; indexB += 2 )
	{
		const b3HullHalfEdge* edgeB = edgesB + indexB;
		const b3HullHalfEdge* twinB = edgesB + indexB + 1;
		B3_ASSERT( edgeB->twin == indexB + 1 && twinB->twin == indexB );

		b3Vec3 qB = pointsB[twinB->origin];
		b3Vec3 eB = b3MulMV( matrix, b3Sub( qB, pointsB[edgeB->origin] ) );
		qB = b3Add( b3MulMV( matrix, qB ), transformBtoA.p );

		b3Vec3 uB = b3MulMV( matrix, planesB[edgeB->face].normal );
		b3Vec3 vB = b3MulMV( matrix, planesB[twinB->face].normal );

		for ( int indexA = 0; indexA < hullA->edgeCount; indexA += 2 )
		{
			const b3HullHalfEdge* edgeA = edgesA + indexA;
			const b3HullHalfEdge* twinA = edgesA + indexA + 1;
			B3_ASSERT( edgeA->twin == indexA + 1 && twinA->twin == indexA );

			b3Vec3 qA = pointsA[twinA->origin];
			b3Vec3 eA = b3Sub( qA, pointsA[edgeA->origin] );
			b3Vec3 uA = planesA[edgeA->face].normal;
			b3Vec3 vA = planesA[twinA->face].normal;

			// See "Collision Detection of Convex Polyhedra Based on Duality Transformation"
			// Two edges build a face on the Minkowski sum if the associated arcs AB and CD intersect on the Gauss map.
			// The associated arcs are defined by the adjacent face normals of each edge.

			// These are signed volumes with an edge optimization to avoid cross products
			// eA parallel to cross(vA, uA)
			// eB parallel to cross(vB, uB)
			// Since only signs are tested, length doesn't matter.

			float cba = b3Dot( uB, eA );
			float dba = b3Dot( vB, eA );
			float adc = -b3Dot( uA, eB );
			float bdc = -b3Dot( vA, eB );

			if ( cba * dba < 0.0f && adc * bdc < 0.0f && cba * bdc > 0.0f )
			{
				// Avoid nearly parallel edges that may lead to invalid separation values at the noise floor.
				if ( b3MaxFloat( cba * cba, dba * dba ) < squaredTolerance * b3LengthSquared( eA ) )
				{
					continue;
				}

				// The intersection of the arcs on the Gauss map is the edge pair axis. Cast the
				// arc of hull B (from uB to vB) against the plane containing the arc of hull A:
				// dot(uB + t * (vB - uB), eA) == 0
				// then
				// t = cba / (cba - dba)
				//
				// The signs of cba and dba differ (Minkowski test), so the division is safe.
				//
				// The axis generated points from B to A by construction since it lands between
				// two face normals on B. This removes the need to orient the separation axis
				// using the hull centers.
				//
				// The axis is perpendicular to both edges so I can use qA and qB as arbitrary
				// points on edgeA and edgeB to measure the separation.
				float t = cba / ( cba - dba );
				b3Vec3 axis = b3Lerp( uB, vB, t );
				B3_VALIDATE( b3LengthSquared( axis ) > 1000.0f * FLT_MIN );
				axis = b3Normalize( axis );
				float separation = b3Dot( axis, b3Sub( qA, qB ) );

				if ( separation > maxSeparation )
				{
					// Continues to find the maximum separating axis
					// Flip normal so it points from A to B
					maxNormal = b3Neg( axis );
					maxSeparation = separation;
					maxIndexA = indexA;
					maxIndexB = indexB;

					if ( separation >= speculativeDistance )
					{
						// Cache hit, shapes are separated
						return (b3EdgeQuery){
							.normal = maxNormal,
							.separation = maxSeparation,
							.indexA = maxIndexA,
							.indexB = maxIndexB,
						};
					}
				}
			}
		}
	}

	return (b3EdgeQuery){
		.normal = maxNormal,
		.separation = maxSeparation,
		.indexA = maxIndexA,
		.indexB = maxIndexB,
	};
}



















































































































































































































































































































#endif
