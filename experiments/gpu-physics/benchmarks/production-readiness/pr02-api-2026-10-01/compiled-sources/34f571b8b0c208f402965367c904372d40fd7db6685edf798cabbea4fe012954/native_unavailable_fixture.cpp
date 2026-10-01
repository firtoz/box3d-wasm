// Native initial-release exclusions: actual linked public C operations.
#include "box3d/box3d.h"
#include "native_api_status.h"
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <thread>
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
#endif

template<class Call> static void unavailable(const char* name, Call call) {
    gpu_b3_native_api_clear_error();
    assert(gpu_b3_native_api_last_error()==nullptr && errno==0);
    assert(gpu_b3_native_api_is_unavailable(name));
    call();
    assert(errno==ENOTSUP);
    assert(gpu_b3_native_api_last_error()!=nullptr);
    assert(std::strcmp(gpu_b3_native_api_last_error(),name)==0);
    (void)b3DefaultWorldDef();
    assert(std::strcmp(gpu_b3_native_api_last_error(),name)==0);
    std::printf("unavailable %s errno=%d\n",name,errno);
}

int main(int argc,char** argv) {
    assert(argc==2);
    assert(!std::filesystem::exists(argv[1]));
    assert(!gpu_b3_native_api_is_unavailable(nullptr));
    assert(!gpu_b3_native_api_is_unavailable("unknown-operation"));
    assert(!gpu_b3_native_api_is_unavailable("b3World_GetCounters"));
    unavailable("b3CreatePlayer", [&] { (void)b3CreatePlayer(nullptr, int{}, int{}); });
    unavailable("b3CreateRecording", [&] { (void)b3CreateRecording(int{}); });
    unavailable("b3DestroyPlayer", [&] { (void)b3DestroyPlayer(nullptr); });
    unavailable("b3DestroyRecording", [&] { (void)b3DestroyRecording(nullptr); });
    unavailable("b3LoadRecordingFromFile", [&] { (void)b3LoadRecordingFromFile(nullptr); });
    unavailable("b3RecPlayer_DrawFrameQueries", [&] { (void)b3RecPlayer_DrawFrameQueries(nullptr, nullptr, int{}, int{}); });
    unavailable("b3RecPlayer_GetBodyCount", [&] { (void)b3RecPlayer_GetBodyCount(nullptr); });
    unavailable("b3RecPlayer_GetBodyId", [&] { (void)b3RecPlayer_GetBodyId(nullptr, int{}); });
    unavailable("b3RecPlayer_GetDivergeFrame", [&] { (void)b3RecPlayer_GetDivergeFrame(nullptr); });
    unavailable("b3RecPlayer_GetFrame", [&] { (void)b3RecPlayer_GetFrame(nullptr); });
    unavailable("b3RecPlayer_GetFrameCount", [&] { (void)b3RecPlayer_GetFrameCount(nullptr); });
    unavailable("b3RecPlayer_GetFrameQuery", [&] { (void)b3RecPlayer_GetFrameQuery(nullptr, int{}); });
    unavailable("b3RecPlayer_GetFrameQueryCount", [&] { (void)b3RecPlayer_GetFrameQueryCount(nullptr); });
    unavailable("b3RecPlayer_GetFrameQueryHit", [&] { (void)b3RecPlayer_GetFrameQueryHit(nullptr, int{}, int{}); });
    unavailable("b3RecPlayer_GetInfo", [&] { (void)b3RecPlayer_GetInfo(nullptr); });
    unavailable("b3RecPlayer_GetKeyframeBudget", [&] { (void)b3RecPlayer_GetKeyframeBudget(nullptr); });
    unavailable("b3RecPlayer_GetKeyframeBytes", [&] { (void)b3RecPlayer_GetKeyframeBytes(nullptr); });
    unavailable("b3RecPlayer_GetKeyframeInterval", [&] { (void)b3RecPlayer_GetKeyframeInterval(nullptr); });
    unavailable("b3RecPlayer_GetKeyframeMinInterval", [&] { (void)b3RecPlayer_GetKeyframeMinInterval(nullptr); });
    unavailable("b3RecPlayer_GetWorldId", [&] { (void)b3RecPlayer_GetWorldId(nullptr); });
    unavailable("b3RecPlayer_HasDiverged", [&] { (void)b3RecPlayer_HasDiverged(nullptr); });
    unavailable("b3RecPlayer_IsAtEnd", [&] { (void)b3RecPlayer_IsAtEnd(nullptr); });
    unavailable("b3RecPlayer_IsAtPreStep", [&] { (void)b3RecPlayer_IsAtPreStep(nullptr); });
    unavailable("b3RecPlayer_Restart", [&] { (void)b3RecPlayer_Restart(nullptr); });
    unavailable("b3RecPlayer_SeekFrame", [&] { (void)b3RecPlayer_SeekFrame(nullptr, int{}); });
    unavailable("b3RecPlayer_SetDebugShapeCallbacks", [&] { (void)b3RecPlayer_SetDebugShapeCallbacks(nullptr, nullptr, nullptr, nullptr); });
    unavailable("b3RecPlayer_SetKeyframePolicy", [&] { (void)b3RecPlayer_SetKeyframePolicy(nullptr, size_t{}, int{}); });
    unavailable("b3RecPlayer_SetWorkerCount", [&] { (void)b3RecPlayer_SetWorkerCount(nullptr, int{}); });
    unavailable("b3RecPlayer_StepFrame", [&] { (void)b3RecPlayer_StepFrame(nullptr); });
    unavailable("b3RecPlayer_SubStepFrame", [&] { (void)b3RecPlayer_SubStepFrame(nullptr); });
    unavailable("b3Recording_GetData", [&] { (void)b3Recording_GetData(nullptr); });
    unavailable("b3Recording_GetSize", [&] { (void)b3Recording_GetSize(nullptr); });
    unavailable("b3SaveRecordingToFile", [&] { (void)b3SaveRecordingToFile(nullptr, nullptr); });
    unavailable("b3ValidateReplay", [&] { (void)b3ValidateReplay(nullptr, int{}, int{}); });
    unavailable("b3World_GetWorkerCount", [&] { (void)b3World_GetWorkerCount(b3WorldId{}); });
    unavailable("b3World_RebuildStaticTree", [&] { (void)b3World_RebuildStaticTree(b3WorldId{}); });
    unavailable("b3World_SetWorkerCount", [&] { (void)b3World_SetWorkerCount(b3WorldId{}, int{}); });
    unavailable("b3World_StartRecording", [&] { (void)b3World_StartRecording(b3WorldId{}, nullptr); });
    unavailable("b3World_StopRecording", [&] { (void)b3World_StopRecording(b3WorldId{}); });
    unavailable("b3CreateRecording",[] { assert(b3CreateRecording(0)==nullptr); });
    unavailable("b3SaveRecordingToFile",[&] { assert(!b3SaveRecordingToFile(nullptr,argv[1])); });
    assert(!std::filesystem::exists(argv[1]));
    std::thread other([] {
        assert(gpu_b3_native_api_last_error()==nullptr);
        unavailable("b3World_GetWorkerCount",[] { (void)b3World_GetWorkerCount(b3WorldId{}); });
        gpu_b3_native_api_clear_error();
    });
    other.join();
    assert(std::strcmp(gpu_b3_native_api_last_error(),"b3SaveRecordingToFile")==0);
    // errno is only valid immediately after the API call, before unrelated
    // filesystem/thread-library calls. The operation-name diagnostic is sticky.
    gpu_b3_native_api_clear_error();
    assert(gpu_b3_native_api_last_error()==nullptr && errno==0);
    std::puts("PASS all 39 unavailable operations; null recording, no file, sticky and thread-local errors");
}
