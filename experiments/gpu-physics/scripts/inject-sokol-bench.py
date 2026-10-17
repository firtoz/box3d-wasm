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


def inject_ui_event_capture(text: str) -> str:
    include = '#include "sokol_glue.h"\n'
    call = 'uiCaptured = HandleEvent( e );'
    require(text, include, "UI event include")
    require(text, call, "UI event capture")
    text = text.replace(include, include + '#include "sokol_ui_input.h"\n').replace(
        call, "uiCaptured = gpu_ui_capture_event( e, HandleEvent( e ) );\n\t\t// Captured presses stay in the UI; releases must clear prior host input.\n\t\tif ( uiCaptured && e->type == SAPP_EVENTTYPE_KEY_UP )\n\t\t{\n\t\t\tSetKeyDown( e->key_code, false );\n\t\t}")
    condition = "\tif ( uiCaptured == false || releaseOrUnfocus )"
    require(text, condition, "captured camera key release")
    return text.replace(condition, "\tif ( uiCaptured == false || releaseOrUnfocus || e->type == SAPP_EVENTTYPE_KEY_UP )")


def inject_main(text: str) -> str:
    text = inject_ui_event_capture(text)
    # Loading must precede bench clocks and any world access. Only the loading
    # callback runs while the worker owns the world; OpenGL remains on UI thread.
    for anchor in ['\tDrawUI( &s_context );', 'static void OnFrame( void )\n{',
                   'static void OnEvent( const sapp_event* e )\n{', 'static void OnCleanup( void )\n{']:
        require(text, anchor, "loading hooks")
    text = text.replace('#include "sokol_glue.h"\n', '#include "sokol_loading.h"\n#include "sokol_glue.h"\n')
    text = text.replace('\tDrawUI( &s_context );', '\tif (gpu_loading_active()) { gpu_loading_draw(); return; }\n\tDrawUI( &s_context );')
    text = text.replace('static void OnFrame( void )\n{', 'static void OnFrame( void )\n{\n    if (gpu_loading_active() ? gpu_loading_poll(b3_nullWorldId) : (s_context.sample && gpu_loading_poll(s_context.sample->m_worldId))) {\n        if (sapp_width() > 0 && sapp_height() > 0) {\n            ResetFrameArena();\n            const sg_swapchain sc = sglue_swapchain();\n            sg_pass pass{};\n            pass.swapchain = sc;\n            pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;\n            pass.action.colors[0].clear_value = {0.035f, 0.045f, 0.065f, 1.0f};\n            sg_begin_pass(&pass);\n            sg_end_pass();\n            gpu_loading_ui_begin();\n            StartUIFrame((float)sapp_frame_duration());\n            RenderUI(&sc);\n            gpu_loading_ui_end();\n            sg_commit();\n        }\n        std::this_thread::sleep_for(std::chrono::milliseconds(16));\n        return;\n    }\n    if (gpu_loading_close_pending()) { sapp_quit(); return; }\n')
    text = text.replace('static void OnEvent( const sapp_event* e )\n{', 'static void OnEvent( const sapp_event* e )\n{\n    if (gpu_loading_active()) { if (e->type == SAPP_EVENTTYPE_QUIT_REQUESTED) { gpu_loading_request_close(); sapp_cancel_quit(); } else { HandleEvent(e); } return; }')
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
    text = text.replace("static void OnEvent( const sapp_event* e )\n{", "static void OnEvent( const sapp_event* e )\n{\n"
        "\t// Benchmarks use scripted inputs only; ordinary viewers retain all controls.\n"
        "\tif ( gpu_sokol_bench_active() && (e->type == SAPP_EVENTTYPE_KEY_DOWN || e->type == SAPP_EVENTTYPE_KEY_UP ||\n"
        "\t    e->type == SAPP_EVENTTYPE_CHAR || e->type == SAPP_EVENTTYPE_MOUSE_DOWN || e->type == SAPP_EVENTTYPE_MOUSE_UP ||\n"
        "\t    e->type == SAPP_EVENTTYPE_MOUSE_MOVE || e->type == SAPP_EVENTTYPE_MOUSE_SCROLL) ) return;\n")
    # Explicit logical window size for matched-renderer benchmarks.
    text = text.replace("desc.width = 1920;", 'desc.width = getenv("GPU_BENCH_WIDTH") ? atoi(getenv("GPU_BENCH_WIDTH")) : 1920;')
    text = text.replace("desc.height = 1080;", 'desc.height = getenv("GPU_BENCH_HEIGHT") ? atoi(getenv("GPU_BENCH_HEIGHT")) : 1080;')
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
        "static int s_hideUI = 0;\n"
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
        "\tif ( s_hideUI ) { s_context.showUI = false; s_context.showControls = false; }\n"
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
        "\tgpu_sokol_bench_shoot( s_context.sample, s_frame, W, H );\n"
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
        '\t\telse if ( strcmp( argv[i], "--hide-ui" ) == 0 )\n'
        "\t\t{\n"
        "\t\t\ts_hideUI = 1;\n"
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
    text = text.replace('static void OnInit( void )\n{', 'static void OnInit( void )\n{\n    gpu_loading_begin_startup();')
    text = text.replace('sg_commit();', 'sg_commit(); gpu_loading_presented();')
    return text


def inject_sample(text: str) -> str:
    select = "void SelectSample( SampleContext* context, int selection, bool restart )\n{"
    require(text, select, "asynchronous scene selection")
    text = '#include "sokol_loading.h"\n' + text.replace(select, select + r'''
#if defined(GPU_PHYSICS_SAMPLES)
    static thread_local bool constructing = false;
    if (!constructing) {
        if (selection < 0 || selection >= g_sampleCount || gpu_loading_active()) return;
        // Retire cached renderer meshes with the host GL context current, before
        // the worker destroys their owning physics world. Initial setup has none.
        if (context->sample) ResetAdapterPool();
        // Window-system mouse capture stays on the host thread.
        if (context->camera.m_thirdPerson) {
            sapp_lock_mouse(false);
            context->camera.m_thirdPerson = false;
        }
        gpu_loading_create_scene([context, selection, restart] {
            constructing = true;
            SelectSample(context, selection, restart);
            constructing = false;
            return context->sample->m_worldId;
        });
        return;
    }
#endif
''')

    require(text, '\tResetAdapterPool();', 'host-thread renderer cleanup')
    text = text.replace('\tResetAdapterPool();',
        '\tif (!gpu_loading_on_scene_worker()) ResetAdapterPool();')

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


def inject_hidden_hud_layout(text: str) -> str:
    # The hidden HUD owns a second row while PAUSED is visible.
    original = "\t\tm_textLine = (int)( 3.0f * fontSize );"
    replacement = "\t\t// Reserve the PAUSED row only while that HUD label is present.\n\t\tm_textLine = (int)( ( m_context->pause ? 4.5f : 3.0f ) * fontSize );"
    require(text, original, "hidden sample text row")
    assert text.count(original) == 1, "hidden sample text row drift"
    return text.replace(original, replacement)


def inject_profile_column_width(text: str) -> str:
    # Native CPU labels include an arrow/leaf spacer and per-row indentation.
    # Measure the complete tree, even when rows are collapsed or currently zero.
    original = "\t\t\tImGui::TableSetupColumn( \"section\", ImGuiTableColumnFlags_WidthFixed, 8.0f * fontSize );"
    replacement = "\t\t\t// Include the same indentation and arrow spacing used to draw every row.\n\t\t\t// Measure collapsed/zero rows too, so expanding a subtree keeps labels visible.\n\t\t\tfloat sectionWidth = ImGui::CalcTextSize( \"section\" ).x;\n\t\t\tconst float labelSpacing = ImGui::GetTreeNodeToLabelSpacing();\n\t\t\tfor ( const RowDef& row : rows )\n\t\t\t{\n\t\t\t\tfloat width = row.indent * fontSize + labelSpacing + ImGui::CalcTextSize( row.name ).x;\n\t\t\t\tsectionWidth = b3MaxFloat( sectionWidth, width );\n\t\t\t}\n\t\t\tImGui::TableSetupColumn( \"section\", ImGuiTableColumnFlags_WidthFixed, sectionWidth );"
    require(text, original, "CPU profile section width")
    assert text.count(original) == 1, "CPU profile section width drift"
    return text.replace(original, replacement)


def inject_native_diagnostics(text: str) -> str:
    # Replace only GPU/combined metrics tabs in the generated copy. CPU keeps
    # its original profiler; partial GPU phases must not feed CPU flame bars.
    for tab, call in [('Profile', 'gpu_samples_draw_native_profile'),
                      ('Counters', 'gpu_samples_draw_native_counters'),
                      ('Frame Time', 'gpu_samples_draw_native_profile')]:
        anchor = 'if ( ImGui::BeginTabItem( "' + tab + '" ) )'
        require(text, anchor, 'native diagnostic ' + tab)
        opening = text.index('{', text.index(anchor) + len(anchor))
        closing = text.index('ImGui::EndTabItem();', opening)
        original = text[opening + 1:closing]
        text = text[:opening + 1] + ('\n#if defined(GPU_PHYSICS_SAMPLES) || defined(BOTH_SAMPLES)\n'
            '\t\t' + call + '(m_worldId);\n#else\n' + original + '\n#endif\n\t\t') + text[closing:]
    return ('#include "box3d/box3d.h"\n'
            'extern "C" void gpu_samples_draw_native_profile(b3WorldId);\n'
            'extern "C" void gpu_samples_draw_native_counters(b3WorldId);\n' + text)


def inject_info_text_wrap(text: str) -> str:
    # Scope wrapping to the Info window, including sample text and the native
    # diagnostics/unsupported-feature explanation inserted below. Keep sliders,
    # other windows and all scene/physics code unchanged.
    start = "static void DrawInfoPanel( SampleContext* context )\n{"
    end = "\n// Minimal in-world HUD"
    require(text, start, "Info panel start")
    require(text, end, "Info panel end")
    first = text.index(start)
    last = text.index(end, first)
    panel = text[first:last]
    title = '\tImGui::TextColored( HexColor( b3_colorGoldenRod ), "%s", entry.Name );'
    close = "\n\tImGui::End();"
    require(panel, title, "Info panel first text")
    require(panel, close, "Info panel close")
    panel = panel.replace(title, "\tImGui::PushTextWrapPos( 0.0f );\n\n" + title)
    panel = panel.replace(close, "\n\tImGui::PopTextWrapPos();" + close)
    return text[:first] + panel + text[last:]


def inject_gpu_limitations(text: str) -> str:
    warm = 'ImGui::Checkbox( "Warm Starting##Solver", &context->enableWarmStarting );'
    require(text, warm, 'warm-start control')
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
    # Wheel's steering slider must update steering, preserving the independently
    # configured suspension damping. Keep the upstream source untouched.
    steering = ('\t\t\tif ( ImGui::SliderFloat( "Damping##Steering", &m_steeringDampingRatio, 0.0f, 2.0f, "%.1f" ) )\n'
                '\t\t\t{\n'
                '\t\t\t\tb3WheelJoint_SetSuspensionDampingRatio( m_jointId, m_suspensionDampingRatio );')
    require(text, steering, "Wheel steering damping")
    text = text.replace(steering, steering.replace(
        'b3WheelJoint_SetSuspensionDampingRatio( m_jointId, m_suspensionDampingRatio )',
        'b3WheelJoint_SetSteeringDampingRatio( m_jointId, m_steeringDampingRatio )'))
    torque = '\t\tImGui::SliderFloat( "Max Torque", &m_maxSpinTorque, 0.0f, 100.0f, "%.0f" );\n'
    require(text, torque, "Driving rear motor torque")
    text = text.replace(torque,
        torque.replace('\t\tImGui::', '\t\tif ( ImGui::').replace(' );\n', ' ) )\n') +
        '\t\t{\n'
        '\t\t\tb3WheelJoint_SetMaxSpinTorque( m_rearLeftId, m_maxSpinTorque );\n'
        '\t\t\tb3WheelJoint_SetMaxSpinTorque( m_rearRightId, m_maxSpinTorque );\n'
        '\t\t}\n')
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
    return '#include "sokol_loading.h"\n#include "sokol_bench_hooks.h"\n' + (text[:start] + gear + text[end:]).replace('sapp_lock_mouse(', 'gpu_loading_lock_mouse(')


def inject_replay(text: str) -> str:
    # The player still receives the full path. Only the modal's display name
    # changes; accept either separator, including paths with both kinds.
    anchor = "\t\tconst char* slash = strrchr( m_path, '\\\\' );"
    require(text, anchor, "Replay file name")
    text = text.replace(anchor,
        "\t\tconst char* slash = strrchr( m_path, '/' );\n"
        "\t\tconst char* backslash = strrchr( m_path, '\\\\' );\n"
        "\t\tif ( backslash != nullptr && ( slash == nullptr || backslash > slash ) )\n"
        "\t\t{\n\t\t\tslash = backslash;\n\t\t}")
    gpu = "#if defined(GPU_PHYSICS_SAMPLES) || defined(BOTH_SAMPLES)\n"
    message = "GPU replay is unavailable. Use the CPU viewer."
    status = '\t\tsnprintf( m_status, sizeof( m_status ), "%s", kGpuReplayUnavailable );\n'
    constructor = "\t\telse\n\t\t{\n\t\t\tCreatePlayer();\n\t\t}\n\t}\n\n\t~ReplayViewer()"
    require(text, constructor, "Replay initial unsupported status")
    text = text.replace(constructor, constructor.replace(
        "\n\t}\n\n\t~ReplayViewer()", "\n" + gpu + status + "#endif\n\t}\n\n\t~ReplayViewer()"))
    create = "\tvoid CreatePlayer()\n\t{\n\t\tClosePlayer();\n"
    require(text, create, "Replay unsupported player guard")
    text = text.replace(create, create + "\n" + gpu +
        "\t\tm_info = b3RecPlayerInfo{};\n\t\tm_frameAccumulator = 0.0f;\n" +
        status + "\t\treturn;\n#endif\n")
    filename = '\t\tImGui::TextUnformatted( slash != nullptr ? slash + 1 : m_path );\n\t\tImGui::Separator();\n'
    require(text, filename, "Replay unsupported modal")
    text = text.replace(filename, filename + "\n" + gpu +
        '\t\tImGui::TextWrapped( "%s", kGpuReplayUnavailable );\n'
        '\t\tif ( ImGui::Button( "Close" ) )\n\t\t{\n\t\t\tImGui::CloseCurrentPopup();\n\t\t}\n'
        '\t\tImGui::EndPopup();\n\t\treturn;\n#endif\n')
    return gpu + 'static const char* kGpuReplayUnavailable = "' + message + '";\n#endif\n' + text


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--src", required=True)
    p.add_argument("--dst", required=True)
    p.add_argument("--sample-src")
    p.add_argument("--sample-dst")
    p.add_argument("--continuous-src")
    p.add_argument("--continuous-dst")
    p.add_argument("--character-src")
    p.add_argument("--character-dst")
    p.add_argument("--joint-src")
    p.add_argument("--joint-dst")
    p.add_argument("--replay-src")
    p.add_argument("--replay-dst")
    p.add_argument("--gpu-sidebar", action="store_true")
    args = p.parse_args()
    main_text = inject_main(Path(args.src).read_text())
    Path(args.dst).parent.mkdir(parents=True, exist_ok=True)
    Path(args.dst).write_text(sokol_split_injection.main(main_text))
    if args.replay_src:
        if not args.replay_dst:
            raise SystemExit("--replay-dst required")
        Path(args.replay_dst).write_text(inject_replay(Path(args.replay_src).read_text()))
    if args.character_src:
        if not args.character_dst: raise SystemExit("--character-dst required")
        character = Path(args.character_src).read_text()
        assert "sapp_lock_mouse(" in character, "character mouse hook drift"
        Path(args.character_dst).write_text('#include "sokol_loading.h"\n' + character.replace("sapp_lock_mouse(", "gpu_loading_lock_mouse("))
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
        sample_text = inject_hidden_hud_layout(inject_profile_column_width(inject_info_text_wrap(inject_sample(Path(args.sample_src).read_text()))))
        if args.gpu_sidebar:
            sample_text = inject_native_diagnostics(inject_gpu_limitations(sample_text))
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
