// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#include "aabb.h"
#include "algorithm.h"
#include "body.h"
#include "core.h"
#include "shape.h"
#include "simd.h"

#include "box3d/collision.h"
#include "box3d/constants.h"
#include "box3d/math_functions.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*
	Convention

	index = row * columnCount + column
	height = minHeight + heightScale * compressedHeights[index];

	column = index % columnCount;
	row = index / columnCount;

	x-axis : columns
	z-axis : rows

	00 --- 01 --- 02 --- 03 X
	|  0   |  1   |  2   |
	04 --- 05 --- 06 --- 07
	|  3   |  4   |  5   |
	08 --- 09 --- 10 --- 11
	|  6   |  7   |  8   |
	12 --- 13 --- 14 --- 15
	Z

	The quads exist before the column and row ends: row < rowCount - 1 and column < columnCount - 1
	quadIndex = row * (columnCount - 1) + column

	Quad origin index from quad index (needs row):
	index = quadIndex + row * columnCount

	Triangle index is related to the quad index
	triangleIndex = 2 * quadIndex + (0/1)
	quadIndex = triangleIndex / 2

	Row and column from quad index:
	row = quadIndex / (columnCount - 1)
	column = quadIndex - row * (columnCount - 1)

	The triangle diagonal is fixed.

	triangle0 = {00, 04, 01} -> {11, 21, 12}
	triangle1 = {04, 05, 01} -> {22, 12, 21}

	11      12
	00 ---- 01
	|     / |
	| 0 / 1 | 1
	| /     |
	04 ---- 05
	21     22

	For adjacency we have

	   NA
	   00 ---- 01 ---- 02
	   |     / |     / |
	NA | 0 / 1 | 2 / 3 |
	   | /     | /     |
	   04 ---- 05 ---- 06
	   |     / |     / |
	NA | 6 / 7 | 8 / 9 |
	   | /     | /     |
	   08 ---- 09 ---- 10

	   0: NA, NA, 1
	   1: 0, 6, 2

	Triangle layouts

	   11  3  12
	   1 ----  3
	   |     /
	 1 | 0 / 2
	   | /
	   2
	   21

			  12
			  2
			/ |
		2 / 1 | 1
		/     |
	   3 ---- 1
	   21  3  22
 */
























































































































































































































































































































































































_Static_assert( b3_concaveEdge3 == 4 * b3_concaveEdge1, "bit math" );
_Static_assert( b3_inverseConcaveEdge3 == 4 * b3_inverseConcaveEdge1, "bit math" );

// Decode the four corner vertices of a height field cell into local space.
// Output order matches the index naming used throughout this file:
// corners[0] = (column, row), corners[1] = (column + 1, row),
// corners[2] = (column, row + 1), corners[3] = (column + 1, row + 1).
static inline void b3GetHeightFieldCellCorners( const b3HeightFieldData* hf, int row, int column, b3Vec3 corners[4] )
{
	B3_ASSERT( 0 <= row && row < hf->rowCount - 1 && 0 <= column && column < hf->columnCount - 1 );

	int columnCount = hf->columnCount;
	int index11 = row * columnCount + column;
	int index12 = index11 + 1;
	int index21 = ( row + 1 ) * columnCount + column;
	int index22 = index21 + 1;

	float minHeight = hf->minHeight;
	float heightScale = hf->heightScale;
	const uint16_t* heights = b3GetHeightFieldCompressedHeights( hf );

	float height11 = minHeight + heightScale * heights[index11];
	float height12 = minHeight + heightScale * heights[index12];
	float height21 = minHeight + heightScale * heights[index21];
	float height22 = minHeight + heightScale * heights[index22];

	float x1 = (float)( column );
	float x2 = (float)( column + 1 );
	float z1 = (float)( row );
	float z2 = (float)( row + 1 );

	b3Vec3 scale = hf->scale;
	corners[0] = b3Mul( scale, (b3Vec3){ x1, height11, z1 } );
	corners[1] = b3Mul( scale, (b3Vec3){ x2, height12, z1 } );
	corners[2] = b3Mul( scale, (b3Vec3){ x1, height21, z2 } );
	corners[3] = b3Mul( scale, (b3Vec3){ x2, height22, z2 } );
}

b3Triangle b3GetHeightFieldTriangle( const b3HeightFieldData* heightField, int triangleIndex )
{
	B3_ASSERT( 0 <= triangleIndex );
	B3_ASSERT( triangleIndex < 2 * ( heightField->columnCount - 1 ) * ( heightField->rowCount - 1 ) );

	b3Triangle triangle;
	triangle.flags = b3GetHeightFieldFlags( heightField )[triangleIndex];

	int columnCount = heightField->columnCount;
	int quadIndex = triangleIndex >> 1;
	int row = quadIndex / ( columnCount - 1 );
	int column = quadIndex - row * ( columnCount - 1 );

	int index11 = row * columnCount + column;
	int index12 = index11 + 1;
	int index21 = ( row + 1 ) * columnCount + column;
	int index22 = index21 + 1;

	int cellIndex = row * ( columnCount - 1 ) + column;

	B3_ASSERT( quadIndex == cellIndex );
	B3_ASSERT( b3GetHeightFieldMaterialIndices( heightField )[cellIndex] != B3_HEIGHT_FIELD_HOLE );
	B3_UNUSED( cellIndex );

	b3Vec3 corners[4];
	b3GetHeightFieldCellCorners( heightField, row, column, corners );

	if ( ( triangleIndex & 1 ) == 0 )
	{
		triangle.vertices[0] = corners[0];
		triangle.vertices[1] = corners[2];
		triangle.vertices[2] = corners[1];
		triangle.i1 = index11;
		triangle.i2 = index21;
		triangle.i3 = index12;
	}
	else
	{
		triangle.vertices[0] = corners[3];
		triangle.vertices[1] = corners[1];
		triangle.vertices[2] = corners[2];
		triangle.i1 = index22;
		triangle.i2 = index12;
		triangle.i3 = index21;
	}

	if ( heightField->clockwise )
	{
		B3_SWAP( triangle.vertices[1], triangle.vertices[2] );
		B3_SWAP( triangle.i2, triangle.i3 );

		// Reversing winding swaps edge1 and edge3; edge2 (the diagonal) is preserved.
		int flags = triangle.flags;
		int edge1Bits = flags & ( b3_concaveEdge1 | b3_inverseConcaveEdge1 );
		int edge3Bits = flags & ( b3_concaveEdge3 | b3_inverseConcaveEdge3 );
		flags &= ~( b3_concaveEdge1 | b3_concaveEdge3 | b3_inverseConcaveEdge1 | b3_inverseConcaveEdge3 );
		flags |= edge1Bits << 2;
		flags |= edge3Bits >> 2;
		triangle.flags = flags;
	}

	return triangle;
}

int b3GetHeightFieldMaterial( const b3HeightFieldData* heightField, int triangleIndex )
{
	B3_ASSERT( 0 <= triangleIndex );
	B3_ASSERT( triangleIndex < 2 * ( heightField->columnCount - 1 ) * ( heightField->rowCount - 1 ) );

	int cellIndex = triangleIndex >> 1;
	return b3GetHeightFieldMaterialIndices( heightField )[cellIndex];
}
















// todo advance cast to the grid border immediately if it starts outside the row/column range
// todo terminate the cast immediately if it leaves the row/column range






















































































































































































































































































































































































































































































































































































































int b3CollideMoverAndHeightField( b3PlaneResult* planes, int capacity, const b3HeightFieldData* shape, const b3Capsule* mover )
{
	b3DistanceInput distanceInput = { 0 };
	distanceInput.proxyB = (b3ShapeProxy){ &mover->center1, 2, 0.0f };
	distanceInput.transform = b3Transform_identity;
	distanceInput.useRadii = false;

	b3SimplexCache cache = { 0 };

	float radius = mover->radius;
	b3V32 center1 = b3LoadV( &mover->center1.x );
	b3V32 center2 = b3LoadV( &mover->center2.x );
	b3V32 r = b3SplatV( radius );
	b3V32 boundsMin = b3SubV( b3MinV( center1, center2 ), r );
	b3V32 boundsMax = b3AddV( b3MaxV( center1, center2 ), r );
	b3V32 boundsCenter = b3MulV( b3_halfV, b3AddV( boundsMin, boundsMax ) );
	b3V32 boundsExtent = b3SubV( boundsMax, boundsCenter );

	float localMinX = b3GetXV( boundsMin );
	float localMinZ = b3GetZV( boundsMin );
	float localMaxX = b3GetXV( boundsMax );
	float localMaxZ = b3GetZV( boundsMax );

	b3Vec3 scale = shape->scale;
	int minRow = (int)floorf( localMinZ / scale.z );
	int maxRow = (int)floorf( localMaxZ / scale.z );
	int minCol = (int)floorf( localMinX / scale.x );
	int maxCol = (int)floorf( localMaxX / scale.x );

	int planeCount = 0;

	// Outer loop on rows and inner loop on columns so that triangle indices
	// increase monotonically.
	for ( int row = minRow; row <= maxRow; ++row )
	{
		if ( row < 0 || shape->rowCount - 1 <= row )
		{
			continue;
		}

		for ( int column = minCol; column <= maxCol; ++column )
		{
			if ( column < 0 || shape->columnCount - 1 <= column )
			{
				continue;
			}

			int cellIndex = row * ( shape->columnCount - 1 ) + column;
			B3_ASSERT( cellIndex < ( shape->rowCount - 1 ) * ( shape->columnCount - 1 ) );
			uint8_t material = b3GetHeightFieldMaterialIndices( shape )[cellIndex];
			if ( material == B3_HEIGHT_FIELD_HOLE )
			{
				continue;
			}

			b3Vec3 corners[4];
			b3GetHeightFieldCellCorners( shape, row, column, corners );
			b3Vec3 point11 = corners[0];
			b3Vec3 point12 = corners[1];
			b3Vec3 point21 = corners[2];
			b3Vec3 point22 = corners[3];

			b3V32 v11 = b3LoadV( &point11.x );
			b3V32 v12 = b3LoadV( &point12.x );
			b3V32 v21 = b3LoadV( &point21.x );
			b3V32 v22 = b3LoadV( &point22.x );

			if ( b3TestBoundsTriangleOverlap( boundsCenter, boundsExtent, v11, v21, v12 ) )
			{
				b3Vec3 triangleVertices[] = { point11, point21, point12 };
				distanceInput.proxyA = (b3ShapeProxy){ triangleVertices, 3, 0.0f };

				// reset the cache
				cache.count = 0;

				// get distance between triangle and mover
				b3DistanceOutput distanceOutput = b3ShapeDistance( &distanceInput, &cache, NULL, 0 );

				if ( distanceOutput.distance == 0.0f )
				{
					// todo SAT
				}
				else if ( distanceOutput.distance <= mover->radius )
				{
					b3Plane plane = { distanceOutput.normal, mover->radius - distanceOutput.distance };
					planes[planeCount] = (b3PlaneResult){ plane, distanceOutput.pointA };
					planeCount += 1;

					if ( planeCount == capacity )
					{
						return planeCount;
					}
				}
			}

			if ( b3TestBoundsTriangleOverlap( boundsCenter, boundsExtent, v21, v22, v12 ) )
			{
				b3Vec3 triangleVertices[] = { point22, point12, point21 };
				distanceInput.proxyA = (b3ShapeProxy){ triangleVertices, 3, 0.0f };

				// reset the cache
				cache.count = 0;

				// get distance between triangle and mover
				b3DistanceOutput distanceOutput = b3ShapeDistance( &distanceInput, &cache, NULL, 0 );

				if ( distanceOutput.distance == 0.0f )
				{
					// todo SAT
				}
				else if ( distanceOutput.distance <= mover->radius )
				{
					b3Plane plane = { distanceOutput.normal, mover->radius - distanceOutput.distance };
					planes[planeCount] = (b3PlaneResult){ plane, distanceOutput.pointA };
					planeCount += 1;

					if ( planeCount == capacity )
					{
						return planeCount;
					}
				}
			}
		}
	}

	return planeCount;
}































































































































































#if defined( _MSC_VER )
#define B3_FILE_SCAN fscanf_s
#else
#define B3_FILE_SCAN fscanf
#endif





























































































