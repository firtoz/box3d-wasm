// SPDX-FileCopyrightText: 2026 Erin Catto
// SPDX-License-Identifier: MIT

// Dirk Gregorius contributed portions of this code

#include "container.h"
#include "math_internal.h"
#include "shape.h"
#include "simd.h"

#include "box3d/collision.h"
#include "box3d/constants.h"

#include <stdint.h>

b3DeclareArray( b3VertexNode );
b3DeclareArray( b3MeshNode );
b3DeclareArray( b3MeshTriangle );
b3DeclareArray( b3Vec3 );
b3DeclareArray( b3Primitive );
b3DeclareArrayNative( uint8_t );

#define B3_BIN_COUNT 8
#define B3_DESIRED_TRIANGLES_PER_LEAF 4
#define B3_LEAF_NODE 3
#define B3_MAXIMUM_TRIANGLES_PER_LEAF 8
#define B3_MESH_STACK_SIZE 256

static bool b3IsLeaf( const b3MeshNode* node )
{
	return node->data.asLeaf.type == B3_LEAF_NODE;
}

static b3MeshNode* b3GetMeshNodesWrite( b3MeshData* mesh )
{
	if ( mesh->nodeOffset == 0 )
	{
		return NULL;
	}

	return (b3MeshNode*)( (intptr_t)mesh + mesh->nodeOffset );
}

static b3MeshNode* b3GetLeftChildWrite( b3MeshNode* node )
{
	// The left child follows its parent.
	B3_ASSERT( !b3IsLeaf( node ) );
	return node + 1;
}

static const b3MeshNode* b3GetLeftChild( const b3MeshNode* node )
{
	// The left child follows its parent.
	B3_ASSERT( !b3IsLeaf( node ) );
	return node + 1;
}

static b3MeshNode* b3GetRightChildWrite( b3MeshNode* node )
{
	// We store the offset of the right child relative to its parent
	B3_ASSERT( !b3IsLeaf( node ) );
	return node + node->data.asNode.childOffset;
}

static const b3MeshNode* b3GetRightChild( const b3MeshNode* node )
{
	// We store the offset of the right child relative to its parent
	B3_ASSERT( !b3IsLeaf( node ) );
	return node + node->data.asNode.childOffset;
}

static const b3MeshNode* b3GetRoot( const b3MeshData* mesh )
{
	// The first node is the root
	return b3GetMeshNodes( mesh );
}

static b3MeshNode* b3GetRootWrite( b3MeshData* mesh )
{
	// The first node is the root
	return b3GetMeshNodesWrite( mesh );
}

static b3MeshTriangle* b3GetMeshTrianglesWrite( b3MeshData* mesh )
{
	if ( mesh->triangleOffset == 0 )
	{
		return NULL;
	}

	return (b3MeshTriangle*)( (intptr_t)mesh + mesh->triangleOffset );
}

static b3Vec3* b3GetMeshVerticesWrite( b3MeshData* mesh )
{
	if ( mesh->vertexOffset == 0 )
	{
		return NULL;
	}

	return (b3Vec3*)( (intptr_t)mesh + mesh->vertexOffset );
}

// static b3Vec3 b3GetVertex( b3MeshData& mesh, int vertexIndex )
//{
//	B3_ASSERT( 0 <= vertexIndex && vertexIndex < mesh.vertexCount );
//	b3Vec3* vertices = b3GetMeshVertices( &mesh );
//	return vertices[vertexIndex];
// }

static uint8_t* b3GetMeshMaterialIndicesWrite( b3MeshData* mesh )
{
	if ( mesh->materialOffset == 0 )
	{
		return NULL;
	}

	return (uint8_t*)( (intptr_t)mesh + mesh->materialOffset );
}

static uint8_t* b3GetMeshFlagsWrite( b3MeshData* mesh )
{
	if ( mesh->flagsOffset == 0 )
	{
		return NULL;
	}

	return (uint8_t*)( (intptr_t)mesh + mesh->flagsOffset );
}

static int b3GetNodeHeight( const b3MeshNode* node )
{
	if ( b3IsLeaf( node ) )
	{
		return 0;
	}

	const b3MeshNode* leftChild = b3GetLeftChild( node );
	int leftHeight = b3GetNodeHeight( leftChild );
	const b3MeshNode* rightChild = b3GetRightChild( node );
	int rightHeight = b3GetNodeHeight( rightChild );

	return 1 + b3MaxInt( leftHeight, rightHeight );
}












#if B3_ENABLE_VALIDATION == 1
static bool b3IsDegenerate( b3Vec3 v1, b3Vec3 v2, b3Vec3 v3, float minArea )
{
	b3Vec3 normal = b3Cross( b3Sub( v2, v1 ), b3Sub( v3, v1 ) );
	float lengthSq = b3LengthSquared( normal );
	return lengthSq < minArea * minArea;
}

static bool b3IsNonDegenerate( const b3MeshData* mesh, float minArea )
{
	const b3MeshTriangle* triangles = b3GetMeshTriangles( mesh );
	const b3Vec3* vertices = b3GetMeshVertices( mesh );

	// Check triangles
	for ( int index = 0; index < mesh->triangleCount; ++index )
	{
		// Index range
		b3MeshTriangle triangle = triangles[index];
		if ( triangle.index1 >= mesh->vertexCount )
		{
			return false;
		}

		if ( triangle.index2 >= mesh->vertexCount )
		{
			return false;
		}

		if ( triangle.index3 >= mesh->vertexCount )
		{
			return false;
		}

		// Degenerate topology
		if ( triangle.index1 == triangle.index2 )
		{
			return false;
		}
		if ( triangle.index1 == triangle.index3 )
		{
			return false;
		}
		if ( triangle.index2 == triangle.index3 )
		{
			return false;
		}

		// Degenerate geometry
		b3Vec3 vertex1 = vertices[triangle.index1];
		b3Vec3 vertex2 = vertices[triangle.index2];
		b3Vec3 vertex3 = vertices[triangle.index3];
		if ( b3IsDegenerate( vertex1, vertex2, vertex3, minArea ) )
		{
			return false;
		}
	}

	return true;
}

static inline b3AABB b3GetNodeAABB( const b3MeshNode* node )
{
	return (b3AABB){
		node->lowerBound,
		node->upperBound,
	};
}

static bool b3IsConsistent( const b3MeshData* mesh )
{
	const b3MeshTriangle* triangles = b3GetMeshTriangles( mesh );
	const b3Vec3* vertices = b3GetMeshVertices( mesh );

	// Check nodes
	int count = 0;
	const b3MeshNode* stack[64];
	stack[count++] = b3GetRoot( mesh );

	while ( count > 0 )
	{
		const b3MeshNode* node = stack[--count];
		b3AABB nodeBounds = b3GetNodeAABB( node );

		if ( b3IsLeaf( node ) == false )
		{
			const b3MeshNode* child1 = b3GetLeftChild( node );
			b3AABB bounds1 = b3GetNodeAABB( child1 );
			const b3MeshNode* child2 = b3GetRightChild( node );
			b3AABB bounds2 = b3GetNodeAABB( child2 );

			if ( !b3AABB_Contains( nodeBounds, bounds1 ) )
			{
				return false;
			}

			if ( !b3AABB_Contains( nodeBounds, bounds2 ) )
			{
				return false;
			}

			stack[count++] = child2;
			stack[count++] = child1;
		}
		else
		{
			b3AABB triangleBounds = B3_BOUNDS3_EMPTY;
			for ( uint32_t index = 0; index < node->data.asLeaf.triangleCount; ++index )
			{
				int triangleIndex = node->triangleOffset + index;
				B3_ASSERT( 0 <= triangleIndex && triangleIndex < mesh->triangleCount );

				b3MeshTriangle triangle = triangles[triangleIndex];

				b3AABB vertexBounds = B3_BOUNDS3_EMPTY;
				vertexBounds = b3AABB_AddPoint( vertexBounds, vertices[triangle.index1] );
				vertexBounds = b3AABB_AddPoint( vertexBounds, vertices[triangle.index2] );
				vertexBounds = b3AABB_AddPoint( vertexBounds, vertices[triangle.index3] );

				triangleBounds = b3AABB_Union( triangleBounds, vertexBounds );
			}

			if ( !b3AABB_Contains( nodeBounds, triangleBounds ) )
			{
				return false;
			}
		}
	}

	return true;
}

bool b3IsValidMesh( const b3MeshData* meshData )
{
	if ( meshData == NULL )
	{
		return false;
	}

	if ( meshData->version != B3_MESH_VERSION )
	{
		return false;
	}

	if ( meshData->byteCount < (int)sizeof( b3MeshData ) )
	{
		return false;
	}

	return b3IsConsistent( meshData );
}

#else

bool b3IsValidMesh( const b3MeshData* meshData )
{
	if ( meshData == NULL )
	{
		return false;
	}

	if ( meshData->version != B3_MESH_VERSION )
	{
		return false;
	}

	if ( meshData->byteCount < (int)sizeof( b3MeshData ) )
	{
		return false;
	}

	return true;
}

#endif

// Node for a vertex linked list
typedef struct b3VertexNode
{
	int32_t vertexIndex;
	int nextNodeIndex;
} b3VertexNode;

#define NAME b3VertexMap
#define KEY_TY uint64_t
#define VAL_TY int
#define HASH_FN vt_hash_integer
#define CMPR_FN vt_cmpr_integer
#define MALLOC_FN b3Alloc
#define FREE_FN b3Free
#include "verstable.h"

typedef struct b3SpatialHash
{
	b3Array( b3VertexNode ) nodes;
	const b3Vec3* vertices;
	int vertexCount;
	b3VertexMap vertexMap;
	float cellSize;
	float tolerance;
} b3SpatialHash;

static void b3SpatialHash_Create( b3SpatialHash* h, const b3Vec3* vertices, int vertexCount, float tolerance )
{
	h->vertices = vertices;
	h->vertexCount = vertexCount;
	h->tolerance = tolerance;
	h->cellSize = 2.0f * tolerance;
	b3Array_CreateN( h->nodes, vertexCount );

	b3VertexMap_init( &h->vertexMap );
	b3VertexMap_reserve( &h->vertexMap, vertexCount );

	B3_ASSERT( h->cellSize > 0.0f );
}

static void b3SpatialHash_Destroy( b3SpatialHash* h )
{
	b3VertexMap_cleanup( &h->vertexMap );
	b3Array_Destroy( h->nodes );
}

// Welding works by bucketing nearby vertices into identical keys in a hash table.
// Bucketing is done manually with an array.
static int32_t b3SpatialHash_FindDuplicate( b3SpatialHash* h, int32_t currentIndex )
{
	B3_ASSERT( currentIndex < h->vertexCount );
	b3Vec3 vertex = h->vertices[currentIndex];
	float cellSize = h->cellSize;
	float tolerance = h->tolerance;

	// Get the grid coordinates for the current vertex
	int32_t baseX = (int32_t)( floorf( vertex.x / cellSize ) );
	int32_t baseY = (int32_t)( floorf( vertex.y / cellSize ) );
	int32_t baseZ = (int32_t)( floorf( vertex.z / cellSize ) );

	// Check the current cell and all 26 neighboring cells (3x3x3 - 1)
	for ( int dx = -1; dx <= 1; ++dx )
	{
		for ( int dy = -1; dy <= 1; ++dy )
		{
			for ( int dz = -1; dz <= 1; ++dz )
			{
				int32_t x = baseX + dx;
				int32_t y = baseY + dy;
				int32_t z = baseZ + dz;

				// Compute hash for this neighboring cell (this is the key in the map)
				uint64_t key = 0;
				key ^= (uint64_t)( x ) + 0x9e3779b9 + ( key << 6 ) + ( key >> 2 );
				key ^= (uint64_t)( y ) + 0x9e3779b9 + ( key << 6 ) + ( key >> 2 );
				key ^= (uint64_t)( z ) + 0x9e3779b9 + ( key << 6 ) + ( key >> 2 );

				b3VertexMap_itr it = b3VertexMap_get( &h->vertexMap, key );
				if ( b3VertexMap_is_end( it ) == false )
				{
					// Check all vertices in this key
					int nodeIndex = it.data->val;

					while ( nodeIndex != B3_NULL_INDEX )
					{
						b3VertexNode node = h->nodes.data[nodeIndex];

						int32_t existingIndex = node.vertexIndex;
						B3_ASSERT( existingIndex < currentIndex );
						B3_ASSERT( existingIndex < h->vertexCount );

						b3Vec3 other = h->vertices[existingIndex];

						// IsEqual inlined: check if vertices are within tolerance
						if ( fabsf( vertex.x - other.x ) <= tolerance && fabsf( vertex.y - other.y ) <= tolerance &&
							 fabsf( vertex.z - other.z ) <= tolerance )
						{
							// Found duplicate
							return existingIndex;
						}

						nodeIndex = node.nextNodeIndex;
					}
				}
			}
		}
	}

	// No duplicate found, add to hash table
	uint64_t currentKey = 0;
	currentKey ^= (uint64_t)( baseX ) + 0x9e3779b9 + ( currentKey << 6 ) + ( currentKey >> 2 );
	currentKey ^= (uint64_t)( baseY ) + 0x9e3779b9 + ( currentKey << 6 ) + ( currentKey >> 2 );
	currentKey ^= (uint64_t)( baseZ ) + 0x9e3779b9 + ( currentKey << 6 ) + ( currentKey >> 2 );

	b3VertexMap_itr it = b3VertexMap_get( &h->vertexMap, currentKey );
	if ( b3VertexMap_is_end( it ) == false )
	{
		int nodeIndex = it.data->val;

		b3VertexNode node = {
			.vertexIndex = currentIndex,
			.nextNodeIndex = nodeIndex,
		};

		it.data->val = h->nodes.count;
		b3Array_Push( h->nodes, node );
	}
	else
	{
		b3VertexNode node = {
			.vertexIndex = currentIndex,
			.nextNodeIndex = B3_NULL_INDEX,
		};

		b3VertexMap_insert( &h->vertexMap, currentKey, h->nodes.count );
		b3Array_Push( h->nodes, node );
	}

	// Not welded
	return B3_NULL_INDEX;
}

typedef struct b3WeldData
{
	const b3Vec3* srcVertices;
	const int32_t* srcIndices;

	b3Vec3* dstVertices;
	int32_t* dstIndices;

	int vertexCount;
	int indexCount;
} b3WeldData;

static int b3WeldVertices( b3WeldData* data, float tolerance )
{
	int vertexCount = data->vertexCount;
	int uniqueCount = 0;

	// Create spatial hash and find duplicates
	b3SpatialHash spatialHash;
	b3SpatialHash_Create( &spatialHash, data->srcVertices, vertexCount, tolerance );
	b3Array( int ) vertexMapping = { 0 };
	b3Array_Resize( vertexMapping, vertexCount );

	for ( int i = 0; i < vertexCount; ++i )
	{
		int32_t duplicateIndex = b3SpatialHash_FindDuplicate( &spatialHash, i );

		if ( duplicateIndex == B3_NULL_INDEX )
		{
			// New unique vertex
			vertexMapping.data[i] = uniqueCount;
			data->dstVertices[uniqueCount] = data->srcVertices[i];
			uniqueCount += 1;
		}
		else
		{
			// Found duplicate, map to existing vertex
			vertexMapping.data[i] = vertexMapping.data[duplicateIndex];
		}
	}

	// Update indices to reference the new vertex array
	int indexCount = data->indexCount;
	for ( int i = 0; i < indexCount; ++i )
	{
		int srcIndex = data->srcIndices[i];
		B3_ASSERT( srcIndex < vertexCount );
		data->dstIndices[i] = vertexMapping.data[srcIndex];
	}

	b3SpatialHash_Destroy( &spatialHash );
	b3Array_Destroy( vertexMapping );

	return uniqueCount;
}

static inline void b3StoreLeaf( b3MeshNode* node, const b3AABB* aabb, int triangleCount, int triangleOffset )
{
	node->data.asLeaf.type = B3_LEAF_NODE;
	node->data.asLeaf.triangleCount = triangleCount;
	node->triangleOffset = triangleOffset;
	node->lowerBound = aabb->lowerBound;
	node->upperBound = aabb->upperBound;
}

typedef struct b3Primitive
{
	b3AABB aabb;
	b3Vec3 center;
	int triangleIndex;
} b3Primitive;

typedef struct b3Bucket
{
	int count;
	b3AABB bounds;
} b3Bucket;

typedef struct b3Split
{
	b3AABB leftBounds;
	b3AABB rightBounds;
	int axis;
	int index;
} b3Split;

static b3Split b3SplitBinnedSah( int count, b3Primitive* primitives )
{
	b3Split split;
	split.axis = -1;
	split.index = -1;

	// Compute bounds of primitive centroids and choose split axis
	b3AABB bounds = { primitives[0].center, primitives[0].center };
	for ( int i = 1; i < count; ++i )
	{
		bounds = b3AABB_AddPoint( bounds, primitives[i].center );
	}

	// Compute costs for splitting after each bucket and keep track of best split
	// This is a small O(n^2) loop. This can be further optimized, but it is already
	// very fast and is kept for simplicity right now.
	int bestBucket = -1;
	float bestCost = FLT_MAX;

	for ( int axis = 0; axis < 3; ++axis )
	{
		b3Vec3 extent = b3AABB_Extents( bounds );
		if ( b3GetByIndex( extent, axis ) < B3_LINEAR_SLOP )
		{
			continue;
		}

		// Initialize buckets
		b3Bucket buckets[B3_BIN_COUNT];
		for ( int i = 0; i < B3_BIN_COUNT; ++i )
		{
			buckets[i].count = 0;
			buckets[i].bounds = B3_BOUNDS3_EMPTY;
		}

		// Fill buckets
		float factor = B3_BIN_COUNT * ( 1.0f - FLT_EPSILON ) /
					   ( b3GetByIndex( bounds.upperBound, axis ) - b3GetByIndex( bounds.lowerBound, axis ) );
		for ( int i = 0; i < count; ++i )
		{
			b3Vec3 center = primitives[i].center;
			int index = (int)( factor * ( b3GetByIndex( center, axis ) - b3GetByIndex( bounds.lowerBound, axis ) ) );
			B3_ASSERT( 0 <= index && index < B3_BIN_COUNT );

			buckets[index].count++;
			buckets[index].bounds = b3AABB_Union( buckets[index].bounds, primitives[i].aabb );
		}

		// Evaluate splits
		for ( int i = 0; i < B3_BIN_COUNT - 1; ++i )
		{
			int leftCount = 0;
			b3AABB leftBounds = B3_BOUNDS3_EMPTY;
			for ( int k = 0; k <= i; ++k )
			{
				leftCount += buckets[k].count;
				leftBounds = b3AABB_Union( leftBounds, buckets[k].bounds );
			}

			int rightCount = 0;
			b3AABB rightBounds = B3_BOUNDS3_EMPTY;
			for ( int k = i + 1; k < B3_BIN_COUNT; ++k )
			{
				rightCount += buckets[k].count;
				rightBounds = b3AABB_Union( rightBounds, buckets[k].bounds );
			}

			B3_ASSERT( leftCount + rightCount == count );
			if ( leftCount > 0 && rightCount > 0 )
			{
				float cost = leftCount * b3AABB_Area( leftBounds ) + rightCount * b3AABB_Area( rightBounds );

				if ( cost < bestCost )
				{
					bestBucket = i;
					bestCost = cost;

					split.axis = axis;
					split.index = leftCount;
					split.leftBounds = leftBounds;
					split.rightBounds = rightBounds;
				}
			}
		}
	}

	// Partition
	if ( bestBucket >= 0 )
	{
		int axis = split.axis;
		float factor = B3_BIN_COUNT * ( 1.0f - FLT_EPSILON ) /
					   ( b3GetByIndex( bounds.upperBound, axis ) - b3GetByIndex( bounds.lowerBound, axis ) );

		int splitIndex = 0;
		for ( int i = 0; i < count; ++i )
		{
			b3Vec3 center = primitives[i].center;
			int index = (int)( factor * ( b3GetByIndex( center, axis ) - b3GetByIndex( bounds.lowerBound, axis ) ) );

			if ( index <= bestBucket )
			{
				b3Primitive temp = primitives[i];
				primitives[i] = primitives[splitIndex];
				primitives[splitIndex] = temp;
				splitIndex++;
			}
		}
		B3_ASSERT( splitIndex == split.index );
	}

	return split;
}

static b3Split b3SplitHalf( int count, b3Primitive* primitives )
{
	// Split in the middle
	int splitIndex = count / 2;

	b3AABB leftBounds = B3_BOUNDS3_EMPTY;
	for ( int i = 0; i < splitIndex; ++i )
	{
		leftBounds = b3AABB_Union( leftBounds, primitives[i].aabb );
	}

	b3AABB rightBounds = B3_BOUNDS3_EMPTY;
	for ( int i = splitIndex; i < count; ++i )
	{

		rightBounds = b3AABB_Union( rightBounds, primitives[i].aabb );
	}

	b3AABB bounds = b3AABB_Union( leftBounds, rightBounds );
	int axis = b3MajorAxis( b3AABB_Extents( bounds ) );

	b3Split split;
	split.axis = axis;
	split.index = splitIndex;
	split.leftBounds = leftBounds;
	split.rightBounds = rightBounds;

	return split;
}

static b3Split b3SplitMedian( int count, b3Primitive* primitives )
{
	B3_ASSERT( count > 2 );

	b3Vec3 lowerBound = primitives[0].center;
	b3Vec3 upperBound = primitives[0].center;

	for ( int i = 1; i < count; ++i )
	{
		lowerBound = b3Min( lowerBound, primitives[i].center );
		upperBound = b3Max( upperBound, primitives[i].center );
	}

	b3Vec3 d = b3Sub( upperBound, lowerBound );
	b3Vec3 c = b3MulSV( 0.5f, b3Add( lowerBound, upperBound ) );

	b3Split split = { 0 };
	split.index = -1;

	// Partition longest axis using the Hoare partition scheme
	// https://en.wikipedia.org/wiki/Quicksort
	// https://nicholasvadivelu.com/2021/01/11/array-partition/
	int i1 = 0, i2 = count;
	if ( d.x >= d.y && d.x >= d.z )
	{
		split.axis = 0;

		float pivot = c.x;

		while ( i1 < i2 )
		{
			while ( i1 < i2 && primitives[i1].center.x < pivot )
			{
				i1 += 1;
			};

			while ( i1 < i2 && primitives[i2 - 1].center.x >= pivot )
			{
				i2 -= 1;
			};

			if ( i1 < i2 )
			{
				// Swap primitives
				b3Primitive temp = primitives[i1];
				primitives[i1] = primitives[i2 - 1];
				primitives[i2 - 1] = temp;

				i1 += 1;
				i2 -= 1;
			}
		}
	}
	else if ( d.y >= d.z )
	{
		split.axis = 1;

		float pivot = c.y;

		while ( i1 < i2 )
		{
			while ( i1 < i2 && primitives[i1].center.y < pivot )
			{
				i1 += 1;
			};

			while ( i1 < i2 && primitives[i2 - 1].center.y >= pivot )
			{
				i2 -= 1;
			};

			if ( i1 < i2 )
			{
				// Swap primitives
				b3Primitive temp = primitives[i1];
				primitives[i1] = primitives[i2 - 1];
				primitives[i2 - 1] = temp;

				i1 += 1;
				i2 -= 1;
			}
		}
	}
	else
	{
		split.axis = 2;

		float pivot = c.z;

		while ( i1 < i2 )
		{
			while ( i1 < i2 && primitives[i1].center.z < pivot )
			{
				i1 += 1;
			};

			while ( i1 < i2 && primitives[i2 - 1].center.z >= pivot )
			{
				i2 -= 1;
			};

			if ( i1 < i2 )
			{
				// Swap primitives
				b3Primitive temp = primitives[i1];
				primitives[i1] = primitives[i2 - 1];
				primitives[i2 - 1] = temp;

				i1 += 1;
				i2 -= 1;
			}
		}
	}
	B3_ASSERT( i1 == i2 );
	B3_ASSERT( 0 <= i1 && i1 < count );

	if ( i1 == 0 || i1 == count - 1 )
	{
		// failed to split
		i1 = count / 2;
	}

	b3AABB leftBounds = B3_BOUNDS3_EMPTY;
	for ( int i = 0; i < i1; ++i )
	{
		leftBounds = b3AABB_Union( leftBounds, primitives[i].aabb );
	}

	b3AABB rightBounds = B3_BOUNDS3_EMPTY;
	for ( int i = i1; i < count; ++i )
	{
		rightBounds = b3AABB_Union( rightBounds, primitives[i].aabb );
	}

	split.index = i1;
	split.leftBounds = leftBounds;
	split.rightBounds = rightBounds;
	return split;
}

#if B3_ENABLE_VALIDATION == 1

static bool b3ValidateSplit( int count, b3Primitive* primitives, const b3Split* split )
{
	if ( split->axis < 0 )
	{
		return false;
	}

	for ( int i = 0; i < split->index; ++i )
	{
		if ( !b3AABB_Contains( split->leftBounds, primitives[i].aabb ) )
		{
			return false;
		}
	}

	for ( int i = split->index; i < count; ++i )
	{
		if ( !b3AABB_Contains( split->rightBounds, primitives[i].aabb ) )
		{
			return false;
		}
	}

	return true;
}

#endif

static int b3BuildRecursive( b3Array( b3MeshNode ) * nodes, int count, b3Primitive* primitives, b3Primitive* base,
							 bool useMedianSplit, int* height )
{
	if ( count > B3_DESIRED_TRIANGLES_PER_LEAF )
	{
		// Try to split the input set using the SAH
		b3Split split;
		if ( useMedianSplit )
		{
			split = b3SplitMedian( count, primitives );
		}
		else
		{
			split = b3SplitBinnedSah( count, primitives );
		}

		if ( split.axis < 0 )
		{
			if ( count > B3_MAXIMUM_TRIANGLES_PER_LEAF )
			{
				// Re-split. This is a less optimal split and can create more false positives!
				split = b3SplitHalf( count, primitives );
			}
			else
			{
				b3AABB bounds = B3_BOUNDS3_EMPTY;
				for ( int i = 0; i < count; ++i )
				{
					bounds = b3AABB_Union( bounds, primitives[i].aabb );
				}

				// We have only a few triangles left. Create a leaf.
				int index = b3Array_AddIndex( *nodes );
				b3StoreLeaf( &nodes->data[index], &bounds, count, (int)( primitives - base ) );

				return index;
			}
		}
		B3_VALIDATE( b3ValidateSplit( count, primitives, &split ) );

		// Allocate node and recurse
		int index = b3Array_AddIndex( *nodes );
		int heightLeft = 0, heightRight = 0;
		int leftIndex = b3BuildRecursive( nodes, split.index, primitives, base, useMedianSplit, &heightLeft );
		int rightIndex =
			b3BuildRecursive( nodes, count - split.index, primitives + split.index, base, useMedianSplit, &heightRight );

		*height = b3MaxInt( heightLeft, heightRight ) + 1;

		B3_UNUSED( leftIndex );
		B3_ASSERT( leftIndex - index == 1 && rightIndex - index > 1 );

		b3AABB aabb = b3AABB_Union( split.leftBounds, split.rightBounds );
		b3MeshNode* node = b3Array_Get( *nodes, index );
		node->data.asNode.axis = split.axis;
		node->data.asNode.childOffset = rightIndex - index;
		node->lowerBound = aabb.lowerBound;
		node->upperBound = aabb.upperBound;

		// Zero so mesh->hash is deterministic
		node->triangleOffset = 0;

		return index;
	}

	b3AABB aabb = B3_BOUNDS3_EMPTY;
	for ( int i = 0; i < count; ++i )
	{
		aabb = b3AABB_Union( aabb, primitives[i].aabb );
	}

	int index = b3Array_AddIndex( *nodes );
	b3StoreLeaf( &nodes->data[index], &aabb, count, (int)( primitives - base ) );

	*height = 1;

	return index;
}

static bool b3SortMeshTriangles( b3MeshData* mesh )
{
	b3MeshTriangle* triangles = b3GetMeshTrianglesWrite( mesh );
	uint8_t* materialIndices = b3GetMeshMaterialIndicesWrite( mesh );

	// Sort triangles in depth-first-order
	int offset = 0;
	b3Array( b3MeshTriangle ) tempTriangles;
	b3Array_CreateN( tempTriangles, mesh->triangleCount );

	b3Array( uint8_t ) tempMaterialIndices;
	b3Array_CreateN( tempMaterialIndices, mesh->triangleCount );

	int count = 0;
	b3MeshNode* stack[B3_MESH_STACK_SIZE];
	stack[count++] = b3GetRootWrite( mesh );

	while ( count > 0 )
	{
		b3MeshNode* node = stack[--count];

		if ( b3IsLeaf( node ) == false )
		{
			if ( count >= B3_MESH_STACK_SIZE - 2 )
			{
				return false;
			}

			stack[count++] = b3GetRightChildWrite( node );
			stack[count++] = b3GetLeftChildWrite( node );
		}
		else
		{
			int triangleCount = node->data.asLeaf.triangleCount;
			int triangleOffset = node->triangleOffset;

			for ( int triangle = 0; triangle < triangleCount; ++triangle )
			{
				int index = triangleOffset + triangle;
				b3Array_Push( tempTriangles, triangles[index] );
				b3Array_Push( tempMaterialIndices, materialIndices[index] );
			}

			node->triangleOffset = offset;
			offset += triangleCount;
		}
	}

	B3_ASSERT( offset == tempTriangles.count );
	B3_ASSERT( tempTriangles.count == mesh->triangleCount );
	B3_ASSERT( tempMaterialIndices.count == mesh->triangleCount );

	// Copy sorted triangle array back to tree
	memcpy( triangles, tempTriangles.data, mesh->triangleCount * sizeof( b3MeshTriangle ) );
	memcpy( materialIndices, tempMaterialIndices.data, mesh->triangleCount * sizeof( uint8_t ) );

	b3Array_Destroy( tempTriangles );
	b3Array_Destroy( tempMaterialIndices );

	return true;
}

typedef struct
{
	int vertex1;
	int vertex2;
	int triangle1;
	int triangle2;
	uint16_t triangleCount;

	// The index of an edge within the parent triangle: 0, 1, or 2. 0xFF is unset
	uint8_t triangleEdgeIndex1;
	uint8_t triangleEdgeIndex2;
} b3MeshEdge;

#define NAME b3EdgeMap
#define KEY_TY uint64_t
#define VAL_TY int
#define HASH_FN vt_hash_integer
#define CMPR_FN vt_cmpr_integer
#define MALLOC_FN b3Alloc
#define FREE_FN b3Free
#include "verstable.h"

#if 0
// todo this is for testing other hash tables
struct b3TestHash
{
	size_t operator()( uint64_t key ) const
	{
		key ^= key >> 23;
		key *= 0x2127599bf4325c37ull;
		key ^= key >> 47;
		return (size_t)key;
	}
};
#endif

// Results for MeshCreationBenchmark
// eastl::hash_map : 4.9703 ms and 5.1445ms with FastHash function
// std::unordered_map : 4.8755 ms and 4.4780ms with FastHash function
// verstable : 3.3968 ms with default FastHash
// no edge identification : 1.7396 ms
static void b3IdentifyEdges( b3MeshData* mesh )
{
	b3MeshTriangle* triangles = b3GetMeshTrianglesWrite( mesh );
	const b3Vec3* vertices = b3GetMeshVertices( mesh );
	uint8_t* flags = b3GetMeshFlagsWrite( mesh );

	int triangleCount = mesh->triangleCount;
	int edgeCount = 3 * triangleCount;
	b3MeshEdge* edges = B3_ALLOC( b3MeshEdge, edgeCount );
	b3Vec3* normals = B3_ALLOC( b3Vec3, triangleCount );

	for ( int i = 0; i < triangleCount; ++i )
	{
		b3MeshTriangle* triangle = triangles + i;
		int i1 = triangle->index1;
		int i2 = triangle->index2;
		int i3 = triangle->index3;

		edges[3 * i + 0].vertex1 = b3MinInt( i1, i2 );
		edges[3 * i + 0].vertex2 = b3MaxInt( i1, i2 );
		edges[3 * i + 0].triangle1 = i;
		edges[3 * i + 0].triangle2 = B3_NULL_INDEX;
		edges[3 * i + 0].triangleEdgeIndex1 = 0;
		edges[3 * i + 0].triangleEdgeIndex2 = 0xFF;
		edges[3 * i + 0].triangleCount = 1;

		edges[3 * i + 1].vertex1 = b3MinInt( i2, i3 );
		edges[3 * i + 1].vertex2 = b3MaxInt( i2, i3 );
		edges[3 * i + 1].triangle1 = i;
		edges[3 * i + 1].triangle2 = B3_NULL_INDEX;
		edges[3 * i + 1].triangleEdgeIndex1 = 1;
		edges[3 * i + 1].triangleEdgeIndex2 = 0xFF;
		edges[3 * i + 1].triangleCount = 1;

		edges[3 * i + 2].vertex1 = b3MinInt( i3, i1 );
		edges[3 * i + 2].vertex2 = b3MaxInt( i3, i1 );
		edges[3 * i + 2].triangle1 = i;
		edges[3 * i + 2].triangle2 = B3_NULL_INDEX;
		edges[3 * i + 2].triangleEdgeIndex1 = 2;
		edges[3 * i + 2].triangleEdgeIndex2 = 0xFF;
		edges[3 * i + 2].triangleCount = 1;

		b3Vec3 v1 = vertices[i1];
		b3Vec3 v2 = vertices[i2];
		b3Vec3 v3 = vertices[i3];

		b3Vec3 e1 = b3Sub( v2, v1 );
		b3Vec3 e2 = b3Sub( v3, v1 );
		b3Vec3 n = b3Cross( e1, e2 );

		normals[i] = b3Normalize( n );
	}

	b3EdgeMap map;
	b3EdgeMap_init( &map );
	b3EdgeMap_reserve( &map, edgeCount );

	uint64_t key = (uint64_t)edges[0].vertex1 << 32 | (uint64_t)edges[0].vertex2;
	b3EdgeMap_insert( &map, key, 0 );

	// Find unique edges and assign adjacency
	for ( int i = 1; i < edgeCount; ++i )
	{
		b3MeshEdge* edge = edges + i;
		key = (uint64_t)edge->vertex1 << 32 | (uint64_t)edge->vertex2;
		b3EdgeMap_itr itr = b3EdgeMap_get( &map, key );

		if ( b3EdgeMap_is_end( itr ) )
		{
			b3EdgeMap_insert( &map, key, i );
		}
		else
		{
			int otherIndex = itr.data->val;
			B3_ASSERT( otherIndex < i );

			b3MeshEdge* base = edges + otherIndex;
			if ( base->triangleCount == 1 )
			{
				base->triangle2 = edge->triangle1;
				base->triangleEdgeIndex2 = edge->triangleEdgeIndex1;
			}

			base->triangleCount += 1;
		}
	}

	b3EdgeMap_cleanup( &map );

	for ( int i = 0; i < edgeCount; ++i )
	{
		b3MeshEdge* edge = edges + i;
		if ( edge->triangleCount != 2 )
		{
			continue;
		}

		B3_ASSERT( edge->triangleEdgeIndex1 < 3 );
		B3_ASSERT( edge->triangleEdgeIndex2 < 3 );

		b3MeshTriangle* triangle1 = triangles + edge->triangle1;
		b3MeshTriangle* triangle2 = triangles + edge->triangle2;
		uint8_t* flag1 = flags + edge->triangle1;
		uint8_t* flag2 = flags + edge->triangle2;

		int j1 = triangle2->index1;
		int j2 = triangle2->index2;
		int j3 = triangle2->index3;

		int opposite = B3_NULL_INDEX;

		switch ( edge->triangleEdgeIndex2 )
		{
			case 0:
				opposite = j3;
				break;

			case 1:
				opposite = j1;
				break;

			case 2:
				opposite = j2;
				break;

			default:
				B3_ASSERT( false );
		}

		int i1 = triangle1->index1;
		int i2 = triangle1->index2;
		int i3 = triangle1->index3;

		b3Vec3 v1 = vertices[i1];
		b3Vec3 v2 = vertices[i2];
		b3Vec3 v3 = vertices[i3];
		b3Vec3 p = vertices[opposite];

		float cos5Deg = 0.9962f;
		float signedVolume = b3SignedVolume( v1, v2, v3, p );
		b3Vec3 n1 = normals[edge->triangle1];
		b3Vec3 n2 = normals[edge->triangle2];
		float cosAngle = b3Dot( n1, n2 );
		if ( signedVolume > 0.0f || cosAngle > cos5Deg )
		{
			int edgeFlags[3] = { b3_concaveEdge1, b3_concaveEdge2, b3_concaveEdge3 };
			*flag1 |= edgeFlags[edge->triangleEdgeIndex1];
			*flag2 |= edgeFlags[edge->triangleEdgeIndex2];
		}

		if ( signedVolume < 0.0f || cosAngle > cos5Deg )
		{
			int edgeFlags[3] = { b3_inverseConcaveEdge1, b3_inverseConcaveEdge2, b3_inverseConcaveEdge3 };
			*flag1 |= edgeFlags[edge->triangleEdgeIndex1];
			*flag2 |= edgeFlags[edge->triangleEdgeIndex2];
		}
	}

	B3_FREE( normals, b3Vec3, triangleCount );
	B3_FREE( edges, b3MeshEdge, edgeCount );
}






















































































































































































































































































































































// todo this should fail if the mesh has a height greater than B3_MESH_STACK_SIZE





























































































































































































































































































































































































































































































































































































































b3Triangle b3GetMeshTriangle( const b3Mesh* mesh, int triangleIndex )
{
	B3_ASSERT( 0 <= triangleIndex && triangleIndex < mesh->data->triangleCount );

	const b3MeshTriangle* triangles = b3GetMeshTriangles( mesh->data );
	const uint8_t* flags = b3GetMeshFlags( mesh->data );
	const b3Vec3* vertices = b3GetMeshVertices( mesh->data );

	b3Triangle result;
	b3MeshTriangle triangle = triangles[triangleIndex];
	uint8_t triangleFlags = flags[triangleIndex];

	b3Vec3 scale = mesh->scale;

	result.vertices[0] = b3Mul( scale, vertices[triangle.index1] );
	result.i1 = triangle.index1;

	if ( scale.x * scale.y * scale.z < 0.0f )
	{
		result.vertices[1] = b3Mul( scale, vertices[triangle.index3] );
		result.vertices[2] = b3Mul( scale, vertices[triangle.index2] );

		result.i2 = triangle.index3;
		result.i3 = triangle.index2;

		// mesh is inverted, so concave edges are now convex
		result.flags = 0;
		result.flags |= ( triangleFlags & b3_inverseConcaveEdge1 ) ? b3_concaveEdge1 : 0;
		result.flags |= ( triangleFlags & b3_inverseConcaveEdge2 ) ? b3_concaveEdge2 : 0;
		result.flags |= ( triangleFlags & b3_inverseConcaveEdge3 ) ? b3_concaveEdge3 : 0;
	}
	else
	{
		result.vertices[1] = b3Mul( scale, vertices[triangle.index2] );
		result.vertices[2] = b3Mul( scale, vertices[triangle.index3] );

		result.i2 = triangle.index2;
		result.i3 = triangle.index3;
		result.flags = triangleFlags;
	}

	return result;
}

int b3CollideMoverAndMesh( b3PlaneResult* planes, int capacity, const b3Mesh* shape, const b3Capsule* mover )
{
	if ( capacity == 0 )
	{
		return 0;
	}

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

	// Scale may have reflection so min/max may become invalid when unscaled
	b3Vec3 meshScale = shape->scale;
	b3V32 scale = b3LoadV( &meshScale.x );
	b3V32 invScale = b3DivV( b3_oneV, scale );
	b3V32 temp1 = b3MulV( invScale, boundsMin );
	b3V32 temp2 = b3MulV( invScale, boundsMax );
	b3V32 invScaledBoundsMin = b3MinV( temp1, temp2 );
	b3V32 invScaledBoundsMax = b3MaxV( temp1, temp2 );
	b3V32 invScaledBoundsCenter = b3MulV( b3_halfV, b3AddV( invScaledBoundsMin, invScaledBoundsMax ) );
	b3V32 invScaledBoundsExtent = b3SubV( invScaledBoundsMax, invScaledBoundsCenter );

	int count = 0;
	const b3MeshNode* stack[B3_MESH_STACK_SIZE];
	const b3MeshNode* node = b3GetRoot( shape->data );
	const b3MeshTriangle* triangles = b3GetMeshTriangles( shape->data );
	const b3Vec3* vertices = b3GetMeshVertices( shape->data );

	int planeCount = 0;
	while ( planeCount < capacity )
	{
		// Test node overlap in unscaled space
		b3V32 nodeMin = b3LoadV( &node->lowerBound.x );
		b3V32 nodeMax = b3LoadV( &node->upperBound.x );
		if ( b3TestBoundsOverlap( nodeMin, nodeMax, invScaledBoundsMin, invScaledBoundsMax ) )
		{
			if ( b3IsLeaf( node ) )
			{
				int triangleCount = node->data.asLeaf.triangleCount;
				int triangleOffset = node->triangleOffset;

				for ( int index = 0; index < triangleCount; ++index )
				{
					int triangleIndex = triangleOffset + index;
					b3MeshTriangle triangle = triangles[triangleIndex];

					b3Vec3 vertex1 = vertices[triangle.index1];
					b3Vec3 vertex2 = vertices[triangle.index2];
					b3Vec3 vertex3 = vertices[triangle.index3];
					b3V32 v1 = b3LoadV( &vertex1.x );
					b3V32 v2 = b3LoadV( &vertex2.x );
					b3V32 v3 = b3LoadV( &vertex3.x );

					// Test triangle bounds overlap in unscaled space
					if ( b3TestBoundsTriangleOverlap( invScaledBoundsCenter, invScaledBoundsExtent, v1, v2, v3 ) )
					{
						// Compute shape distance in scaled space. Winding order doesn't matter.
						// todo implement one-sided collision?
						b3Vec3 triangleVertices[] = { b3Mul( meshScale, vertex1 ), b3Mul( meshScale, vertex2 ),
													  b3Mul( meshScale, vertex3 ) };
						distanceInput.proxyA = (b3ShapeProxy){ triangleVertices, 3, 0.0f };

						// reset the cache
						cache.count = 0;

						// get distance between triangle and query shape
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
			else
			{
				// Recurse
				B3_ASSERT( count <= B3_MESH_STACK_SIZE - 1 );
				stack[count++] = b3GetRightChild( node );
				node = b3GetLeftChild( node );

				continue;
			}
		}

		if ( count == 0 )
		{
			break;
		}
		node = stack[--count];
	}

	return planeCount;
}




























































































