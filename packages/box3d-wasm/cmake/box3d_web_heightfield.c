#include "box3d_web_shared.h"

B3W_EXPORT int b3wCreateWave(int worldHandle, int rowCount, int columnCount, float scaleX, float scaleY, float scaleZ,
	float rowFrequency, float columnFrequency, int makeHoles)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL || rowCount < 1 || columnCount < 1) return 0;
	b3Vec3 scale = { scaleX, scaleY, scaleZ };
	b3HeightFieldData* heightField = b3CreateWave(rowCount, columnCount, scale, rowFrequency, columnFrequency, makeHoles != 0);
	if (heightField == NULL) return 0;
	return b3wAllocHeightFieldSlot(worldHandle, heightField);
}

B3W_EXPORT int b3wCreateGridHeightField(int worldHandle, int rowCount, int columnCount, float scaleX, float scaleY,
	float scaleZ, int makeHoles)
{
	b3wWorldSlot* world = b3wGetWorld(worldHandle);
	if (world == NULL || rowCount < 1 || columnCount < 1) return 0;
	b3Vec3 scale = { scaleX, scaleY, scaleZ };
	b3HeightFieldData* heightField = b3CreateGrid(rowCount, columnCount, scale, makeHoles != 0);
	if (heightField == NULL) return 0;
	return b3wAllocHeightFieldSlot(worldHandle, heightField);
}

B3W_EXPORT void b3wDestroyHeightField(int heightFieldHandle)
{
	b3wHeightFieldSlot* slot = b3wGetHeightField(heightFieldHandle);
	if (slot == NULL) return;
	b3DestroyHeightField(slot->heightField);
	b3wFreeHeightFieldSlot(heightFieldHandle);
}

B3W_EXPORT uint64_t b3wCreateHeightFieldShape(uint64_t bodyPacked, int heightFieldHandle, float density, float friction,
	float restitution, float rollingResistance, int isSensor)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	b3wHeightFieldSlot* heightField = b3wGetHeightField(heightFieldHandle);
	if (!b3Body_IsValid(bodyId) || heightField == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	shapeDef.baseMaterial.friction = friction;
	shapeDef.baseMaterial.restitution = restitution;
	shapeDef.baseMaterial.rollingResistance = rollingResistance;
	shapeDef.isSensor = isSensor != 0;
	b3ShapeId shapeId = b3CreateHeightFieldShape(bodyId, &shapeDef, heightField->heightField);
	return b3StoreShapeId(shapeId);
}
