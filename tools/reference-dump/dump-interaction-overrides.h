#pragma once

#include "sample.h"

struct DumpInteraction
{
	const char* action;
	float args[6];
};

void patch_dump_sample_entries();
bool apply_dump_interaction( Sample* sample, const char* sampleName, const DumpInteraction& interaction );
