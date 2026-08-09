#include "box3d_web_shared.h"

#include <stdlib.h>

B3W_EXPORT int b3wCreateCompound(int capsuleCount, int hullCount, int meshCount, int sphereCount, const b3CompoundCapsuleDef* capsules,
					  const b3CompoundHullDef* hulls, const b3CompoundMeshDef* meshes, const b3CompoundSphereDef* spheres)
{
	b3CompoundDef def = { 0 };
	def.capsules = (b3CompoundCapsuleDef*)capsules;
	def.capsuleCount = capsuleCount;
	def.hulls = (b3CompoundHullDef*)hulls;
	def.hullCount = hullCount;
	def.meshes = (b3CompoundMeshDef*)meshes;
	def.meshCount = meshCount;
	def.spheres = (b3CompoundSphereDef*)spheres;
	def.sphereCount = sphereCount;
	b3CompoundData* compound = b3CreateCompound(&def);
	return b3wAllocCompoundSlot(compound);
}

B3W_EXPORT int b3wCreateCompoundFromHulls(int hullCount, const float* hullData, int strideFloats)
{
	if (hullCount <= 0) return 0;

	b3BoxHull* boxHulls = (b3BoxHull*)malloc(sizeof(b3BoxHull) * hullCount);
	b3CompoundHullDef* hullDefs = (b3CompoundHullDef*)malloc(sizeof(b3CompoundHullDef) * hullCount);

	for (int i = 0; i < hullCount; i++)
	{
		const float* d = hullData + i * strideFloats;
		boxHulls[i] = b3MakeBoxHull(d[0], d[1], d[2]);
		hullDefs[i].hull = &boxHulls[i].base;
		hullDefs[i].transform.p.x = d[3];
		hullDefs[i].transform.p.y = d[4];
		hullDefs[i].transform.p.z = d[5];
		hullDefs[i].transform.q.v.x = d[6];
		hullDefs[i].transform.q.v.y = d[7];
		hullDefs[i].transform.q.v.z = d[8];
		hullDefs[i].transform.q.s = d[9];
		hullDefs[i].material = b3DefaultSurfaceMaterial();
		hullDefs[i].material.friction = d[10];
		hullDefs[i].material.restitution = d[11];
		hullDefs[i].material.rollingResistance = d[12];
	}

	b3CompoundDef def = { 0 };
	def.hulls = hullDefs;
	def.hullCount = hullCount;

	b3CompoundData* compound = b3CreateCompound(&def);
	free(hullDefs);
	free(boxHulls);

	return b3wAllocCompoundSlot(compound);
}

B3W_EXPORT int b3wCreateCompoundFromSpheres(int sphereCount, const float* sphereData, int strideFloats)
{
	if (sphereCount <= 0) return 0;

	b3CompoundSphereDef* sphereDefs = (b3CompoundSphereDef*)malloc(sizeof(b3CompoundSphereDef) * sphereCount);

	for (int i = 0; i < sphereCount; i++)
	{
		const float* d = sphereData + i * strideFloats;
		sphereDefs[i].sphere.center.x = d[0];
		sphereDefs[i].sphere.center.y = d[1];
		sphereDefs[i].sphere.center.z = d[2];
		sphereDefs[i].sphere.radius = d[3];
		sphereDefs[i].material = b3DefaultSurfaceMaterial();
		sphereDefs[i].material.friction = d[4];
		sphereDefs[i].material.restitution = d[5];
		sphereDefs[i].material.rollingResistance = d[6];
	}

	b3CompoundDef def = { 0 };
	def.spheres = sphereDefs;
	def.sphereCount = sphereCount;

	b3CompoundData* compound = b3CreateCompound(&def);
	free(sphereDefs);

	return b3wAllocCompoundSlot(compound);
}

B3W_EXPORT void b3wDestroyCompound(int compoundHandle)
{
	b3wCompoundSlot* slot = b3wGetCompound(compoundHandle);
	if (slot == NULL) return;
	b3DestroyCompound(slot->compound);
	b3wFreeCompoundSlot(compoundHandle);
}

B3W_EXPORT int b3wGetCompoundTreeHeight(int compoundHandle)
{
	b3wCompoundSlot* compound = b3wGetCompound(compoundHandle);
	if (compound == NULL) return 0;
	return b3DynamicTree_GetHeight(&compound->compound->tree);
}

B3W_EXPORT uint64_t b3wCreateCompoundShape(uint64_t bodyPacked, int compoundHandle, float density)
{
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	b3wCompoundSlot* compound = b3wGetCompound(compoundHandle);
	if (!b3Body_IsValid(bodyId) || compound == NULL) return 0;
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = density;
	b3ShapeId shapeId = b3CreateBakedCompoundShape(bodyId, &shapeDef, compound->compound);
	return b3StoreShapeId(shapeId);
}

B3W_EXPORT int b3wCreateCompoundFromMeshes(int meshCount, const float* meshData, int strideFloats)
{
	if (meshCount <= 0 || meshData == NULL) return 0;

	b3CompoundMeshDef* meshDefs = (b3CompoundMeshDef*)malloc(sizeof(b3CompoundMeshDef) * (size_t)meshCount);
	b3SurfaceMaterial* materials = (b3SurfaceMaterial*)malloc(sizeof(b3SurfaceMaterial) * (size_t)meshCount);
	if (meshDefs == NULL || materials == NULL)
	{
		free(meshDefs);
		free(materials);
		return 0;
	}

	for (int i = 0; i < meshCount; ++i)
	{
		const float* d = meshData + i * strideFloats;
		int meshHandle = (int)d[0];
		b3wMeshSlot* mesh = b3wGetMesh(meshHandle);
		if (mesh == NULL)
		{
			free(meshDefs);
			free(materials);
			return 0;
		}
		materials[i] = b3DefaultSurfaceMaterial();
		materials[i].friction = d[11];
		materials[i].restitution = d[12];
		materials[i].rollingResistance = d[13];
		meshDefs[i].meshData = mesh->mesh;
		meshDefs[i].transform.p.x = d[1];
		meshDefs[i].transform.p.y = d[2];
		meshDefs[i].transform.p.z = d[3];
		meshDefs[i].transform.q.v.x = d[4];
		meshDefs[i].transform.q.v.y = d[5];
		meshDefs[i].transform.q.v.z = d[6];
		meshDefs[i].transform.q.s = d[7];
		meshDefs[i].scale.x = d[8];
		meshDefs[i].scale.y = d[9];
		meshDefs[i].scale.z = d[10];
		meshDefs[i].materials = &materials[i];
		meshDefs[i].materialCount = 1;
	}

	b3CompoundDef def = { 0 };
	def.meshes = meshDefs;
	def.meshCount = meshCount;
	b3CompoundData* compound = b3CreateCompound(&def);
	free(meshDefs);
	free(materials);
	if (compound == NULL) return 0;
	return b3wAllocCompoundSlot(compound);
}
