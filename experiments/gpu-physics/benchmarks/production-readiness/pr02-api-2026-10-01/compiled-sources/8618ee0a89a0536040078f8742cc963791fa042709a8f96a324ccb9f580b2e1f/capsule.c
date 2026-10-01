// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#include "math_internal.h"
#include "shape.h"

#include "box3d/base.h"
#include "box3d/collision.h"
#include "box3d/constants.h"



























































b3AABB b3ComputeSweptCapsuleAABB( const b3Capsule* shape, b3Transform xf1, b3Transform xf2 )
{
	b3Vec3 r = { shape->radius, shape->radius, shape->radius };
	b3Vec3 a = b3TransformPoint( xf1, shape->center1 );
	b3Vec3 b = b3TransformPoint( xf1, shape->center2 );
	b3Vec3 c = b3TransformPoint( xf2, shape->center1 );
	b3Vec3 d = b3TransformPoint( xf2, shape->center2 );

	b3AABB aabb = {
		.lowerBound = b3Sub( b3Min( b3Min( a, b ), b3Min( c, d ) ), r ),
		.upperBound = b3Add( b3Max( b3Max( a, b ), b3Max( c, d ) ), r ),
	};
	return aabb;
}














// Precision Improvements for Ray / Sphere Intersection - Ray Tracing Gems 2019
// http://www.codercorner.com/blog/?p=321

































































































































































































































































int b3CollideMoverAndCapsule( b3PlaneResult* result, const b3Capsule* shape, const b3Capsule* mover )
{
	float totalRadius = mover->radius + shape->radius;

	b3SegmentDistanceResult approach = b3SegmentDistance( shape->center1, shape->center2, mover->center1, mover->center2 );

	// The normal points from the shape toward the mover.
	float distance;
	b3Vec3 normal = b3GetLengthAndNormalize( &distance, b3Sub( approach.point2, approach.point1 ) );

	if ( distance > totalRadius )
	{
		return 0;
	}

	float linearSlop = B3_LINEAR_SLOP;
	if ( distance < linearSlop )
	{
		// Deep overlap: the core segments intersect. Pick an arbitrary direction perpendicular
		// the to capsule axis.
		float moverLength;
		b3Vec3 moverAxis = b3GetLengthAndNormalize( &moverLength, b3Sub( mover->center2, mover->center1 ) );
		normal = moverLength > linearSlop ? b3Perp( moverAxis ) : b3Vec3_axisY;
		distance = 0.0f;
	}

	b3Plane plane = { normal, totalRadius - distance };
	*result = (b3PlaneResult){ plane, approach.point1 };
	return 1;
}
