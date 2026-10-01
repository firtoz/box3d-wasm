#include "box3d/box3d.h"

#include <stdint.h>
#include <cmath>
#include "contact_metrics.h"
#include "native_diagnostics.h"

#ifdef BOTH_SAMPLES
extern "C" void gpu_samples_get_cpu_stats(b3WorldId worldId, b3Profile* profile, b3Counters* counters);
#endif

extern "C" float gpu_b3_world_last_encode_ms(b3WorldId id);
extern "C" float gpu_b3_world_last_fetch_ms(b3WorldId id);
extern "C" float gpu_b3_world_last_collide_ms(b3WorldId id);
extern "C" float gpu_b3_world_last_solve_ms(b3WorldId id);
extern "C" float gpu_b3_world_last_integrate_ms(b3WorldId id);
extern "C" float gpu_b3_world_last_prepare_ms(b3WorldId id);
extern "C" float gpu_b3_world_last_device_ms(b3WorldId id);
extern "C" uint64_t gpu_b3_world_last_timestamp_step(b3WorldId id);
extern "C" uint64_t gpu_b3_world_physics_step(b3WorldId id);
extern "C" bool gpu_b3_world_pose_export_live(b3WorldId id);
extern "C" const char* gpu_b3_world_gpu_fail(b3WorldId id);
extern "C" bool gpu_gl_poses_imported(void);
extern "C" float gpu_samples_last_draw_list_ms(void);
extern "C" float gpu_samples_last_pose_prep_ms(void);
extern "C" float gpu_samples_last_import_ms(void);

#include "imgui.h"

extern "C" void gpu_samples_draw_sidebar(b3WorldId worldId)
{
	b3Counters gpu = {};
	gpu_b3_world_counts(worldId, &gpu.bodyCount, &gpu.shapeCount, &gpu.jointCount);

	ImGui::SeparatorText("Physics");
#ifdef BOTH_SAMPLES
	b3Profile cpuProfile = {};
	b3Counters cpu = {};
	gpu_samples_get_cpu_stats(worldId, &cpuProfile, &cpu);
	ImGui::Text("CPU  %.2f ms", cpuProfile.step);
	ImGui::TextDisabled("bodies %d  shapes %d", cpu.bodyCount, cpu.shapeCount);
	ImGui::TextDisabled("contacts %d  joints %d", cpu.contactCount, cpu.jointCount);
#endif
	float collide = gpu_b3_world_last_collide_ms(worldId);
	float solve = gpu_b3_world_last_solve_ms(worldId);
	float integrate = gpu_b3_world_last_integrate_ms(worldId);
	float prepare = gpu_b3_world_last_prepare_ms(worldId);
	float device = gpu_b3_world_last_device_ms(worldId);
	ImGui::Text("GPU device      %.2f ms  step %llu / phys %llu", device,
		(unsigned long long)gpu_b3_world_last_timestamp_step(worldId),
		(unsigned long long)gpu_b3_world_physics_step(worldId));
    GpuContactMetrics contacts = {};
    gpu_b3_world_contact_metrics(worldId, &contacts);
    if (!contacts.known) {
        ImGui::TextDisabled("GPU contact metrics pending");
    } else {
        ImGui::TextDisabled("GPU contact step %llu%s", (unsigned long long)contacts.snapshot_step,
            contacts.capacity_loss ? " INVALID" : contacts.current ? "" : " (older state)");
        ImGui::TextDisabled("roots %u  manifold slots %u  touching roots %u",
            contacts.allocated_roots, contacts.allocated_manifold_slots, contacts.touching_roots);
        ImGui::TextDisabled("non-sensor roots %u  unique pairs %u", contacts.non_sensor_roots, contacts.candidate_pairs);
    }
	ImGui::Text("GPU collide     %.2f ms", collide);
	ImGui::Text("GPU prepare     %.2f ms", prepare);
	ImGui::Text("GPU solve       %.2f ms", solve);
	ImGui::Text("GPU integrate   %.2f ms", integrate);
	ImGui::Text("GPU encode      %.2f ms", gpu_b3_world_last_encode_ms(worldId));
	ImGui::Text("GPU fetch       %.2f ms", gpu_b3_world_last_fetch_ms(worldId));
	ImGui::Text("CPU import      %.2f ms", gpu_samples_last_import_ms());
	ImGui::Text("CPU pose prep   %.2f ms", gpu_samples_last_pose_prep_ms());
	ImGui::Text("CPU draw list   %.2f ms", gpu_samples_last_draw_list_ms());
	ImGui::TextDisabled("CPU draw = import+pose+list; do not add to GPU device");
	if (const char* fail = gpu_b3_world_gpu_fail(worldId))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.25f, 0.2f, 1.0f));
		ImGui::TextWrapped("GPU FAILED\n%s", fail);
		ImGui::PopStyleColor();
	}
	ImGui::TextDisabled("device = GPU clock start..sleep");
	ImGui::TextDisabled("collide = bp+narrow+graph; prepare = islands");
	ImGui::TextDisabled("encode = submit; fetch = pose snapshot (not GL import)");
	ImGui::TextDisabled("draw list = Sokol DrawShape from CPU snapshot");
	if (gpu_b3_world_pose_export_live(worldId) && gpu_gl_poses_imported())
	{
		ImGui::TextDisabled("GL import: FD live, DrawShape still CPU snapshot");
	}
	else if (gpu_b3_world_pose_export_live(worldId))
	{
		ImGui::TextDisabled("poses: vk export, GL import pending");
	}
	else
	{
		ImGui::TextDisabled("poses: cpu snapshot; setters win over stale copy");
	}
	ImGui::TextDisabled("bodies %d  shapes %d  joints %d", gpu.bodyCount, gpu.shapeCount, gpu.jointCount);
}

extern "C" void gpu_samples_draw_native_profile(b3WorldId worldId)
{
    GpuNativeProfile profile = {};
    gpu_b3_world_native_profile(worldId, &profile);
    ImGui::Text("GPU device timestamps (ms), step %llu / physics %llu",
        (unsigned long long)profile.timestamp_step, (unsigned long long)profile.physics_step);
    if (profile.valid != GPU_DIAGNOSTIC_OK) {
        ImGui::TextDisabled("GPU profile unavailable (status %u)", profile.valid);
        return;
    }
    if (!profile.measured_fields) {
        ImGui::TextDisabled("GPU timestamps pending or unavailable on this adapter");
        return;
    }
    const unsigned fields[] = {0,1,2,3,4,9,10};
    const char* names[] = {"device start..sleep", "broadphase", "narrowphase + graph",
        "prepare + solve + integrate", "island preparation", "constraint solve aggregate",
        "integration aggregate"};
    for (unsigned row = 0; row < sizeof fields / sizeof *fields; ++row) {
        const unsigned field = fields[row];
        if ((profile.measured_fields & (1u << field)) && std::isfinite(profile.values[field]))
            ImGui::Text("%s: %.3f ms", names[row], profile.values[field]);
        else ImGui::TextDisabled("%s: unavailable", names[row]);
    }
    ImGui::TextDisabled("Aggregate rows overlap; do not add them.");
    ImGui::TextDisabled("CPU substage timings are unavailable for GPU physics.");
    if (profile.timestamp_step != profile.physics_step)
        ImGui::TextDisabled("Latest timestamp is from an older physics step.");
}

extern "C" void gpu_samples_draw_native_counters(b3WorldId worldId)
{
    b3Counters counters = {};
    const unsigned status = gpu_b3_world_native_counters(worldId, &counters);
    if (status != GPU_DIAGNOSTIC_OK) {
        ImGui::TextDisabled("GPU counters unavailable (status %u)", status);
        return;
    }
    ImGui::Text("public bodies / shapes / contacts / joints: %d / %d / %d / %d",
        counters.bodyCount, counters.shapeCount, counters.contactCount, counters.jointCount);
    if (counters.islandCount >= 0) ImGui::Text("completed-step islands: %d", counters.islandCount);
    else ImGui::TextDisabled("Island labels unavailable after topology mutation or before a step.");
    ImGui::TextDisabled("Explicit counters synchronize contact records.");
    int touching = 0;
    for (int bucket = 0; bucket < B3_CONTACT_MANIFOLD_COUNT_BUCKETS; ++bucket) {
        if (counters.manifoldCounts[bucket] < 0) continue;
        touching += counters.manifoldCounts[bucket];
        ImGui::Text("touching contacts with %d%s manifolds: %d", bucket + 1,
            bucket + 1 == B3_CONTACT_MANIFOLD_COUNT_BUCKETS ? "+" : "", counters.manifoldCounts[bucket]);
    }
    ImGui::Text("touching public contacts: %d", touching);
    ImGui::TextDisabled("CPU worker/tree/allocator/SAT/TOI and color/recycling counters unavailable.");
    b3Capacity peak = {};
    const unsigned peak_status = gpu_b3_world_native_max_capacity(worldId, &peak);
    if (peak_status == GPU_DIAGNOSTIC_OK) {
        ImGui::Text("peak live static shapes / bodies: %d / %d", peak.staticShapeCount, peak.staticBodyCount);
        ImGui::Text("peak live dynamic + kinematic shapes / bodies: %d / %d", peak.dynamicShapeCount, peak.dynamicBodyCount);
        ImGui::Text("peak pre-CCD non-sensor contact roots: %d", peak.contactCount);
    } else ImGui::TextDisabled("Peak occupancy unavailable (status %u)", peak_status);
    GpuNativeAllocation allocation = {};
    gpu_b3_world_native_allocation(worldId, &allocation);
    if (allocation.valid == GPU_DIAGNOSTIC_OK) {
        ImGui::Text("primary owned GPU physics buffers: %llu bytes",
            (unsigned long long)allocation.primary_buffer_bytes);
        ImGui::Text("reserved body / shape / joint / contact slots: %u / %u / %u / %u",
            allocation.body_capacity, allocation.shape_capacity, allocation.joint_capacity, allocation.contact_capacity);
        ImGui::TextDisabled("Excludes staging, cache copies, pipelines, renderer and driver allocations.");
    }
}
