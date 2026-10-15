#ifndef GPU_SAMPLES_DRAW_COLOR_H
#define GPU_SAMPLES_DRAW_COLOR_H

#include "box3d/box3d.h"

/* Existing body flags from src/types.rs, observed through the current pose
 * mirror. These are render inputs; selecting a color never changes the flags. */
enum
{
	gpu_drawStatic = 1u,
	gpu_drawSleep = 4u,
	gpu_drawKinematic = 16u,
	gpu_drawBullet = 32u,
	gpu_drawDisabled = 128u,
	gpu_drawFast = 32768u,
	gpu_drawSpeedCapped = 131072u,
	gpu_drawHadTOI = 262144u,
};

static inline uint32_t gpu_samples_draw_color(uint32_t customColor, b3BodyType type, float mass,
											uint32_t flags, bool sensor, bool coolPalette)
{
	/* A custom color may include a packed material preset. */
	if (customColor != 0)
	{
		return customColor;
	}
	const bool awake = (flags & (gpu_drawStatic | gpu_drawSleep | gpu_drawDisabled)) == 0;
	b3HexColor rgb;
	b3DebugMaterial material = b3_debugMaterialDefault;
	bool ordinaryState = false;
	if (type == b3_dynamicBody && mass == 0.0f)
	{
		rgb = b3_colorRed;
	}
	else if (flags & gpu_drawDisabled)
	{
		rgb = b3_colorSlateGray;
	}
	else if (sensor)
	{
		rgb = b3_colorWheat;
	}
	else if (flags & gpu_drawHadTOI)
	{
		rgb = b3_colorLime;
	}
	else if ((flags & gpu_drawBullet) && awake)
	{
		rgb = b3_colorTurquoise;
	}
	else if (flags & gpu_drawSpeedCapped)
	{
		rgb = b3_colorYellow;
	}
	else if (flags & gpu_drawFast)
	{
		rgb = b3_colorOrange;
		material = b3_debugMaterialGlossy;
	}
	else if (type == b3_staticBody)
	{
		rgb = b3_colorDarkGray;
		material = b3_debugMaterialMatte;
		ordinaryState = true;
	}
	else if (type == b3_kinematicBody)
	{
		rgb = awake ? b3_colorSteelBlue : b3_colorLightSteelBlue;
		material = awake ? b3_debugMaterialMetallic : b3_debugMaterialMatte;
		ordinaryState = true;
	}
	else
	{
		rgb = awake ? b3_colorTan : b3_colorLightSlateGray;
		material = awake ? b3_debugMaterialSoft : b3_debugMaterialDead;
		ordinaryState = true;
	}
	if (coolPalette && ordinaryState)
	{
		/* Retain the combined GPU palette, while preserving state material.
		 * Explicit custom colors and diagnostic hues have precedence. */
		rgb = type == b3_staticBody ? b3_colorDarkSlateBlue : b3_colorCornflowerBlue;
	}
	/* HadTOI and speed-cap inputs are recorded at the actual operation.
	 * CCD_NO_HIT remains independent proxy state. */
	return b3MakeDebugColor(rgb, material);
}

#endif
