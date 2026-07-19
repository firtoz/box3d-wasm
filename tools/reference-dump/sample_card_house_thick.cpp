// Restored from box3d e961bfb after upstream removed "Card House Thick".
// Kept so our extra TS port can still dump-compare against a C++ twin.

#include "sample.h"

#include "box3d/box3d.h"

class CardHouseThick : public Sample
{
public:
	explicit CardHouseThick( SampleContext* context )
		: Sample( context )
	{
		if ( context->restart == false )
		{
			m_camera->SetView( 0.0f, 25.0f, 10.0f, { 0.0f, 2.0f, 0.0f } );
		}

		AddGroundBox( 10.0f );

		const float alpha = 25.0f * B3_DEG_TO_RAD;
		const float width = 0.38f;
		const float height = 0.98f;
		const float depth = 0.08f;

		float offsetX = 0.5f * height * b3Sin( alpha ) + 0.045f;
		float offsetY = 0.5f * height * b3Cos( alpha ) + 0.035f;

		b3BoxHull box = b3MakeBoxHull( 0.5f * depth, 0.5f * height, 0.5f * width );
		AddVerticalRow( 4, -6.0f * offsetX, offsetX, offsetY, alpha, box );
		AddHorizontalRow( 3, -4.0f * offsetX, 4.0f * offsetX, 2.0f * offsetY + 0.04f, box );
		AddVerticalRow( 3, -4.0f * offsetX, offsetX, 3.0f * offsetY + 0.08f, alpha, box );
		AddHorizontalRow( 2, -2.0f * offsetX, 4.0f * offsetX, 4.0f * offsetY + 0.12f, box );
		AddVerticalRow( 2, -2.0f * offsetX, offsetX, 5.0f * offsetY + 0.16f, alpha, box );
		AddHorizontalRow( 1, -0.0f * offsetX, 4.0f * offsetX, 6.0f * offsetY + 0.20f, box );
		AddVerticalRow( 1, -0.0f * offsetX, offsetX, 7.0f * offsetY + 0.24f, alpha, box );
	}

	void AddVerticalRow( int n, float startX, float offsetX, float startY, float alpha, const b3BoxHull& box )
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.friction = 0.8f;

		for ( int index = 0; index < n; ++index )
		{
			bodyDef.position = { startX - offsetX, startY, 0.0f };
			bodyDef.rotation = b3MakeQuatFromAxisAngle( b3Vec3_axisZ, -alpha );
			b3BodyId body1 = b3CreateBody( m_worldId, &bodyDef );
			b3CreateHullShape( body1, &shapeDef, &box.base );

			bodyDef.position = { startX + offsetX, startY, 0.0f };
			bodyDef.rotation = b3MakeQuatFromAxisAngle( b3Vec3_axisZ, alpha );
			b3BodyId body2 = b3CreateBody( m_worldId, &bodyDef );
			b3CreateHullShape( body2, &shapeDef, &box.base );

			startX += 4.0f * offsetX;
		}
	}

	void AddHorizontalRow( int n, float startX, float offsetX, float startY, const b3BoxHull& box )
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.friction = 0.8f;

		for ( int index = 0; index < n; ++index )
		{
			bodyDef.position = { startX + index * offsetX, startY, 0.0f };
			bodyDef.rotation = b3MakeQuatFromAxisAngle( b3Vec3_axisZ, 0.5f * B3_PI );
			b3BodyId body = b3CreateBody( m_worldId, &bodyDef );
			b3CreateHullShape( body, &shapeDef, &box.base );
		}
	}

	static Sample* Create( SampleContext* context )
	{
		return new CardHouseThick( context );
	}
};

static int sampleCardHouseThick = RegisterSample( "Stacking", "Card House Thick", CardHouseThick::Create );
