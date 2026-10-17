// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#include "box3d/types.h"

#include "core.h"

#include "box3d/constants.h"










































































static void b3EmptyDrawShape( void* userShape, b3WorldTransform transform, b3HexColor color, void* context )
{
	B3_UNUSED( userShape, transform, color, context );
}

static void b3EmptyDrawSegment( b3Pos p1, b3Pos p2, b3HexColor color, void* context )
{
	B3_UNUSED( p1, p2, color, context );
}

static void b3EmptyDrawTransform( b3WorldTransform transform, void* context )
{
	B3_UNUSED( transform, context );
}

static void b3EmptyDrawPoint( b3Pos p, float size, b3HexColor color, void* context )
{
	B3_UNUSED( p, size, color, context );
}

static void b3EmptyDrawSphere( b3Pos p, float radius, b3HexColor color, float alpha, void* context )
{
	B3_UNUSED( p, radius, color, alpha, context );
}

static void b3EmptyDrawCapsule( b3Pos p1, b3Pos p2, float radius, b3HexColor color, float alpha, void* context )
{
	B3_UNUSED( p1, p2, radius, color, alpha, context );
}

static void b3EmptyDrawBounds( b3AABB aabb, b3HexColor color, void* context )
{
	B3_UNUSED( aabb, color, context );
}

static void b3EmptyDrawBox( b3Vec3 extents, b3WorldTransform transform, b3HexColor color, void* context )
{
	B3_UNUSED( extents, transform, color, context );
}

static void b3EmptyDrawString( b3Pos p, const char* s, b3HexColor color, void* context )
{
	B3_UNUSED( p, s, color, context );
}

b3DebugDraw b3DefaultDebugDraw( void )
{
	b3DebugDraw draw = { 0 };

	// These allow the user to skip some implementations and not hit null exceptions.
	draw.DrawShapeFcn = b3EmptyDrawShape;
	draw.DrawSegmentFcn = b3EmptyDrawSegment;
	draw.DrawTransformFcn = b3EmptyDrawTransform;
	draw.DrawPointFcn = b3EmptyDrawPoint;
	draw.DrawSphereFcn = b3EmptyDrawSphere;
	draw.DrawCapsuleFcn = b3EmptyDrawCapsule;
	draw.DrawBoundsFcn = b3EmptyDrawBounds;
	draw.DrawBoxFcn = b3EmptyDrawBox;
	draw.DrawStringFcn = b3EmptyDrawString;

	// Not too small, not too big.
	float h = 100.0f * b3GetLengthUnitsPerMeter();
	draw.drawingBounds = (b3AABB){
		.lowerBound = { -h, -h, -h },
		.upperBound = { h, h, h },
	};

	draw.jointScale = 1.0f;
	draw.forceScale = 1.0f;

	return draw;
}
