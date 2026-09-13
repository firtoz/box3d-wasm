#include "sokol_loading.h"
#include "imgui.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <cstring>
#include <string>
#include <algorithm>

#if defined(GPU_PHYSICS_SAMPLES)
extern "C" bool gpu_b3_world_loading_needed(b3WorldId);
extern "C" void gpu_b3_world_prepare_loading(b3WorldId, unsigned);
extern "C" void gpu_b3_loading_message(char*, size_t);
extern "C" const char* gpu_b3_world_gpu_fail(b3WorldId);
namespace {
std::thread worker;
std::atomic<int> stage{0}; // 1 buffers/common kernels, 2 collision, 3 ready
std::string failure;
bool active = false;
unsigned frames = 0;
std::chrono::steady_clock::time_point started;
}
bool gpu_loading_active() { return active; }
bool gpu_loading_poll(b3WorldId world) {
    if (!active && gpu_b3_world_loading_needed(world)) {
        if (worker.joinable()) worker.join();
        failure.clear();
        active = true;
        frames = 0;
        stage.store(1);
        started = std::chrono::steady_clock::now();
        worker = std::thread([world] {
            gpu_b3_world_prepare_loading(world, 1);
            stage.store(2);
            gpu_b3_world_prepare_loading(world, 2);
            if (const char* error = gpu_b3_world_gpu_fail(world); error && error[0]) {
                failure = error;
                stage.store(4);
            } else { stage.store(3); }
        });
    }
    if (active && stage.load() == 3) {
        worker.join();
        active = false;
        fprintf(stderr, "gpu-loading: ready after %.3f s, %u loading frames, world %u:%u (no physics steps)\n",
            std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count(), frames,
            unsigned(world.index1), unsigned(world.generation));
    }
    if (active) ++frames;
    return active;
}
void gpu_loading_draw() {
    const int current = stage.load();
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    char detail[256]{};
    gpu_b3_loading_message(detail, sizeof(detail));
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
    ImGui::TextColored(ImVec4(0.45f, 0.77f, 1.0f, 1.0f), "PREPARING SIMULATION");
    ImGui::Spacing();
    ImGui::Text("%c  Loading GPU physics     %.1f s", "|/-\\"[int(elapsed*5)%4], elapsed);
    ImGui::Separator();
    ImGui::TextUnformatted("Done    Scene created");
    ImGui::Text("%s    GPU buffers and common shaders", current > 1 ? "Done" : "Active");
    ImGui::Text("%s    Scene collision shader", current > 2 ? "Done" : current == 2 ? "Active" : "Waiting");
    ImGui::TextUnformatted("Waiting    Start simulation");
    ImGui::Spacing();
    ImGui::TextWrapped("%s", detail[0] ? detail : "Preparing GPU resources...");
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
bool gpu_loading_active() { return false; }
bool gpu_loading_poll(b3WorldId) { return false; }
void gpu_loading_draw() {}
void gpu_loading_shutdown() {}
#endif
