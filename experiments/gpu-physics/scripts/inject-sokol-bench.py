#!/usr/bin/env python3
"""Copy Box3D sample sources and inject Sokol bench hooks. Fail if anchors drift."""
from __future__ import annotations

import argparse
import sokol_split_injection
from pathlib import Path


def require(haystack: str, needle: str, label: str) -> None:
    n = haystack.count(needle)
    if n != 1:
        raise SystemExit(
            f"sokol-bench inject ({label}): expected exactly one occurrence of {needle!r}, found {n}"
        )


def inject_main(text: str) -> str:
    # Loading must precede bench clocks and any world access. Only the loading
    # callback runs while the worker owns the world; OpenGL remains on UI thread.
    for anchor in ['\tDrawUI( &s_context );', 'static void OnFrame( void )\n{',
                   'static void OnEvent( const sapp_event* e )\n{', 'static void OnCleanup( void )\n{']:
        require(text, anchor, "loading hooks")
    text = text.replace('#include "sokol_glue.h"\n', '#include "sokol_loading.h"\n#include "sokol_glue.h"\n')
    text = text.replace('\tDrawUI( &s_context );', '\tif (gpu_loading_active()) { gpu_loading_draw(); return; }\n\tDrawUI( &s_context );')
    text = text.replace('static void OnFrame( void )\n{', 'static void OnFrame( void )\n{\n    if (s_context.sample && gpu_loading_poll(s_context.sample->m_worldId)) {\n        if (sapp_width() > 0 && sapp_height() > 0) {\n            ResetFrameArena();\n            const sg_swapchain sc = sglue_swapchain();\n            sg_pass pass{};\n            pass.swapchain = sc;\n            pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;\n            pass.action.colors[0].clear_value = {0.035f, 0.045f, 0.065f, 1.0f};\n            sg_begin_pass(&pass);\n            sg_end_pass();\n            StartUIFrame((float)sapp_frame_duration());\n            RenderUI(&sc);\n            sg_commit();\n        }\n        std::this_thread::sleep_for(std::chrono::milliseconds(16));\n        return;\n    }\n')
    text = text.replace('static void OnEvent( const sapp_event* e )\n{', 'static void OnEvent( const sapp_event* e )\n{\n    if (gpu_loading_active()) { HandleEvent(e); return; }')
    text = text.replace('static void OnCleanup( void )\n{', 'static void OnCleanup( void )\n{\n    gpu_loading_shutdown();')
    anchors = [
        '#include "sokol_glue.h"\n',
        "static int s_sampleOverride = -1;\n",
        "\tSelectSample( &s_context, index, false );\n",
        "\tconst uint64_t frameStart = b3GetTicks();\n",
        "\ts_context.sample->Step();\n",
        "\tRenderFrame( &sc, &fi );\n",
        "\tsg_commit();\n",
        '\t\telse if ( strcmp( argv[i], "--sample" ) == 0 && i + 1 < argc )\n',
        '\tfprintf( stderr, "samples: %d frames, %d sokol errors\\n", s_frame, errors );\n',
    ]
    for a in anchors:
        require(text, a, "main.cpp")

    text = text.replace(
        '#include "sokol_glue.h"\n',
        '#include "sokol_glue.h"\n#include "sokol_bench_hooks.h"\n'
        "#if defined(BOTH_SAMPLES)\n"
        "static const char* kGpuSokolBenchMode = \"both\";\n"
        "#elif defined(GPU_PHYSICS_SAMPLES)\n"
        "static const char* kGpuSokolBenchMode = \"gpu\";\n"
        "#else\n"
        "static const char* kGpuSokolBenchMode = \"cpu\";\n"
        "#endif\n",
    )
    text = text.replace(
        "static int s_sampleOverride = -1;\n",
        "static int s_sampleOverride = -1;\n"
        "static char s_sampleName[128];\n"
        "static char s_benchJson[1024];\n"
        "static int s_benchWarmup = 20;\n"
        "static int s_benchTimed = 60;\n"
        "static int s_benchUnpaced = 1;\n"
        "static int s_benchCompleted = 0;\n"
        "static int s_benchPauseScript = 0;\n"
        "static int s_benchSleep = 1;\n"
		"static int s_benchWorkers = 0;\n"
		"static char s_switchName[128];\n"
		"static int s_switchAfter = -1;\n"
		"static int s_healthScan = 0;\n",
    )
    text = text.replace(
        "\tSelectSample( &s_context, index, false );\n",
        "\tif ( s_benchJson[0] && s_sampleName[0] == 0 )\n"
        "\t{\n"
        "\t\tfprintf( stderr, \"sokol-bench: --sample-name is required\\n\" );\n"
        "\t\ts_exitCode = 1;\n"
        "\t\tsapp_quit();\n"
        "\t\treturn;\n"
        "\t}\n"
        "\tif ( s_sampleName[0] )\n"
        "\t{\n"
        "\t\tint named = gpu_sokol_bench_sample_index_by_name( s_sampleName );\n"
        "\t\tif ( named < 0 )\n"
        "\t\t{\n"
        "\t\t\ts_exitCode = 1;\n"
        "\t\t\tsapp_quit();\n"
        "\t\t\treturn;\n"
        "\t\t}\n"
        "\t\tindex = named;\n"
        "\t}\n"
        "\tif ( s_benchJson[0] )\n"
        "\t{\n"
        "\t\tint workers = s_benchWorkers > 0 ? s_benchWorkers : s_context.workerCount;\n"
        "\t\tgpu_sokol_bench_configure( s_benchJson, s_benchWarmup, s_benchTimed, s_benchUnpaced != 0,\n"
        "\t\t\ts_benchCompleted != 0, s_benchPauseScript != 0, workers, s_benchSleep );\n"
        "\t\tgpu_sokol_bench_set_health_scan( s_healthScan != 0 );\n"
        "\t\ts_context.enableSleep = s_benchSleep != 0;\n"
        "\t\ts_context.workerCount = workers;\n"
        "\t\ts_context.hertz = 60.0f;\n"
        "\t\ts_context.subStepCount = 4;\n"
        "\t\tif ( s_frameLimit < 0 )\n"
        "\t\t{\n"
        "\t\t\ts_frameLimit = s_benchWarmup + s_benchTimed + ( s_benchPauseScript ? 12 : 0 );\n"
        "\t\t}\n"
        "\t}\n"
        "\tSelectSample( &s_context, index, false );\n",
    )
    text = text.replace(
        "\tconst uint64_t frameStart = b3GetTicks();\n",
        "\tconst uint64_t frameStart = b3GetTicks();\n"
        "\tgpu_sokol_bench_begin_frame();\n"
        "\tgpu_sokol_bench_apply_script( &s_context.pause, &s_context.singleStep, s_frame );\n"
        "\tif ( s_switchName[0] && s_switchAfter >= 0 && s_frame == s_switchAfter )\n"
        "\t{\n"
        "\t\tint named = gpu_sokol_bench_sample_index_by_name( s_switchName );\n"
        "\t\tif ( named >= 0 )\n"
        "\t\t{\n"
        "\t\t\tSelectSample( &s_context, named, false );\n"
        "\t\t\tgpu_sokol_bench_note_switch();\n"
        "\t\t\ts_switchAfter = -1;\n"
        "\t\t\treturn;\n"
        "\t\t}\n"
        "\t}\n",
    )
    text = text.replace(
        "\ts_context.sample->Step();\n",
        "\tgpu_sokol_bench_village_drop( s_context.sample->m_worldId, s_sampleName, s_frame );\n"
        "\tgpu_sokol_bench_note_step_request( s_context.sample ? s_context.sample->m_stepCount + 1 : 0 );\n"
        "\ts_context.sample->Step();\n",
    )
    text = text.replace(
        "\tRenderFrame( &sc, &fi );\n",
        "\tgpu_sokol_bench_mark( \"render\" );\n"
        "\tRenderFrame( &sc, &fi );\n"
        "\tgpu_sokol_bench_mark( \"ui\" );\n",
    )
    text = text.replace(
        "\tsg_commit();\n",
        "\tgpu_sokol_bench_mark( \"commit\" );\n"
        "\tsg_commit();\n"
        "\tgpu_sokol_bench_mark( \"limiter\" );\n"
        "\tgpu_sokol_bench_end_frame();\n",
    )
    limiter = (
        "\tif ( s_frameLimit < 0 )\n"
        "\t{\n"
        "\t\tLimitFrameRate( frameStart );\n"
        "\t}\n"
    )
    require(text, limiter, "main.cpp limiter")
    text = text.replace(
        limiter,
        "\tif ( gpu_sokol_bench_active() )\n"
        "\t{\n"
        "\t\tif ( gpu_sokol_bench_unpaced() == false )\n"
        "\t\t{\n"
        "\t\t\tLimitFrameRate( frameStart );\n"
        "\t\t}\n"
        "\t}\n"
        "\telse if ( s_frameLimit < 0 )\n"
        "\t{\n"
        "\t\tLimitFrameRate( frameStart );\n"
        "\t}\n",
    )
    text = text.replace(
        '\t\tif ( strcmp( argv[i], "--frames" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        "\t\t\ts_frameLimit = atoi( argv[++i] );\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--sample" ) == 0 && i + 1 < argc )\n',
        '\t\tif ( strcmp( argv[i], "--frames" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        "\t\t\ts_frameLimit = atoi( argv[++i] );\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--sample-name" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        '\t\t\tsnprintf( s_sampleName, sizeof s_sampleName, "%s", argv[++i] );\n'
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--bench-json" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        '\t\t\tsnprintf( s_benchJson, sizeof s_benchJson, "%s", argv[++i] );\n'
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--warmup" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        "\t\t\ts_benchWarmup = atoi( argv[++i] );\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--timed" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        "\t\t\ts_benchTimed = atoi( argv[++i] );\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--paced" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_benchUnpaced = 0;\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--unpaced" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_benchUnpaced = 1;\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--completed-step" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_benchCompleted = 1;\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--pause-script" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_benchPauseScript = 1;\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--sleep" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_benchSleep = 1;\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--no-sleep" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_benchSleep = 0;\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--workers" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        "\t\t\ts_benchWorkers = atoi( argv[++i] );\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--health-scan" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_healthScan = 1;\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--switch-sample-name" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        '\t\t\tsnprintf( s_switchName, sizeof s_switchName, "%s", argv[++i] );\n'
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--switch-after" ) == 0 && i + 1 < argc )\n'
        "\t\t{\n"
        "\t\t\ts_switchAfter = atoi( argv[++i] );\n"
        "\t\t}\n"
        '\t\telse if ( strcmp( argv[i], "--sample" ) == 0 && i + 1 < argc )\n',
    )
    text = text.replace(
        '\tfprintf( stderr, "samples: %d frames, %d sokol errors\\n", s_frame, errors );\n',
        '\tfprintf( stderr, "samples: %d frames, %d sokol errors\\n", s_frame, errors );\n'
        "\tgpu_sokol_bench_finish( s_frame, errors, s_sampleName, kGpuSokolBenchMode );\n",
    )
    return text


def inject_sample(text: str) -> str:
    anchors = [
        '#include "sample.h"\n',
        "\tb3World_EnableSleeping( m_worldId, m_context->enableSleep );\n",
        "\t\tb3World_Step( m_worldId, timeStep, m_context->subStepCount );\n",
        "\t\tm_profiles[m_currentProfileIndex] = b3World_GetProfile( m_worldId );\n",
        "\t\tfilter.name = \"pick\";\n"
        "\t\tb3RayResult result = b3World_CastRayClosest( m_worldId, pickRay.origin, pickRay.translation, filter );\n",
        "\tb3World_Draw( m_worldId, &debugDraw, B3_DEFAULT_MASK_BITS );\n",
    ]
    for a in anchors:
        require(text, a, "sample.cpp")
    text = text.replace(
        '#include "sample.h"\n',
        '#include "sample.h"\n#include "sokol_bench_hooks.h"\n'
        "#if defined(GPU_PHYSICS_SAMPLES)\n"
        "extern \"C\" void b3World_Wait( b3WorldId worldId );\n"
        "extern \"C\" uint64_t gpu_b3_world_physics_step( b3WorldId worldId );\n"
        "extern \"C\" uint64_t gpu_b3_world_pose_snapshot_epoch( b3WorldId worldId );\n"
        "extern \"C\" uint64_t gpu_b3_world_pose_snapshot_step( b3WorldId worldId );\n"
        "extern \"C\" uint64_t gpu_b3_world_completed_step( b3WorldId worldId );\n"
        "extern \"C\" int gpu_b3_world_completed_known( b3WorldId worldId );\n"
        "extern \"C\" uint64_t gpu_b3_world_last_timestamp_step( b3WorldId worldId );\n"
        "struct GpuQueryProfile { float wait_ms; float dispatch_ms; float map_ms; uint64_t copied_bytes; float encode_ms; };\n"
        "extern \"C\" GpuQueryProfile gpu_b3_world_last_query_profile( b3WorldId worldId );\n"
        "extern \"C\" void gpu_sokol_bench_note_query( float wait_ms, float dispatch_ms, float map_ms, uint64_t copied_bytes, float encode_ms );\n"
        "#endif\n",
    )
    text = text.replace(
        "\tb3World_EnableSleeping( m_worldId, m_context->enableSleep );\n"
        "\tb3World_EnableWarmStarting( m_worldId, m_context->enableWarmStarting );\n"
        "\tb3World_EnableContinuous( m_worldId, m_context->enableContinuous );\n",
        "\tgpu_sokol_bench_mark( \"setters\" );\n"
        "\tb3World_EnableSleeping( m_worldId, m_context->enableSleep );\n"
        "\tb3World_EnableWarmStarting( m_worldId, m_context->enableWarmStarting );\n"
        "\tb3World_EnableContinuous( m_worldId, m_context->enableContinuous );\n",
    )
    text = text.replace(
        "\t\tb3World_Step( m_worldId, timeStep, m_context->subStepCount );\n",
        "\t\tgpu_sokol_bench_mark( \"physics\" );\n"
        "\t\tb3World_Step( m_worldId, timeStep, m_context->subStepCount );\n"
        "#if defined(GPU_PHYSICS_SAMPLES)\n"
        "\t\tif ( gpu_sokol_bench_completed_step() )\n"
        "\t\t{\n"
        "\t\t\tb3World_Wait( m_worldId );\n"
        "\t\t}\n"
        "#endif\n"
        "\t\tgpu_sokol_bench_note_world( m_worldId );\n",
    )
    text = text.replace(
        "\t\tm_profiles[m_currentProfileIndex] = b3World_GetProfile( m_worldId );\n",
        "\t\tgpu_sokol_bench_mark( \"profile\" );\n"
        "\t\tm_profiles[m_currentProfileIndex] = b3World_GetProfile( m_worldId );\n",
    )
    text = text.replace(
        "\t\tfilter.name = \"pick\";\n"
        "\t\tb3RayResult result = b3World_CastRayClosest( m_worldId, pickRay.origin, pickRay.translation, filter );\n",
        "\t\tfilter.name = \"pick\";\n"
        "\t\tgpu_sokol_bench_mark( \"pick\" );\n"
        "\t\tb3RayResult result = b3World_CastRayClosest( m_worldId, pickRay.origin, pickRay.translation, filter );\n"
        "#if defined(GPU_PHYSICS_SAMPLES)\n"
        "\t\t{\n"
        "\t\t\tGpuQueryProfile qp = gpu_b3_world_last_query_profile( m_worldId );\n"
        "\t\t\tgpu_sokol_bench_note_query( qp.wait_ms, qp.dispatch_ms, qp.map_ms, qp.copied_bytes, qp.encode_ms );\n"
        "\t\t}\n"
        "#endif\n",
    )
    text = text.replace(
        "\tb3World_Draw( m_worldId, &debugDraw, B3_DEFAULT_MASK_BITS );\n",
        "\tgpu_sokol_bench_mark( \"draw\" );\n"
        "\tb3World_Draw( m_worldId, &debugDraw, B3_DEFAULT_MASK_BITS );\n"
        "\t{\n"
        "\t\tint submitted = m_stepCount;\n"
        "\t\tint completed = m_stepCount;\n"
        "\t\tuint64_t snap = 0;\n"
        "\t\tint in_flight = 0;\n"
        "#if defined(GPU_PHYSICS_SAMPLES)\n"
        "\t\tsubmitted = (int)gpu_b3_world_physics_step( m_worldId );\n"
        "\t\tsnap = gpu_b3_world_pose_snapshot_epoch( m_worldId );\n"
        "\t\tint pose_step = (int)gpu_b3_world_pose_snapshot_step( m_worldId );\n"
        "\t\tif ( pose_step != 0 )\n"
        "\t\t{\n"
        "\t\t\tsnap = (uint64_t)pose_step;\n"
        "\t\t}\n"
        "\t\tif ( gpu_sokol_bench_completed_step() )\n"
        "\t\t{\n"
        "\t\t\tcompleted = submitted;\n"
        "\t\t\tin_flight = 0;\n"
        "\t\t}\n"
        "\t\telse if ( gpu_b3_world_completed_known( m_worldId ) )\n"
        "\t\t{\n"
        "\t\t\tcompleted = (int)gpu_b3_world_completed_step( m_worldId );\n"
        "\t\t\tin_flight = completed != submitted;\n"
        "\t\t}\n"
        "\t\telse\n"
        "\t\t{\n"
        "\t\t\tcompleted = 0;\n"
        "\t\t\tin_flight = -1;\n"
        "\t\t}\n"
        "#endif\n"
        "\t\tgpu_sokol_bench_note_identities( submitted, snap, completed, in_flight );\n"
        "\t}\n",
    )
    return text


def inject_gpu_limitations(text: str) -> str:
    warm = 'ImGui::Checkbox( "Warm Starting##Solver", &context->enableWarmStarting );'
    require(text, warm, 'unsupported warm-start control')
    text = text.replace(warm, 'ImGui::TextDisabled( "Warm starting: always enabled on GPU" );')
    workers = 'if ( ImGui::SliderInt( "Workers##Solver", &context->workerCount, 1, B3_MAX_WORKERS ) )'
    require(text, workers, 'GPU worker control')
    text = text.replace(workers, '#if defined(BOTH_SAMPLES)\n'
                        '\t\tconst bool workersChanged = ImGui::SliderInt( "CPU Workers##Solver", &context->workerCount, 1, B3_MAX_WORKERS );\n'
                        '#else\n'
                        '\t\tImGui::TextDisabled( "GPU scheduling is automatic" );\n'
                        '\t\tconst bool workersChanged = false;\n'
                        '#endif\n\t\tif ( workersChanged )')
    recording = 'if ( context->sample->HasSolverControls() && ImGui::CollapsingHeader( "Recording", ImGuiTreeNodeFlags_DefaultOpen ) )'
    require(text, recording, 'unsupported recording control')
    text = text.replace(recording, 'ImGui::TextDisabled( "Recording unavailable with GPU physics" );\n'
                        '\t' + recording.replace('if ( ', 'if ( false && ', 1))
    return text


def inject_joint(text: str) -> str:
    start = text.index("class GearLift : public Sample")
    end = text.index("static int sampleGearLift", start)
    gear = text[start:end]
    replacements = {
        "\t\tAddGroundBox( 20.0f );": "\t\tif ((gpu_sokol_bench_gear_diagnostic() & 2u) == 0u) AddGroundBox( 20.0f );",
        "\t\tCreateMesh( groundId );": "\t\tm_mesh = nullptr;\n\t\tif ((gpu_sokol_bench_gear_diagnostic() & 2u) == 0u) CreateMesh( groundId );",
        "\t\tm_enableMotor = true;": "\t\tm_enableMotor = (gpu_sokol_bench_gear_diagnostic() & 4u) == 0u;",
        "\t\tCreateDebris();": "\t\tif ((gpu_sokol_bench_gear_diagnostic() & 1u) == 0u) CreateDebris();",
        "\t\tb3DestroyMesh( m_mesh );": "\t\tif (m_mesh != nullptr) b3DestroyMesh( m_mesh );",
    }
    for old, new in replacements.items():
        require(gear, old, "Gear Lift diagnostic")
        gear = gear.replace(old, new)
    return '#include "sokol_bench_hooks.h"\n' + text[:start] + gear + text[end:]


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--src", required=True)
    p.add_argument("--dst", required=True)
    p.add_argument("--sample-src")
    p.add_argument("--sample-dst")
    p.add_argument("--continuous-src")
    p.add_argument("--continuous-dst")
    p.add_argument("--joint-src")
    p.add_argument("--joint-dst")
    p.add_argument("--gpu-sidebar", action="store_true")
    args = p.parse_args()
    main_text = inject_main(Path(args.src).read_text())
    Path(args.dst).parent.mkdir(parents=True, exist_ok=True)
    Path(args.dst).write_text(sokol_split_injection.main(main_text))
    if args.joint_src:
        if not args.joint_dst:
            raise SystemExit("--joint-dst required")
        Path(args.joint_dst).write_text(sokol_split_injection.joint(inject_joint(Path(args.joint_src).read_text())))
    if args.continuous_src:
        if not args.continuous_dst:
            raise SystemExit("--continuous-dst required")
        continuous = Path(args.continuous_src).read_text()
        needle = "g_randomSeed = (uint32_t)b3GetTicks();"
        require(continuous, needle, "Mesh Drop seed")
        continuous = '#include "sokol_bench_hooks.h"\n' + continuous.replace(
            needle, "g_randomSeed = gpu_sokol_bench_scene_seed((uint32_t)b3GetTicks());")
        Path(args.continuous_dst).write_text(continuous)
    if args.sample_src:
        if not args.sample_dst:
            raise SystemExit("sokol-bench inject: --sample-dst required with --sample-src")
        sample_text = inject_sample(Path(args.sample_src).read_text())
        if args.gpu_sidebar:
            sample_text = inject_gpu_limitations(sample_text)
            needle = (
                'ImGui::TextColored( HexColor( b3_colorSeaGreen ), "step %d", context->sample->m_stepCount );\n'
                "\tImGui::Separator();"
            )
            require(sample_text, needle, "sample.cpp sidebar")
            sample_text = (
                '#include "box3d/box3d.h"\nextern "C" void gpu_samples_draw_sidebar(b3WorldId worldId);\n'
                + sample_text.replace(
                    needle,
                    'ImGui::TextColored( HexColor( b3_colorSeaGreen ), "step %d", context->sample->m_stepCount );\n'
                    "\tgpu_samples_draw_sidebar( context->sample->m_worldId );\n"
                    "\tImGui::Separator();",
                )
            )
        Path(args.sample_dst).parent.mkdir(parents=True, exist_ok=True)
        Path(args.sample_dst).write_text(sokol_split_injection.sample(sample_text))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
