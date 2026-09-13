#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "earcut.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <array>
class Fixture { public: b3WorldId m_worldId; b3MeshData* m_mesh;
void CreateMesh( b3BodyId groundId )
	{
		// Silhouette of the Box2D stairwell traced as a closed loop. It is extruded
		// four meters along z into a single triangle mesh, the 3D analogue of a
		// b2Chain loop. The solid is inside the loop, so the windings face outward
		// to put the collision normals on the open basin side where the debris sits.
		static const b3Vec2 points[32] = {
			{ -11.3000f, -0.2167f }, { 9.3375f, -0.2167f },	 { 9.3375f, 7.1917f },	{ 8.8083f, 7.1917f },  { 8.8083f, 0.3125f },
			{ 0.3417f, 0.3125f },	 { 0.3417f, 0.8417f },	 { -0.1875f, 0.8417f }, { -0.1875f, 1.3708f }, { -0.7167f, 1.3708f },
			{ -0.7167f, 1.9000f },	 { -1.2458f, 1.9000f },	 { -1.2458f, 2.4292f }, { -1.7750f, 2.4292f }, { -1.7750f, 2.9583f },
			{ -2.3042f, 2.9583f },	 { -2.3042f, 3.4875f },	 { -2.8333f, 3.4875f }, { -2.8333f, 4.0167f }, { -3.3625f, 4.0167f },
			{ -3.3625f, 4.5458f },	 { -3.8917f, 4.5458f },	 { -3.8917f, 5.0750f }, { -4.4208f, 5.0750f }, { -4.4208f, 5.6042f },
			{ -4.9500f, 5.6042f },	 { -4.9500f, 6.1333f },	 { -5.4792f, 6.1333f }, { -5.4792f, 6.6625f }, { -6.0083f, 6.6625f },
			{ -6.0083f, 7.1917f },	 { -11.3000f, 7.1917f },
		};

		float zMin = -2.0f; // four meters across z
		float zMax = 2.0f;

		// Two vertices per silhouette point, one at each depth.
		b3Vec3 vertices[64];
		for ( int i = 0; i < 32; ++i )
		{
			vertices[2 * i + 0] = { points[i].x, points[i].y, zMin };
			vertices[2 * i + 1] = { points[i].x, points[i].y, zMax };
		}

		std::vector<int> indices;

		// Side walls: two triangles per silhouette edge, wound so the normal faces
		// the open basin. The lo ring sits at zMin, the hi ring at zMax.
		for ( int i = 0; i < 32; ++i )
		{
			int j = ( i + 1 ) % 32;
			int aLo = 2 * i, aHi = 2 * i + 1;
			int bLo = 2 * j, bHi = 2 * j + 1;

			indices.push_back( aLo );
			indices.push_back( bLo );
			indices.push_back( bHi );

			indices.push_back( aLo );
			indices.push_back( bHi );
			indices.push_back( aHi );
		}

		// End caps close the prism into a watertight manifold with no boundary edges.
		// The non convex silhouette is triangulated with earcut, then each triangle is
		// wound so the cap normal points out of the solid: the zMin cap faces -z and
		// the zMax cap faces +z. The two ring edges each gain a second owner this way.
		std::vector<std::array<double, 2>> ring;
		ring.reserve( 32 );
		for ( int i = 0; i < 32; ++i )
		{
			ring.push_back( { points[i].x, points[i].y } );
		}
		std::vector<std::vector<std::array<double, 2>>> polygon = { ring };
		std::vector<uint32_t> cap = mapbox::earcut<uint32_t>( polygon );

		for ( size_t k = 0; k + 3 <= cap.size(); k += 3 )
		{
			int r0 = (int)cap[k], r1 = (int)cap[k + 1], r2 = (int)cap[k + 2];
			PushCap( indices, points, r0, r1, r2, 1, true );  // zMax cap, +z out
			PushCap( indices, points, r0, r1, r2, 0, false ); // zMin cap, -z out
		}

		b3MeshDef def = {};
		def.vertices = vertices;
		def.vertexCount = 64;
		def.indices = indices.data();
		def.triangleCount = (int)( indices.size() / 3 );
		def.identifyEdges = true;

		m_mesh = b3CreateMesh( &def, nullptr, 0 );

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.customColor = b3_colorDarkSeaGreen;
		b3SurfaceMaterial material = shapeDef.baseMaterial;
		shapeDef.materials = &material;
		shapeDef.materialCount = 1;
		b3CreateMeshShape( groundId, &shapeDef, m_mesh, b3Vec3_one );

		// Back wall: a 0.1 m thick box closing the far side over the full mesh extent.
		b3Vec2 lower = points[0];
		b3Vec2 upper = points[0];
		for ( int i = 1; i < 32; ++i )
		{
			lower.x = b3MinFloat( lower.x, points[i].x );
			lower.y = b3MinFloat( lower.y, points[i].y );
			upper.x = b3MaxFloat( upper.x, points[i].x );
			upper.y = b3MaxFloat( upper.y, points[i].y );
		}

		float wallHalfThick = 0.05f;
		b3Vec3 wallCenter = { 0.5f * ( lower.x + upper.x ), 0.5f * ( lower.y + upper.y ), -zMax - wallHalfThick };
		b3BoxHull wall =
			b3MakeOffsetBoxHull( 0.5f * ( upper.x - lower.x ), 0.5f * ( upper.y - lower.y ), wallHalfThick, wallCenter );
		b3CreateHullShape( groundId, &shapeDef, &wall.base );
	}

	// Push one cap triangle, flipping the winding so its z-normal has the wanted sign.
	void PushCap( std::vector<int>& indices, const b3Vec2* poly, int r0, int r1, int r2, int vOffset, bool wantPositiveZ )
	{
		float cross =
			( poly[r1].x - poly[r0].x ) * ( poly[r2].y - poly[r0].y ) - ( poly[r1].y - poly[r0].y ) * ( poly[r2].x - poly[r0].x );
		bool positive = cross > 0.0f;

		int v0 = 2 * r0 + vOffset;
		int v1 = 2 * r1 + vOffset;
		int v2 = 2 * r2 + vOffset;

		indices.push_back( v0 );
		if ( positive == wantPositiveZ )
		{
			indices.push_back( v1 );
			indices.push_back( v2 );
		}
		else
		{
			indices.push_back( v2 );
			indices.push_back( v1 );
		}
	}

	
};
int main(int argc, char** argv){auto wd=b3DefaultWorldDef();wd.gravity=b3Vec3_zero;wd.enableSleep=false;wd.enableContinuous=false;auto world=b3CreateWorld(&wd);auto gd=b3DefaultBodyDef();auto ground=b3CreateBody(world,&gd);Fixture fixture;fixture.m_worldId=world;fixture.CreateMesh(ground);
b3Vec3 vertices[]={{0.174166054f,-0.227081954f,0.0900000036f},{0.251808524f,0.160288826f,-0.0300000012f},{-0.120846272f,-0.0499619171f,-0.270000011f},{0.130766988f,0.0f,0.270000011f},{0.0226820186f,0.258815616f,0.150000006f},{-0.15796712f,-0.14472869f,0.210000008f},{0.201279074f,-0.0733945295f,-0.210000008f},{-0.119846463f,0.230514303f,-0.150000006f},{-0.0741920024f,-0.276397526f,-0.0900000036f},{-0.293946117f,0.0519203208f,0.0300000012f}};auto h=b3CreateHull(vertices,10,10);auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;bd.position={-2.23551345f,3.65449882f,1.35078895f};bd.rotation={{-0.249519289f,0.466266036f,-0.56968528f},0.629122198f};// Captured Gear Lift body 204, step 800: clipping yields more than four
// vertices, with the penetrating support witness beyond the first four.
if (argc == 2 && std::strcmp(argv[1], "clipped-support") == 0) {
 bd.position={-0.279433757f,1.59668708f,-0.223208442f};
 bd.rotation={{0.197710097f,-0.56771481f,-0.0384295136f},0.798206508f};
}
auto rock=b3CreateBody(world,&bd);auto sd=b3DefaultShapeDef();sd.baseMaterial.rollingResistance=0.3f;b3CreateHullShape(rock,&sd,h);
b3World_Step(world,1e-7f,4);int capacity=b3Body_GetContactCapacity(rock);std::vector<b3ContactData> cs(capacity);int count=b3Body_GetContactData(rock,cs.data(),capacity);printf("contacts %d\n",count);for(int i=0;i<count;i++)for(int j=0;j<cs[i].manifoldCount;j++){auto m=cs[i].manifolds[j];float sign=b3Shape_GetBody(cs[i].shapeIdA).index1==rock.index1?-1.0f:1.0f;printf("normal %.9g %.9g %.9g points %d\n",sign*m.normal.x,sign*m.normal.y,sign*m.normal.z,m.pointCount);for(int k=0;k<m.pointCount;k++)printf("separation %.9g\n",m.points[k].separation);}b3DestroyWorld(world);b3DestroyHull(h);b3DestroyMesh(fixture.m_mesh);}
