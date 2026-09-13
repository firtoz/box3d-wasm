#pragma once

#include "box3d/types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void gpu_sokol_bench_begin_frame(void);
void gpu_sokol_bench_village_drop(b3WorldId world, const char* sample, int frame);
void gpu_sokol_bench_mark(const char* phase);
void gpu_sokol_bench_end_frame(void);
void gpu_sokol_bench_note_step_request(int requested_step);
void gpu_sokol_bench_note_identities(int submitted_step, uint64_t snapshot_gen, int completed_step,
	int in_flight);
void gpu_sokol_bench_note_query(float wait_ms, float dispatch_ms, float map_ms, uint64_t copied_bytes,
	float encode_ms);
void gpu_sokol_bench_note_world(b3WorldId world);
void gpu_sokol_bench_configure(const char* json_path, int warmup, int timed, bool unpaced,
	bool completed_step, bool pause_script, int worker_count, int enable_sleep);
void gpu_sokol_bench_set_health_scan(bool enable);
uint32_t gpu_sokol_bench_scene_seed(uint32_t fallback);
uint32_t gpu_sokol_bench_gear_diagnostic(void);
void gpu_sokol_bench_note_switch(void);
int gpu_sokol_bench_sample_index_by_name(const char* name);
bool gpu_sokol_bench_active(void);
bool gpu_sokol_bench_unpaced(void);
bool gpu_sokol_bench_completed_step(void);
void gpu_sokol_bench_apply_script(bool* pause, int* single_step, int frame_index);
void gpu_sokol_bench_finish(int frames, int sokol_errors, const char* sample_name, const char* mode);
int gpu_sokol_bench_swap_interval(void);

#ifdef __cplusplus
}
#endif
