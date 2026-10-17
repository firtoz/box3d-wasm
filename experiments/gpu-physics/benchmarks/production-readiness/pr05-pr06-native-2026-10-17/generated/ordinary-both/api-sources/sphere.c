// SPDX-FileCopyrightText: 2026 Erin Catto
// SPDX-License-Identifier: MIT

// Dirk Gregorius contributed portions of this code

#include "math_internal.h"
#include "shape.h"

#include "box3d/base.h"
#include "box3d/collision.h"
#include "box3d/constants.h"



























b3AABB b3ComputeSweptSphereAABB( const b3Sphere* shape, b3Transform xf1, b3Transform xf2 )
{
	b3Vec3 r = { shape->radius, shape->radius, shape->radius };
	b3Vec3 center1 = b3TransformPoint( xf1, shape->center );
	b3Vec3 center2 = b3TransformPoint( xf2, shape->center );
	return ( b3AABB ){ b3Sub( b3Min( center1, center2 ), r ), b3Add( b3Max( center1, center2 ), r ) };
}














// Precision Improvements for Ray / Sphere Intersection - Ray Tracing Gems 2019
// http://www.codercorner.com/blog/?p=321

















































































// Precision Improvements for Ray / Sphere Intersection - Ray Tracing Gems 2019
// http://www.codercorner.com/blog/?p=321
// This will do interior hits.









































































int b3CollideMoverAndSphere( b3PlaneResult* result, const b3Sphere* shape, const b3Capsule* mover )
{
	float totalRadius = mover->radius + shape->radius;
	b3Vec3 closest = b3PointToSegmentDistance( mover->center1, mover->center2, shape->center );

	// The normal points from the sphere toward the mover.
	float distance;
	b3Vec3 normal = b3GetLengthAndNormalize( &distance, b3Sub( closest, shape->center ) );

	if ( distance > totalRadius )
	{
		return 0;
	}

	float linearSlop = B3_LINEAR_SLOP;
	if ( distance < linearSlop )
	{
		// Deep overlap: the mover axis passes through the sphere center, so no
		// direction is preferred. Push perpendicular to the mover axis.
		float length;
		b3Vec3 axis = b3GetLengthAndNormalize( &length, b3Sub( mover->center2, mover->center1 ) );
		normal = length > linearSlop ? b3Perp( axis ) : b3Vec3_axisY;
		distance = 0.0f;
	}

	b3Plane plane = { normal, totalRadius - distance };
	*result = ( b3PlaneResult ){ plane, shape->center };
	return 1;
}
