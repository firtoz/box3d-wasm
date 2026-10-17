#ifndef GPU_HEIGHT_FIELD_COMPAT_H
#define GPU_HEIGHT_FIELD_COMPAT_H

#include "box3d/box3d.h"
#include <math.h>

typedef struct GpuHeightFieldEncoding
{
	float minHeight;
	float heightScale;
	b3Vec3 scale;
} GpuHeightFieldEncoding;

static inline GpuHeightFieldEncoding gpu_height_field_encoding(const b3HeightFieldData* field)
{
	GpuHeightFieldEncoding encoding = {field->minHeight, field->heightScale, field->scale};
	// Native grids allow zero vertical scale. Encode the same flat vertices
	// without passing a singular scale to the GPU mesh implementation.
	// Keep invalid metadata on its original rejection path.
	if (field->scale.y == 0.0f && isfinite(field->minHeight) && isfinite(field->heightScale) &&
		field->heightScale >= 0.0f && isfinite(field->minHeight + field->heightScale * (float)UINT16_MAX) &&
		isfinite(field->scale.x) && field->scale.x > 0.0f &&
		isfinite(field->scale.z) && field->scale.z > 0.0f)
	{
		encoding.minHeight = 0.0f;
		encoding.heightScale = 0.0f;
		encoding.scale.y = 1.0f;
	}
	return encoding;
}

#endif
