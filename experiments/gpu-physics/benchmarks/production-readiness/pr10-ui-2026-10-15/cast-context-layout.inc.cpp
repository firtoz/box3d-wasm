struct ObservedCastContext
{
	b3Pos points[3];
	b3Vec3 normals[3];
	float fractions[3];
	uint64_t materialIds[3];
	int triangleIndices[3];
	int childIndices[3];
	int count;
	bool initialOverlap;
};
