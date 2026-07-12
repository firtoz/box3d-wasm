#include "dump-interaction-overrides.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include "sample_bodies.cpp"
#include "sample_benchmark.cpp"
#include "sample_collision.cpp"
#include "sample_continuous.cpp"
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
	patch_sample_entry( "Ray Curtain", DumpRayCurtain::Create );
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
	(void)sample;
	return nullptr;
}
