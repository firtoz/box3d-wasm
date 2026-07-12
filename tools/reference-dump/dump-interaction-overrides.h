#pragma once

#include "sample.h"

struct DumpInteraction
{
	const char* action;
	float args[6];
};

void patch_dump_sample_entries();
bool apply_dump_interaction( Sample* sample, const char* sampleName, const DumpInteraction& interaction );
/** Optional checkpoint JSON fields (e.g. `"rays":{...}`) for the active dump sample. */
const char* get_dump_checkpoint_extras( Sample* sample, const char* sampleName );
