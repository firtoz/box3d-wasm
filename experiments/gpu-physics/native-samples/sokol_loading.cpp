#include "sokol_loading.h"
#include "imgui.h"
#include "sokol_app.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <cstring>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <exception>

#if defined(GPU_PHYSICS_SAMPLES)
extern "C" uint64_t gpu_b3_shared_device_startup_ns();
extern "C" bool gpu_b3_world_loading_needed(b3WorldId);
extern "C" void gpu_b3_world_prepare_loading(b3WorldId, unsigned);
extern "C" void gpu_b3_loading_message(char*, size_t);
extern "C" const char* gpu_b3_world_gpu_fail(b3WorldId);
namespace {
std::thread worker;
thread_local bool scene_worker = false;
std::atomic<int> pending_mouse{-1};
std::atomic<int> stage{0}; // 1 buffers/common kernels, 2 collision, 3 ready
std::string failure;
std::string original_title;
bool active = false;
std::atomic<bool> close_pending{false};
unsigned frames = 0;
std::chrono::steady_clock::time_point started;
std::function<b3WorldId()> pending_scene;
b3WorldId prepared_world{};
double scene_ms = 0, gpu_ms = 0, device_ms = 0, first_loading_ms = -1;
bool initial_startup = false;
bool startup_report_pending = false;
void prepare_gpu(b3WorldId world) {
    prepared_world = world;
    if (close_pending.load()) { stage.store(3); return; }
    const auto gpu_started = std::chrono::steady_clock::now();
    stage.store(1);
    gpu_b3_world_prepare_loading(world, 1);
    if (close_pending.load()) { stage.store(3); return; }
    stage.store(2);
    gpu_b3_world_prepare_loading(world, 2);
    gpu_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now()-gpu_started).count();
    prepared_world = world;
    if (const char* error = gpu_b3_world_gpu_fail(world); error && error[0]) {
        failure = error; stage.store(4);
    } else { stage.store(3); }
}
}
void gpu_loading_begin_startup() {
    started = std::chrono::steady_clock::now(); initial_startup = true;
}
void gpu_loading_lock_mouse(bool lock) {
    if (scene_worker) pending_mouse.store(lock ? 1 : 0);
    else sapp_lock_mouse(lock);
}
bool gpu_loading_on_scene_worker() { return scene_worker; }
void gpu_loading_request_close() { close_pending = true; }
bool gpu_loading_close_pending() { return close_pending; }
bool gpu_loading_active() { return active; }
void gpu_loading_create_scene(std::function<b3WorldId()> create) {
    if (active) return;
    if (worker.joinable()) worker.join();
    original_title = sapp_query_desc().window_title;
    sapp_set_window_title("Preparing simulation: creating scene");
    pending_scene = std::move(create);
    failure.clear(); active = true; frames = 0;
    scene_ms = gpu_ms = 0; first_loading_ms = -1;
    startup_report_pending = true;
    if (!initial_startup) started = std::chrono::steady_clock::now();
    initial_startup = false;
    stage.store(0);
}
bool gpu_loading_poll(b3WorldId world) {
    if (close_pending.load() && stage.load() == 4) {
        if (worker.joinable()) worker.join();
        active = false;
        return false;
    }
    if (close_pending && pending_scene) {
        pending_scene = {}; active = false;
        return false;
    }
    // Present the loading UI once before handing exclusive scene ownership to
    // the worker. While active, the host must not read SampleContext or world.
    if (active && pending_scene && frames > 0) {
        auto create = std::move(pending_scene);
        worker = std::thread([create = std::move(create)] {
            scene_worker = true;
            try {
                const auto scene_started = std::chrono::steady_clock::now();
                const uint64_t device_before = gpu_b3_shared_device_startup_ns();
                const b3WorldId world = create();
                scene_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now()-scene_started).count();
                device_ms = double(gpu_b3_shared_device_startup_ns() - device_before) / 1e6;
                scene_ms -= device_ms;
                prepare_gpu(world);
            } catch (const std::exception& error) { failure = error.what(); stage.store(4); }
        });
    }
    if (!active && gpu_b3_world_loading_needed(world)) {
        if (worker.joinable()) worker.join();
        failure.clear(); active = true; frames = 0;
        started = std::chrono::steady_clock::now(); stage.store(1);
        worker = std::thread([world] { prepare_gpu(world); });
    }
    if (active && stage.load() == 3) {
        worker.join(); active = false;
        if (!original_title.empty()) sapp_set_window_title(original_title.c_str());
        const int mouse = pending_mouse.exchange(-1);
        if (mouse >= 0) sapp_lock_mouse(mouse != 0);
        fprintf(stderr, "gpu-loading: ready after %.3f s, %u loading frames, world %u:%u (no physics steps)\n",
            std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count(), frames,
            unsigned(prepared_world.index1), unsigned(prepared_world.generation));
    }
    return active;
}
void gpu_loading_presented() {
    const double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now()-started).count();
    if (active) {
        ++frames;
        if (first_loading_ms < 0) first_loading_ms = elapsed;
    } else if (startup_report_pending) {
        startup_report_pending = false;
        fprintf(stderr,"startup: scene=%.3f ms gpu=%.3f ms first-scene-frame=%.3f ms loading-frames=%u\n",scene_ms,gpu_ms,elapsed,frames);
        if (const char* path = std::getenv("GPU_PHYSICS_STARTUP_REPORT")) {
            if (FILE* file = std::fopen(path,"w")) {
                const char* cubes = std::getenv("GPU_BENCH_CUBES");
                std::fprintf(file,"{\"schema\":\"startup-v1\",\"mode\":\"sokol-renderer\",\"dynamic_cubes\":%lu,\"device_ms\":%.9f,\"scene_ms\":%.9f,\"gpu_prepare_ms\":%.9f,\"first_frame_ms\":%.9f,\"first_loading_frame_ms\":%.9f,\"loading_frames\":%u}\n",cubes?std::strtoul(cubes,nullptr,10):0,device_ms,scene_ms,gpu_ms,elapsed,first_loading_ms,frames);
                std::fclose(file);
            }
        }
    }
}
void gpu_loading_draw() {
    const int current = stage.load();
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    char detail[256]{};
    if (current > 0) gpu_b3_loading_message(detail, sizeof(detail));
    const auto& io = ImGui::GetIO();
    if (current == 4) {
        ImGui::SetNextWindowPos(ImVec2(30,30));
        ImGui::Begin("GPU preparation failed");
        ImGui::TextWrapped("%s", failure.c_str());
        ImGui::TextUnformatted("Close the window to exit. The simulation has not advanced.");
        ImGui::End();
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(std::min(560.0f, io.DisplaySize.x - 32.0f), 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28, 24));
    ImGui::Begin("Preparing simulation", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    ImGui::TextColored(ImVec4(0.45f, 0.77f, 1.0f, 1.0f), "%s", close_pending ? "CLOSING SIMULATION" : "PREPARING SIMULATION");
    ImGui::Spacing();
    ImGui::Text("%c  Loading GPU physics     %.1f s", "|/-\\"[int(elapsed*5)%4], elapsed);
    ImGui::Separator();
    ImGui::Text("%s    Create scene", current > 0 ? "Done" : "Active");
    ImGui::Text("%s    GPU buffers and common shaders", current > 1 ? "Done" : current == 1 ? "Active" : "Waiting");
    ImGui::Text("%s    Scene collision shader", current > 2 ? "Done" : current == 2 ? "Active" : "Waiting");
    ImGui::TextUnformatted(close_pending ? "Waiting    Finish current operation and close" : "Waiting    Start simulation");
    ImGui::Spacing();
    ImGui::TextWrapped("%s", detail[0] ? detail : current == 0 ? "Creating scene objects..." : "Preparing GPU resources...");
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("First-time shader compilation can take a few minutes.");
    ImGui::TextWrapped("Progress advances when each operation finishes.");
    ImGui::PopStyleColor();
    ImGui::End();
    ImGui::PopStyleVar();
}
void gpu_loading_shutdown() { if (worker.joinable()) worker.join(); }
#else
void gpu_loading_create_scene(std::function<b3WorldId()> create) { create(); }
void gpu_loading_presented() {}
void gpu_loading_begin_startup() {}
void gpu_loading_lock_mouse(bool lock) { sapp_lock_mouse(lock); }
bool gpu_loading_on_scene_worker() { return false; }
void gpu_loading_request_close() {}
bool gpu_loading_close_pending() { return false; }
bool gpu_loading_active() { return false; }
bool gpu_loading_poll(b3WorldId) { return false; }
void gpu_loading_draw() {}
void gpu_loading_shutdown() {}
#endif
