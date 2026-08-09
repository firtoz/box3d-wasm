#include "dump-interaction-overrides.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include "stability.h"
#include "utils.h"

#include "sample_bodies.cpp"
#include "sample_benchmark.cpp"
#include "sample_collision.cpp"
#include "sample_continuous.cpp"
#include "sample_events.cpp"
#include "sample_issues.cpp"
#include "sample_joint.cpp"

class DumpMotorJoint : public MotorJoint
{
public:
	using MotorJoint::MotorJoint;

	static Sample* Create( SampleContext* context )
	{
		return new DumpMotorJoint( context );
	}

	bool ApplyDumpInteraction( const DumpInteraction& interaction )
	{
		if ( strcmp( interaction.action, "set-speed" ) == 0 )
		{
			m_speed = interaction.args[0];
			return true;
		}

		return false;
	}
};

class DumpTopDownFriction : public TopDownFriction
{
public:
	using TopDownFriction::TopDownFriction;

	static Sample* Create( SampleContext* context )
	{
		return new DumpTopDownFriction( context );
	}

	bool ApplyDumpInteraction( const DumpInteraction& interaction )
	{
		if ( strcmp( interaction.action, "explode" ) == 0 )
		{
			b3ExplosionDef def = b3DefaultExplosionDef();
			def.position = { interaction.args[0], interaction.args[1], interaction.args[2] };
			def.radius = interaction.args[3];
			def.falloff = interaction.args[4];
			def.impulsePerArea = interaction.args[5];
			b3World_Explode( m_worldId, &def );
			return true;
		}

		return false;
	}
};

class DumpDoor : public Door
{
public:
	using Door::Door;

	static Sample* Create( SampleContext* context )
	{
		return new DumpDoor( context );
	}

	bool ApplyDumpInteraction( const DumpInteraction& interaction )
	{
		if ( strcmp( interaction.action, "impulse" ) == 0 )
		{
			m_magnitude = interaction.args[0];
			b3Pos p = b3Body_GetWorldPoint( m_doorId, { 0.75f, 0.0f, 0.0f } );
			b3Body_ApplyLinearImpulse( m_doorId, { 0.0f, 0.0f, -m_magnitude }, p, true );
			m_translationError1 = 0.0f;
			m_translationError2 = 0.0f;
			return true;
		}

		return false;
	}
};

class DumpWeeble : public Weeble
{
public:
	using Weeble::Weeble;

	static Sample* Create( SampleContext* context )
	{
		return new DumpWeeble( context );
	}

	bool ApplyDumpInteraction( const DumpInteraction& interaction )
	{
		if ( strcmp( interaction.action, "teleport" ) == 0 )
		{
			b3Body_SetTransform( m_weebleId, { 0.0f, 5.0f, 0.0f }, b3MakeQuatFromAxisAngle( b3Vec3_axisZ, 0.95f * B3_PI ) );
			b3Body_SetAwake( m_weebleId, true );
			return true;
		}

		return false;
	}
};

class DumpBulletVersusStack : public BulletVersusStack
{
public:
	using BulletVersusStack::BulletVersusStack;

	static Sample* Create( SampleContext* context )
	{
		return new DumpBulletVersusStack( context );
	}

	bool ApplyDumpInteraction( const DumpInteraction& interaction )
	{
		if ( strcmp( interaction.action, "launch" ) == 0 )
		{
			Launch();
			return true;
		}

		return false;
	}
};

// Skip mid-run destroy/respawn so dump body order stays aligned with WASM handle order.
// Live demo still respawns every 140 steps; explode-at-create is enough for dump parity.
class DumpDestruction : public BenchmarkDestruction
{
public:
	using BenchmarkDestruction::BenchmarkDestruction;

	static Sample* Create( SampleContext* context )
	{
		return new DumpDestruction( context );
	}

	void Step() override
	{
		Sample::Step();
	}
};

class DumpCrash : public Crash
{
public:
	using Crash::Crash;

	static Sample* Create( SampleContext* context )
	{
		return new DumpCrash( context );
	}

	bool ApplyDumpInteraction( const DumpInteraction& interaction )
	{
		if ( strcmp( interaction.action, "add-joint" ) == 0 )
		{
			b3WeldJointDef jointDef = b3DefaultWeldJointDef();
			jointDef.base.bodyIdA = m_bodyId1;
			jointDef.base.bodyIdB = m_bodyId2;
			b3CreateWeldJoint( m_worldId, &jointDef );
			return true;
		}

		return false;
	}
};

// Unique dump alias for Determinism CreateMeshDrop (upstream RegisterSample name "Mesh Drop"
// collides with Continuous / Mesh Drop).
class DumpCreateMeshDrop : public Sample
{
public:
	explicit DumpCreateMeshDrop( SampleContext* context )
		: Sample( context )
	{
		m_data = CreateMeshDrop( m_worldId, b3Pos_zero );
	}

	~DumpCreateMeshDrop() override
	{
		DestroyMeshDrop( &m_data );
	}

	static Sample* Create( SampleContext* context )
	{
		return new DumpCreateMeshDrop( context );
	}

	MeshDropData m_data;
};

// Continuous Mesh Drop with the commented fixed seed from upstream Generate().
// Standalone (not subclassing MeshDrop) so body IDs are allocated once in creation order.
class DumpContinuousMeshDrop : public Sample
{
public:
	static constexpr int m_gridCount = 32;
	static constexpr int m_bodyCount = m_gridCount * m_gridCount;

	explicit DumpContinuousMeshDrop( SampleContext* context )
		: Sample( context )
	{
		m_groundMesh = nullptr;
		m_groundId = b3_nullBodyId;
		CreateGround();

		m_bodies = new b3BodyId[m_bodyCount];
		memset( m_bodies, 0, m_bodyCount * sizeof( b3BodyId ) );

		b3BoxHull box = b3MakeBoxHull( 0.02f, 0.2f, 0.04f );

		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.rollingResistance = 0.1f;
		// m_collide = true → no category/mask filter on dynamics.

		g_randomSeed = 1910133196;

		int bodyIndex = 0;
		for ( int i = 0; i < m_gridCount; ++i )
		{
			for ( int j = 0; j < m_gridCount; ++j )
			{
				b3Vec3 linearVelocity = RandomVec3Uniform( -1.0f, 1.0f );
				b3Vec3 angularVelocity = RandomVec3Uniform( -5.0f, 5.0f );

				bodyDef.position = { 0.5f * ( i - 0.5f * m_gridCount ), 5.0f, 0.5f * ( j - 0.5f * m_gridCount ) };
				bodyDef.linearVelocity = linearVelocity;
				bodyDef.angularVelocity = angularVelocity;
				b3BodyId bodyId = b3CreateBody( m_worldId, &bodyDef );
				b3CreateHullShape( bodyId, &shapeDef, &box.base );
				m_bodies[bodyIndex] = bodyId;
				bodyIndex += 1;
			}
		}
	}

	~DumpContinuousMeshDrop() override
	{
		delete[] m_bodies;
		if ( m_groundMesh != nullptr )
		{
			b3DestroyMesh( m_groundMesh );
			m_groundMesh = nullptr;
		}
	}

	void CreateGround()
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		m_groundId = b3CreateBody( m_worldId, &bodyDef );

		int gridCount = 40;
		float cellWidth = 1.0f;
		float rowHz = 0.1f;
		float columnHz = 0.2f;
		float groundAmplitude = 0.5f;

		m_groundMesh = b3CreateWaveMesh( gridCount, gridCount, cellWidth, groundAmplitude, rowHz, columnHz );
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.filter.categoryBits = 1;
		b3CreateMeshShape( m_groundId, &shapeDef, m_groundMesh, b3Vec3_one );

		float extent = 0.5f * gridCount * cellWidth;
		float halfHeight = 1.0f;

		{
			b3Transform transform;
			transform.p = { 0.0f, halfHeight, -extent };
			transform.q = b3Quat_identity;
			b3BoxHull wallBox = b3MakeTransformedBoxHull( extent, halfHeight, 0.1f, transform );
			b3CreateHullShape( m_groundId, &shapeDef, &wallBox.base );
		}

		{
			b3Transform transform;
			transform.p = { 0.0f, halfHeight, extent };
			transform.q = b3Quat_identity;
			b3BoxHull wallBox = b3MakeTransformedBoxHull( extent, halfHeight, 0.1f, transform );
			b3CreateHullShape( m_groundId, &shapeDef, &wallBox.base );
		}

		{
			b3Transform transform;
			transform.p = { -extent, halfHeight, 0.0f };
			transform.q = b3Quat_identity;
			b3BoxHull wallBox = b3MakeTransformedBoxHull( 0.1f, halfHeight, extent, transform );
			b3CreateHullShape( m_groundId, &shapeDef, &wallBox.base );
		}

		{
			b3Transform transform;
			transform.p = { extent, halfHeight, 0.0f };
			transform.q = b3Quat_identity;
			b3BoxHull wallBox = b3MakeTransformedBoxHull( 0.1f, halfHeight, extent, transform );
			b3CreateHullShape( m_groundId, &shapeDef, &wallBox.base );
		}
	}

	static Sample* Create( SampleContext* context )
	{
		return new DumpContinuousMeshDrop( context );
	}

	b3MeshData* m_groundMesh;
	b3BodyId m_groundId;
	b3BodyId* m_bodies;
};

// Release CreateLargeWorld is 1000×1000 statics — not dumpable. Mirror BENCHMARK_DEBUG scale.
class DumpLargeWorld : public Sample
{
public:
	explicit DumpLargeWorld( SampleContext* context )
		: Sample( context )
	{
		b3Capacity capacity = {};
		capacity.staticShapeCount = 1024;
		capacity.staticBodyCount = 1024;
		capacity.dynamicShapeCount = 16;
		capacity.dynamicBodyCount = 16;
		capacity.contactCount = 1024 < 8 * 16 ? 8 * 16 : 1024;
		CreateWorld( &capacity );

		const float cell = 10.0f;
		const int gridCount = 32;
		const float halfSpan = 0.5f * cell * gridCount;

		b3BoxHull box = b3MakeBoxHull( 0.5f * cell, 0.25f, 0.5f * cell );
		b3BodyDef bodyDef = b3DefaultBodyDef();
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.invokeContactCreation = true;

		for ( int i = 0; i < gridCount; ++i )
		{
			float x = -halfSpan + ( i + 0.5f ) * cell;
			for ( int j = 0; j < gridCount; ++j )
			{
				float z = -halfSpan + ( j + 0.5f ) * cell;
				bodyDef.position = (b3Pos){ x, 0.0f, z };
				b3BodyId body = b3CreateBody( m_worldId, &bodyDef );
				b3CreateHullShape( body, &shapeDef, &box.base );
			}
		}
	}

	void Step() override
	{
		StepDebugLargeWorld();
		Sample::Step();
	}

	static Sample* Create( SampleContext* context )
	{
		return new DumpLargeWorld( context );
	}

private:
	void StepDebugLargeWorld()
	{
		const int sphereCount = 16;
		const int dropInterval = 8;
		const float cell = 10.0f;
		const int gridCount = 32;

		if ( m_spheresDropped >= sphereCount )
		{
			return;
		}

		if ( m_stepCount == 0 )
		{
			return;
		}

		if ( ( m_stepCount % dropInterval ) != 0 )
		{
			return;
		}

		int side = 1;
		while ( side * side < sphereCount )
		{
			side += 1;
		}

		int idx = m_spheresDropped;
		int gi = idx % side;
		int gj = idx / side;

		float halfSpan = 0.5f * cell * gridCount;
		float inset = 0.1f * 2.0f * halfSpan;
		float usable = 2.0f * halfSpan - 2.0f * inset;
		float x = -halfSpan + inset + ( gi + 0.5f ) * ( usable / side );
		float z = -halfSpan + inset + ( gj + 0.5f ) * ( usable / side );

		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = (b3Pos){ x, 1.5f, z };

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3Sphere sphere = { { 0.0f, 0.0f, 0.0f }, 0.5f };

		b3BodyId body = b3CreateBody( m_worldId, &bodyDef );
		b3CreateSphereShape( body, &shapeDef, &sphere );
		(void)body;

		m_spheresDropped += 1;
	}

	int m_spheresDropped = 0;
};

class DumpRayCurtain : public RayCurtain
{
public:
	struct Hit
	{
		int hit;
		float fraction;
		b3Vec3 point;
		b3Vec3 normal;
	};

	explicit DumpRayCurtain( SampleContext* context )
		: RayCurtain( context )
	{
		CaptureRays();
	}

	static Sample* Create( SampleContext* context )
	{
		return new DumpRayCurtain( context );
	}

	void Step() override
	{
		Sample::Step();
		// Upstream casts in Render after Step, then advances offset.
		CaptureRays();
		AdvanceOffset();
	}

	const char* CheckpointExtrasJson()
	{
		m_extrasJson.clear();
		char buf[256];
		snprintf( buf, sizeof( buf ), "\"rays\":{\"o\":%.17g,\"r\":[", (double)m_capturedOffset );
		m_extrasJson += buf;
		for ( size_t i = 0; i < m_hits.size(); ++i )
		{
			const Hit& h = m_hits[i];
			if ( i > 0 )
				m_extrasJson += ",";
			snprintf( buf, sizeof( buf ),
					  "{\"h\":%d,\"f\":%.17g,\"p\":[%.17g,%.17g,%.17g],\"n\":[%.17g,%.17g,%.17g]}", h.hit, (double)h.fraction,
					  (double)h.point.x, (double)h.point.y, (double)h.point.z, (double)h.normal.x, (double)h.normal.y,
					  (double)h.normal.z );
			m_extrasJson += buf;
		}
		m_extrasJson += "]}";
		return m_extrasJson.c_str();
	}

private:
	void CaptureRays()
	{
		m_hits.clear();
		m_capturedOffset = m_offset;
		constexpr int rayCount = 161; // (-8 .. 8) / 0.1 inclusive
		for ( int i = 0; i < rayCount; ++i )
		{
			float x = -8.0f + 0.1f * (float)i;
			b3Pos rayOrigin = { x, 8.0f, m_offset };
			b3Pos rayEnd = { x, 0.0f, m_offset };
			b3Vec3 rayTranslation = b3SubPos( rayEnd, rayOrigin );
			b3RayResult result = b3World_CastRayClosest( m_worldId, rayOrigin, rayTranslation, b3DefaultQueryFilter() );
			Hit hit = {};
			if ( result.hit )
			{
				hit.hit = 1;
				hit.fraction = result.fraction;
				hit.point = result.point;
				hit.normal = result.normal;
			}
			else
			{
				hit.hit = 0;
				hit.fraction = 1.0f;
				hit.point = rayEnd;
				hit.normal = { 0.0f, 1.0f, 0.0f };
			}
			m_hits.push_back( hit );
		}
	}

	void AdvanceOffset()
	{
		if ( m_offset > 2.0f )
			m_speed = -m_absSpeed;
		else if ( m_offset < -2.0f )
			m_speed = m_absSpeed;
		m_offset += m_speed;
	}

	std::vector<Hit> m_hits;
	float m_capturedOffset = 2.0f;
	std::string m_extrasJson;
};

class DumpCapsuleCastRay : public CapsuleCastRay
{
public:
	explicit DumpCapsuleCastRay( SampleContext* context )
		: CapsuleCastRay( context )
	{
		CaptureRay();
	}

	static Sample* Create( SampleContext* context )
	{
		return new DumpCapsuleCastRay( context );
	}

	void Step() override
	{
		Sample::Step();
		CaptureRay();
	}

	const char* CheckpointExtrasJson()
	{
		m_extrasJson.clear();
		char buf[256];
		snprintf( buf, sizeof( buf ), "\"rays\":{\"o\":0,\"r\":[" );
		m_extrasJson += buf;
		snprintf( buf, sizeof( buf ),
				  "{\"h\":%d,\"f\":%.17g,\"p\":[%.17g,%.17g,%.17g],\"n\":[%.17g,%.17g,%.17g]}",
				  m_hit, (double)m_fraction, (double)m_point.x, (double)m_point.y, (double)m_point.z, (double)m_normal.x,
				  (double)m_normal.y, (double)m_normal.z );
		m_extrasJson += buf;
		m_extrasJson += "]}";
		return m_extrasJson.c_str();
	}

private:
	void CaptureRay()
	{
		b3Pos origin = { -1.0f, 0.5f, 0.0f };
		b3Vec3 translation = { 2.0f, 0.0f, 0.0f };
		b3BodyCastResult result =
			b3Body_CastRay( m_bodyId, origin, translation, b3DefaultQueryFilter(), 1.0f, b3WorldTransform_identity );
		if ( result.hit )
		{
			m_hit = 1;
			m_fraction = result.fraction;
			m_point = result.point;
			m_normal = result.normal;
		}
		else
		{
			m_hit = 0;
			m_fraction = 1.0f;
			m_point = b3OffsetPos( origin, translation );
			m_normal = { 0.0f, 1.0f, 0.0f };
		}
	}

	int m_hit = 0;
	float m_fraction = 1.0f;
	b3Pos m_point = { 1.0f, 0.5f, 0.0f };
	b3Vec3 m_normal = { 0.0f, 1.0f, 0.0f };
	std::string m_extrasJson;
};

// Upstream SensorHits::Launch uses RandomFloatRange(200, 300). Dump uses fixed speed 250.
class DumpSensorHits : public SensorHits
{
public:
	explicit DumpSensorHits( SampleContext* context )
		: SensorHits( context )
	{
		b3Body_SetLinearVelocity( m_bodyId, { 250.0f, 0.0f } );
	}

	static Sample* Create( SampleContext* context )
	{
		return new DumpSensorHits( context );
	}
};

static void patch_sample_entry( const char* name, SampleCreateFcn* createFcn )
{
	for ( int i = 0; i < g_sampleCount; ++i )
	{
		if ( strcmp( g_sampleEntries[i].Name, name ) == 0 )
		{
			g_sampleEntries[i].CreateFcn = createFcn;
			return;
		}
	}
}

void patch_dump_sample_entries()
{
	patch_sample_entry( "Motor Joint", DumpMotorJoint::Create );
	patch_sample_entry( "Top Down Friction", DumpTopDownFriction::Create );
	patch_sample_entry( "Door", DumpDoor::Create );
	patch_sample_entry( "Weeble", DumpWeeble::Create );
	patch_sample_entry( "Bullet vs Stack", DumpBulletVersusStack::Create );
	patch_sample_entry( "Destruction", DumpDestruction::Create );
	patch_sample_entry( "Crash", DumpCrash::Create );
	// Dump-only aliases sharing Crash construction with different interaction schedules.
	RegisterSample( "Issues", "Crash Joint Awake", DumpCrash::Create );
	RegisterSample( "Issues", "Crash Joint Asleep", DumpCrash::Create );
	// Unique aliases: upstream Determinism and Continuous both RegisterSample(..., "Mesh Drop", ...).
	RegisterSample( "Determinism", "Determinism Mesh Drop", DumpCreateMeshDrop::Create );
	RegisterSample( "Continuous", "Continuous Mesh Drop", DumpContinuousMeshDrop::Create );
	patch_sample_entry( "Ray Curtain", DumpRayCurtain::Create );
	patch_sample_entry( "Capsule Cast Ray", DumpCapsuleCastRay::Create );
	patch_sample_entry( "Large World", DumpLargeWorld::Create );
	patch_sample_entry( "Sensor Hits", DumpSensorHits::Create );
}

bool apply_dump_interaction( Sample* sample, const char* sampleName, const DumpInteraction& interaction )
{
	if ( strcmp( sampleName, "Motor Joint" ) == 0 )
	{
		return static_cast<DumpMotorJoint*>( sample )->ApplyDumpInteraction( interaction );
	}

	if ( strcmp( sampleName, "Top Down Friction" ) == 0 )
	{
		return static_cast<DumpTopDownFriction*>( sample )->ApplyDumpInteraction( interaction );
	}

	if ( strcmp( sampleName, "Door" ) == 0 )
	{
		return static_cast<DumpDoor*>( sample )->ApplyDumpInteraction( interaction );
	}

	if ( strcmp( sampleName, "Weeble" ) == 0 )
	{
		return static_cast<DumpWeeble*>( sample )->ApplyDumpInteraction( interaction );
	}

	if ( strcmp( sampleName, "Bullet vs Stack" ) == 0 )
	{
		return static_cast<DumpBulletVersusStack*>( sample )->ApplyDumpInteraction( interaction );
	}

	if ( strcmp( sampleName, "Crash" ) == 0 ||
		 strcmp( sampleName, "Crash Joint Awake" ) == 0 ||
		 strcmp( sampleName, "Crash Joint Asleep" ) == 0 )
	{
		return static_cast<DumpCrash*>( sample )->ApplyDumpInteraction( interaction );
	}

	// Candy Cups / Explosion only need the world handle for explode.
	if ( ( strcmp( sampleName, "Candy Cups" ) == 0 || strcmp( sampleName, "Explosion" ) == 0 ) &&
		 strcmp( interaction.action, "explode" ) == 0 )
	{
		b3ExplosionDef def = b3DefaultExplosionDef();
		def.position = { interaction.args[0], interaction.args[1], interaction.args[2] };
		def.radius = interaction.args[3];
		def.falloff = interaction.args[4];
		def.impulsePerArea = interaction.args[5];
		b3World_Explode( sample->m_worldId, &def );
		return true;
	}

	return false;
}

const char* get_dump_checkpoint_extras( Sample* sample, const char* sampleName )
{
	if ( strcmp( sampleName, "Ray Curtain" ) == 0 )
	{
		return static_cast<DumpRayCurtain*>( sample )->CheckpointExtrasJson();
	}
	if ( strcmp( sampleName, "Capsule Cast Ray" ) == 0 )
	{
		return static_cast<DumpCapsuleCastRay*>( sample )->CheckpointExtrasJson();
	}
	(void)sample;
	return nullptr;
}
