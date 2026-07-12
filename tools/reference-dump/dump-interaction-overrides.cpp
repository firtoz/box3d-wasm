#include "dump-interaction-overrides.h"

#include <string.h>

#include "sample_bodies.cpp"
#include "sample_continuous.cpp"
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
