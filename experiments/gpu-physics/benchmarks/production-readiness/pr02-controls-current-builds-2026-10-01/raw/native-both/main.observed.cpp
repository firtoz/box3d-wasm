
#if defined(BOTH_SAMPLES)
#include "../c_abi/both_view.h"
#include "sokol_split_stream.h"
#include "sokol_split_input.h"
#include "sokol_split_text.h"
static int pointerPane = -1;
static float splitMouseX = -1, splitMouseY = -1;
static bool splitMouseInside = false;
#endif
#if defined(BOTH_SAMPLES)
#include "imgui.h"
#endif
// SPDX-FileCopyrightText: 2025 Erin Catto
// SPDX-License-Identifier: MIT

#if defined( _MSC_VER )
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define _CRTDBG_MAP_ALLOC
// stdlib before crtdbg so the malloc->_malloc_dbg remap lands after the real prototype
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "gfx/debug_adapter.h"
#include "gfx/keycodes.h"
#include "gfx/renderer.h"
#include "host/gui.h"
#include "sample.h"
#include "sokol_app.h"
#include "sokol_loading.h"
#include "sokol_glue.h"
#include "sokol_bench_hooks.h"
#if defined(BOTH_SAMPLES)
static const char* kGpuSokolBenchMode = "both";
#elif defined(GPU_PHYSICS_SAMPLES)
static const char* kGpuSokolBenchMode = "gpu";
#else
static const char* kGpuSokolBenchMode = "cpu";
#endif

#include "box3d/box3d.h"
#include "box3d/math_functions.h"
#include "gfx/draw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#else
#define FrameMark
#endif

#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// clang-format off
#include <windows.h>
#include <timeapi.h> // timeBeginPeriod for fine sleep granularity
// clang-format on
#pragma comment( lib, "winmm.lib" ) // MSVC auto-link
#endif

static SampleContext s_context;
#include "controls_observer.inc"
static int s_frame = 0;
static int s_frameLimit = -1;
static int s_sampleOverride = -1;
static char s_sampleName[128];
static char s_benchJson[1024];
static int s_benchWarmup = 20;
static int s_benchTimed = 60;
static int s_benchUnpaced = 1;
static int s_benchCompleted = 0;
static int s_benchPauseScript = 0;
static int s_benchSleep = 1;
static int s_hideUI = 0;
static int s_benchWorkers = 0;
static char s_switchName[128];
static int s_switchAfter = -1;
static int s_healthScan = 0;
static int s_exitCode = 0;

static int CompareSamples( const void* a, const void* b )
{
	SampleEntry* entryA = (SampleEntry*)a;
	SampleEntry* entryB = (SampleEntry*)b;

	int result = strcmp( entryA->Category, entryB->Category );
	if ( result == 0 )
	{
		result = strcmp( entryA->Name, entryB->Name );
	}

	return result;
}

static void SortSamples()
{
	// Sorting reorders the table, so recover the replay viewer's slot by identity.
	SampleCreateFcn* replayFcn = ( g_replayIndex >= 0 ) ? g_sampleEntries[g_replayIndex].CreateFcn : nullptr;
	qsort( g_sampleEntries, g_sampleCount, sizeof( SampleEntry ), CompareSamples );
	if ( replayFcn != nullptr )
	{
		g_replayIndex = -1;
		for ( int i = 0; i < g_sampleCount; ++i )
		{
			if ( g_sampleEntries[i].CreateFcn == replayFcn )
			{
				g_replayIndex = i;
				break;
			}
		}
	}
}

// Single host UI callback fired from inside StartUIFrame: menu bar, panels, and
// the active sample's drawer.
static void OnDrawUI( void )
{
	if (gpu_loading_active()) { gpu_loading_draw(); return; }
#if defined(BOTH_SAMPLES)
    ImGui::SetNextWindowPos(ImVec2(8.0f, (float)sapp_height() - 8.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::Begin("Comparison view", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
    bool split = both_split_enabled();
    if (ImGui::Checkbox("Split: CPU left / GPU right", &split)) {
        s_context.sample->MouseUp({0, 0}, 0);
        both_set_split(split);
        pointerPane = -1;
    }
    ImGui::End();
#endif
    // UI layout uses the whole window; projection/picking keeps the pane size.
#if defined(BOTH_SAMPLES)
    int paneWidth = s_context.camera.m_width;
    s_context.camera.m_width = sapp_width();
    if (both_split_enabled()) {
        auto* fg = ImGui::GetForegroundDrawList();
        fg->AddLine(ImVec2(sapp_width()/2, 22), ImVec2(sapp_width()/2, sapp_height()), IM_COL32(160,160,170,255));
        fg->AddText(ImVec2(sapp_width()/4 - 12, 28), IM_COL32(255,190,70,255), "CPU");
        fg->AddText(ImVec2(3*sapp_width()/4 - 12, 28), IM_COL32(130,180,255,255), "GPU");
        if (splitMouseInside && (!ImGui::GetIO().WantCaptureMouse || pointerPane >= 0)) {
            const int width = sapp_width() / 2;
            const int sourcePane = pointerPane >= 0 ? pointerPane : (splitMouseX >= width ? 1 : 0);
            const int peer = 1 - sourcePane;
            const float x = split_pointer_x(splitMouseX, sapp_width(), sourcePane) + peer * width;
            const float y = splitMouseY;
            const ImU32 color = peer ? IM_COL32(130,180,255,255) : IM_COL32(255,190,70,255);
            fg->PushClipRect(ImVec2(peer * width, 0), ImVec2((peer + 1) * width, sapp_height()), true);
            fg->AddLine(ImVec2(x-8,y), ImVec2(x+8,y), IM_COL32(0,0,0,220), 4);
            fg->AddLine(ImVec2(x,y-8), ImVec2(x,y+8), IM_COL32(0,0,0,220), 4);
            fg->AddLine(ImVec2(x-7,y), ImVec2(x+7,y), color, 2);
            fg->AddLine(ImVec2(x,y-7), ImVec2(x,y+7), color, 2);
            if (pointerPane >= 0) fg->AddCircle(ImVec2(x,y), 11, color, 16, 2);
            fg->PopClipRect();
        }
    }
#endif
    DrawUI( &s_context );
#if defined(BOTH_SAMPLES)
    s_context.camera.m_width = paneWidth;
#endif
}

static void OnInit( void )
{
    gpu_loading_begin_startup();
#ifdef TRACY_ENABLE
	tracy::StartupProfiler();
#endif

#if defined( _WIN32 )
	timeBeginPeriod( 1 );
#endif
	const sg_environment env = sglue_environment();
	InitRenderer( &env );
	InitUI( &env, OnDrawUI );
	InitAdapter();

	constexpr float DEG = 3.14159265358979323846f / 180.0f;
	s_context.camera.SetFov( 50.0f * DEG );
	s_context.camera.SetClip( 0.1f, Camera::kViewDistance );

	s_context.Load();

	// First run with no settings opens with the controls window up.
	s_context.showControls = s_context.newUser;

	int cores = (int)std::thread::hardware_concurrency();
	s_context.workerCount = b3ClampInt( cores / 2, 1, 8 );
	s_context.windowWidth = sapp_width();
	s_context.windowHeight = sapp_height();

	SortSamples();

	// A first run with no settings opens straight into the replay viewer.
	int index = s_context.sampleIndex;
	if ( s_context.newUser && g_replayIndex >= 0 )
	{
		index = g_replayIndex;
	}

	// --sample N selects a registered sample by sorted index, overriding the
	// persisted one. Lets a headless --frames run target a specific sample.
	if ( s_sampleOverride >= 0 && s_sampleOverride < g_sampleCount )
	{
		index = s_sampleOverride;
	}

	if ( s_benchJson[0] && s_sampleName[0] == 0 )
	{
		fprintf( stderr, "sokol-bench: --sample-name is required\n" );
		s_exitCode = 1;
		sapp_quit();
		return;
	}
	if ( s_sampleName[0] )
	{
		int named = gpu_sokol_bench_sample_index_by_name( s_sampleName );
		if ( named < 0 )
		{
			s_exitCode = 1;
			sapp_quit();
			return;
		}
		index = named;
	}
	if ( s_benchJson[0] )
	{
		int workers = s_benchWorkers > 0 ? s_benchWorkers : s_context.workerCount;
		gpu_sokol_bench_configure( s_benchJson, s_benchWarmup, s_benchTimed, s_benchUnpaced != 0,
			s_benchCompleted != 0, s_benchPauseScript != 0, workers, s_benchSleep );
		gpu_sokol_bench_set_health_scan( s_healthScan != 0 );
		s_context.enableSleep = s_benchSleep != 0;
		s_context.workerCount = workers;
		s_context.hertz = 60.0f;
		s_context.subStepCount = 4;
		if ( s_frameLimit < 0 )
		{
			s_frameLimit = s_benchWarmup + s_benchTimed + ( s_benchPauseScript ? 12 : 0 );
		}
	}
	if ( s_hideUI ) { s_context.showUI = false; s_context.showControls = false; }
	SelectSample( &s_context, index, false );
}

static void OnEvent( const sapp_event* e )
{
	// Benchmarks use scripted inputs only; ordinary viewers retain all controls.
	if ( gpu_sokol_bench_active() && (e->type == SAPP_EVENTTYPE_KEY_DOWN || e->type == SAPP_EVENTTYPE_KEY_UP ||
	    e->type == SAPP_EVENTTYPE_CHAR || e->type == SAPP_EVENTTYPE_MOUSE_DOWN || e->type == SAPP_EVENTTYPE_MOUSE_UP ||
	    e->type == SAPP_EVENTTYPE_MOUSE_MOVE || e->type == SAPP_EVENTTYPE_MOUSE_SCROLL) ) return;

    if (gpu_loading_active()) { if (e->type == SAPP_EVENTTYPE_QUIT_REQUESTED) { gpu_loading_request_close(); sapp_cancel_quit(); } else { HandleEvent(e); } return; }
	//if ( e->type == SAPP_EVENTTYPE_KEY_DOWN || e->type == SAPP_EVENTTYPE_CHAR )
	//{
	//	volatile int dummy = 0;
	//	dummy = 32;
	//}

	bool uiCaptured = false;
	if (s_context.camera.m_thirdPerson == false)
	{
		uiCaptured = HandleEvent( e );
	}

	// The camera must always see button releases and focus loss, even when the UI
	// captures the event. Otherwise a release over an ImGui panel never clears the
	// drag flag and the camera keeps orbiting.
	const bool releaseOrUnfocus = e->type == SAPP_EVENTTYPE_MOUSE_UP || e->type == SAPP_EVENTTYPE_UNFOCUSED;
	if ( uiCaptured == false || releaseOrUnfocus )
	{
		s_context.camera.OnEvent( e );
	}

#if defined(BOTH_SAMPLES)
    if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE || e->type == SAPP_EVENTTYPE_MOUSE_DOWN || e->type == SAPP_EVENTTYPE_MOUSE_UP || e->type == SAPP_EVENTTYPE_MOUSE_ENTER) {
        splitMouseX = e->mouse_x; splitMouseY = e->mouse_y; splitMouseInside = true;
    }
    if (e->type == SAPP_EVENTTYPE_MOUSE_LEAVE || e->type == SAPP_EVENTTYPE_UNFOCUSED) splitMouseInside = false;
    if (releaseOrUnfocus) {
        both_pointer_up();
        pointerPane = -1;
    }
#endif
    if (uiCaptured) return;
#if defined(BOTH_SAMPLES)
    sapp_event localEvent = *e;
    if (both_split_enabled() && sapp_width() >= 2) {
        int paneWidth = sapp_width() / 2;
        if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN && pointerPane < 0) pointerPane = e->mouse_x >= paneWidth ? 1 : 0;
        localEvent.mouse_x = split_pointer_x(e->mouse_x, sapp_width(), pointerPane);
        e = &localEvent;
    }
#endif

	// Keep keyboard mods only. sokol packs the held mouse button into modifiers
	// (SAPP_MODIFIER_LMB == 0x100), which would defeat the sample's modifiers == 0 checks.
	const int mods = e->modifiers & ( SAPP_MODIFIER_SHIFT | SAPP_MODIFIER_CTRL | SAPP_MODIFIER_ALT | SAPP_MODIFIER_SUPER );

	switch ( e->type )
	{
		case SAPP_EVENTTYPE_KEY_DOWN:
			SetKeyDown( e->key_code, true );

			// Global shortcuts on first press; repeats are ignored. The sokol
			// analog of Box2D's KeyCallback.
			if ( e->key_repeat == false )
			{
				switch ( e->key_code )
				{
					case SAPP_KEYCODE_TAB:
						s_context.showUI = !s_context.showUI;
						break;

					case SAPP_KEYCODE_ESCAPE:
						// Layered cancel. An open picker is an ImGui popup that
						// already swallowed this. So peel the controls window, then
						// the selection. Quit lives on Ctrl+Q now.
						if ( s_context.showControls )
						{
							s_context.showControls = false;
						}
						else
						{
							ClearSelection();
						}
						break;

					case KEY_Q:
						if ( mods & SAPP_MODIFIER_CTRL )
						{
							sapp_request_quit();
							break;
						}
						s_context.sample->Keyboard( e->key_code, ACTION_PRESS, mods );
						break;

					case KEY_O:
						if ( mods & SAPP_MODIFIER_CTRL )
						{
							// Ctrl+O opens the fuzzy picker. Force the UI visible so it shows.
							s_context.showUI = true;
							s_context.openSamplePicker = true;
							break;
						}
						s_context.sample->Keyboard( e->key_code, ACTION_PRESS, mods );
						break;

					// Step forward. Backward is , and only a sample that can rewind
					// handles it, so that key falls through to the sample.
					case KEY_PERIOD:
						s_context.singleStep += ( mods & SAPP_MODIFIER_SHIFT ) ? 5 : 1;
						break;

					case SAPP_KEYCODE_P:
					case SAPP_KEYCODE_PAUSE:
						s_context.pause = !s_context.pause;
						break;

					case SAPP_KEYCODE_M:
						s_context.showMetrics = !s_context.showMetrics;
						break;

					case SAPP_KEYCODE_R:
						SelectSample( &s_context, s_context.sampleIndex, true );
						break;

					case SAPP_KEYCODE_LEFT_BRACKET:
						SelectSample( &s_context, b3MaxInt( 0, s_context.sampleIndex - 1 ), false );
						break;

					case SAPP_KEYCODE_RIGHT_BRACKET:
						SelectSample( &s_context, b3MinInt( g_sampleCount - 1, s_context.sampleIndex + 1 ), false );
						break;

					case SAPP_KEYCODE_F:
					{
						// Frame the selection, or let the sample frame its whole scene when nothing is
						// selected. A non-body selection such as a recorded query supplies its own bounds and
						// takes priority over the hovered body. The replay viewer's scene lives in a
						// player-owned world, not the base world, so the whole-scene case routes through
						// FocusHome.
						Camera& cam = s_context.camera;
						float aspect = cam.m_height > 0 ? (float)cam.m_width / (float)cam.m_height : 1.0f;
						b3AABB bounds;
						if ( s_context.sample->FocusBounds( &bounds ) )
						{
							cam.Frame( bounds, aspect, 1.5f );
						}
						else
						{
							b3BodyId bodyId = s_context.sample->FocusBody();
							if ( B3_IS_NON_NULL( bodyId ) )
							{
								cam.Frame( b3Body_ComputeAABB( bodyId ), aspect, 1.5f );
							}
							else
							{
								s_context.sample->FocusHome();
							}
						}
					}
					break;

					default:
						s_context.sample->Keyboard( e->key_code, ACTION_PRESS, mods );
						break;
				}
			}
			break;

		case SAPP_EVENTTYPE_KEY_UP:
			SetKeyDown( e->key_code, false );
			break;

		case SAPP_EVENTTYPE_CHAR:
			// ? toggles the controls window. Read the typed character so it works
			// on any layout and a text field can still type a literal ?, since the
			// picker captures the event before it reaches here.
			if ( e->char_code == '?' && e->key_repeat == false )
			{
				s_context.showControls = !s_context.showControls;
			}
			break;

		case SAPP_EVENTTYPE_MOUSE_DOWN:
			s_context.mouseX = e->mouse_x;
			s_context.mouseY = e->mouse_y;
			s_context.sample->MouseDown( { e->mouse_x, e->mouse_y }, e->mouse_button, mods );
			break;

		case SAPP_EVENTTYPE_MOUSE_UP:
			s_context.sample->MouseUp( { e->mouse_x, e->mouse_y }, e->mouse_button );
			break;

		case SAPP_EVENTTYPE_MOUSE_MOVE:
			s_context.mouseX = e->mouse_x;
			s_context.mouseY = e->mouse_y;
			s_context.mouseDX = e->mouse_dx;
			s_context.mouseDY = e->mouse_dy;
			s_context.sample->MouseMove( { e->mouse_x, e->mouse_y } );
			break;

		case SAPP_EVENTTYPE_RESIZED:
		{
			int w = sapp_width();
			int h = sapp_height();
			s_context.minimized = ( w == 0 || h == 0 );
			if ( s_context.minimized == false )
			{
				s_context.windowWidth = w;
				s_context.windowHeight = h;
			}
		}
		break;

		default:
			break;
	}
}

// This limits the render rate to the simulation rate. Physics debugging
// ergonomics require render frames and simulation frames to be one to one.
// I don't recommend this setup for games, where a decoupled render
// and simulation rate is desirable.
static void LimitFrameRate( uint64_t frameStart )
{
	// By limiting to the simulation hertz, this allows me to run the simulation
	// at 240Hz and render at that rate when using a 240Hz monitor. It also allows
	// me to run a 10Hz simulation in at the wall clock rate.
	float hertz = b3ClampFloat(s_context.hertz, 5.0f, 1000.0f);
	const float targetMs = 1000.0f / hertz;
	const float spinMs = 2.0f;

	int sleepMs = (int)( targetMs - spinMs - b3GetMilliseconds( frameStart ) );
	if ( sleepMs > 0 )
	{
		b3Sleep( sleepMs );
	}

	while ( b3GetMilliseconds( frameStart ) < targetMs )
	{
		b3Yield();
	}
}

static void OnFrame( void )
{
    if (gpu_loading_active() ? gpu_loading_poll(b3_nullWorldId) : (s_context.sample && gpu_loading_poll(s_context.sample->m_worldId))) {
        if (sapp_width() > 0 && sapp_height() > 0) {
            ResetFrameArena();
            const sg_swapchain sc = sglue_swapchain();
            sg_pass pass{};
            pass.swapchain = sc;
            pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
            pass.action.colors[0].clear_value = {0.035f, 0.045f, 0.065f, 1.0f};
            sg_begin_pass(&pass);
            sg_end_pass();
            StartUIFrame((float)sapp_frame_duration());
            RenderUI(&sc);
            sg_commit(); gpu_loading_presented();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        return;
    }
    if (gpu_loading_close_pending()) { sapp_quit(); return; }

	if ( s_frameLimit >= 0 && s_frame >= s_frameLimit )
	{
		sapp_quit();
		return;
	}

	const uint64_t frameStart = b3GetTicks();
	gpu_sokol_bench_begin_frame();
	gpu_sokol_bench_apply_script( &s_context.pause, &s_context.singleStep, s_frame );
	if ( s_switchName[0] && s_switchAfter >= 0 && s_frame == s_switchAfter )
	{
		int named = gpu_sokol_bench_sample_index_by_name( s_switchName );
		if ( named >= 0 )
		{
			SelectSample( &s_context, named, false );
			gpu_sokol_bench_note_switch();
			s_switchAfter = -1;
			return;
		}
	}

	// Nothing to draw while minimized. sapp reports a 0x0 framebuffer then, which
	// would drive the swapchain and every render target to zero size. Pace the
	// loop so it doesn't spin and bail.
	if ( s_context.minimized )
	{
		if ( s_frameLimit < 0 )
		{
			LimitFrameRate( frameStart );
		}
		return;
	}

	// Handle a deferred Replay > Open request before any GPU work this frame.
	// The native picker blocks on a nested run loop. Running it here (no pass
	// open, no buffers updated, no ImGui frame begun) keeps it from re-entering
	// a half-built frame the way it did when invoked from inside the UI callback.
	if ( s_context.openReplayPicker )
	{
		s_context.openReplayPicker = false;
		OpenReplayFileDialog( &s_context );
	}

	const float dt = (float)sapp_frame_duration();
	const int W = sapp_width();
	const int H = sapp_height();

	Camera& camera = s_context.camera;
#if defined(BOTH_SAMPLES)
    const bool splitActive = both_split_enabled() && W >= 2;
    camera.Update(dt, splitActive ? W / 2 : W, H);
    both_begin_frame();
    if (both_split_enabled()) {
        PickRay ray = camera.BuildPickRay(s_context.mouseX, s_context.mouseY);
        both_pointer_move(ray.origin, ray.translation);
    }
#else
    camera.Update(dt, W, H);
#endif

	// Push the current length scale and Z-up choice. The replay player sets length
	// units from its header on load and restores them on close, so querying every
	// frame tracks load/unload without extra wiring. Live samples sit at 1 unit per
	// meter, leaving the transform identity.
	camera.SetRenderTransform( b3GetLengthUnitsPerMeter(), s_context.viewZUp );
	camera.SetDrawDistance( s_context.drawDistance );

	// Sync the draw origin to the camera eye once per frame, before any drawing. This must hold even
	// for samples that drive their own Step without calling Sample::Step. Render re-syncs after Step
	// because a third person follow moves the eye while stepping. The draw origin is in simulation
	// space; the view folds in the scale and up axis.
	SetDrawOrigin( camera.DrawOrigin() );

	ResetFrameArena();

	// Apply the per-frame draw state the UI owns, then advance the sample. Step
	// queues the HUD text; Render fills the instance and overlay arenas via the
	// b3DebugDraw adapter and the sample's own Draw* calls.
	SetTransparentDynamic( s_context.transparentDynamic );
	SetTransparentKinematic( s_context.transparentKinematic );
	s_context.sample->ResetText();

	gpu_sokol_bench_shoot( s_context.sample, s_frame, W, H );
	gpu_sokol_bench_village_drop( s_context.sample->m_worldId, s_sampleName, s_frame );
	gpu_sokol_bench_note_step_request( s_context.sample ? s_context.sample->m_stepCount + 1 : 0 );
	s_context.sample->Step();
	controls_observe(s_context, s_frame);
	s_context.sample->Render();

	FrameInput fi{};
	fi.view = camera.View();
	fi.viewInv = camera.ViewInverse();
	fi.proj = camera.Proj();
	fi.projInv = camera.ProjInverse();
	fi.cameraPosition = camera.Position();
	// Read back the origin Render actually shifted to. A follow-cam sample can
	// move it during Step/Render, so reuse the live value rather than the eye.
	b3Pos drawOrigin = GetDrawOrigin();
	fi.drawOrigin = b3ToVec3( drawOrigin );
	// Wrap the origin to the grid period in double, before it narrows to float.
	// A float can't resolve a 1 m cell at 1e7 m, so feeding the raw origin to
	// the grid would shatter the lines. The pattern repeats every 10 cells, so
	// the wrapped offset draws identical lines at any distance.
	double gridPeriod = 10.0 * BOX3D_GROUND_GRID_CELL_SIZE;
	fi.gridWrap.x = (float)fmod( (double)drawOrigin.x, gridPeriod );
	fi.gridWrap.y = (float)fmod( (double)drawOrigin.z, gridPeriod );
	fi.time = (float)sapp_frame_count() / 60.0f;
	fi.debugMode = s_context.debugView;
	fi.disableShadows = !s_context.enableShadows;
	fi.disableAmbientOcclusion = !s_context.enableGtao;
	fi.zUp = s_context.viewZUp;

	const sg_swapchain sc = sglue_swapchain();
	gpu_sokol_bench_mark( "render" );
#if defined(BOTH_SAMPLES)
    if (splitActive) {
        split_stream_view(0);
        split_render_viewport(0, W / 2);
        RenderFrame(&sc, &fi);
        std::vector<SplitText> labels;
        split_capture_text(labels, 0);
        ResetFrameArena();
        split_stream_view(1);
        both_draw_second();
        s_context.sample->Render();
        split_render_viewport(W / 2, W / 2);
        RenderFrame(&sc, &fi);
        split_capture_text(labels, W / 2);
        split_publish_text(labels);
        split_stream_view(0);
        split_render_viewport(0, 0);
    } else {
        split_stream_view(0);
        split_render_viewport(0, 0);
        RenderFrame(&sc, &fi);
    }
#else
    RenderFrame(&sc, &fi);
#endif
	gpu_sokol_bench_mark( "ui" );

	// StartUIFrame runs after RenderFrame: it drains the text arena with the
	// camera state RenderFrame just latched and runs the UI draw callback.
	StartUIFrame( dt );

	RenderUI( &sc );
	gpu_sokol_bench_mark( "commit" );
	sg_commit(); gpu_loading_presented();
	gpu_sokol_bench_mark( "limiter" );
	gpu_sokol_bench_end_frame();
	++s_frame;

	// For the Tracy profiler
	FrameMark;

	if ( gpu_sokol_bench_active() )
	{
		if ( gpu_sokol_bench_unpaced() == false )
		{
			LimitFrameRate( frameStart );
		}
	}
	else if ( s_frameLimit < 0 )
	{
		LimitFrameRate( frameStart );
	}
}

static void OnCleanup( void )
{
    gpu_loading_shutdown();
	// Destroy the sample first because it will destroy debug shapes.
	delete s_context.sample;
	s_context.sample = nullptr;
	s_context.Save();

	ShutdownUI();
	ShutdownRenderer();

	const int errors = GetRenderErrorCount();
	fprintf( stderr, "samples: %d frames, %d sokol errors\n", s_frame, errors );
	gpu_sokol_bench_finish( s_frame, errors, s_sampleName, kGpuSokolBenchMode );

#if defined( _WIN32 )
	timeEndPeriod( 1 );
#endif

	#ifdef TRACY_ENABLE
	tracy::ShutdownProfiler();
#endif

	// Return instead of exit so sokol frees its own window and context. main
	// dumps CRT leaks once sapp_run returns, past that teardown.
	s_exitCode = errors == 0 ? 0 : 1;
}

static void OnAppLog( const char* tag, uint32_t logLevel, uint32_t logItemId, const char* message, uint32_t lineNumber,
					  const char* filename, void* userData )
{
	(void)tag;
	(void)logItemId;
	(void)filename;
	(void)userData;
	(void)lineNumber;

	const char* level = ( logLevel == 0 ) ? "panic" : ( logLevel == 1 ) ? "error" : ( logLevel == 2 ) ? "warn" : "info";
	fprintf( stderr, "sokol(level %s) %s\n", level, message ? message : "(no message)" );

	if ( logLevel == 0 )
	{
		fprintf( stderr, "Ensure you have OpenGL 4.5 if on Linux" );
		exit( 1 );
	}
}

static sapp_desc BuildAppDesc( int argc, char** argv )
{
	for ( int i = 1; i < argc; ++i )
	{
		if ( strcmp( argv[i], "--frames" ) == 0 && i + 1 < argc )
		{
			s_frameLimit = atoi( argv[++i] );
		}
		else if ( strcmp( argv[i], "--sample-name" ) == 0 && i + 1 < argc )
		{
			snprintf( s_sampleName, sizeof s_sampleName, "%s", argv[++i] );
		}
		else if ( strcmp( argv[i], "--bench-json" ) == 0 && i + 1 < argc )
		{
			snprintf( s_benchJson, sizeof s_benchJson, "%s", argv[++i] );
		}
		else if ( strcmp( argv[i], "--warmup" ) == 0 && i + 1 < argc )
		{
			s_benchWarmup = atoi( argv[++i] );
		}
		else if ( strcmp( argv[i], "--timed" ) == 0 && i + 1 < argc )
		{
			s_benchTimed = atoi( argv[++i] );
		}
		else if ( strcmp( argv[i], "--paced" ) == 0 )
		{
			s_benchUnpaced = 0;
		}
		else if ( strcmp( argv[i], "--unpaced" ) == 0 )
		{
			s_benchUnpaced = 1;
		}
		else if ( strcmp( argv[i], "--completed-step" ) == 0 )
		{
			s_benchCompleted = 1;
		}
		else if ( strcmp( argv[i], "--pause-script" ) == 0 )
		{
			s_benchPauseScript = 1;
		}
		else if ( strcmp( argv[i], "--sleep" ) == 0 )
		{
			s_benchSleep = 1;
		}
		else if ( strcmp( argv[i], "--no-sleep" ) == 0 )
		{
			s_benchSleep = 0;
		}
		else if ( strcmp( argv[i], "--workers" ) == 0 && i + 1 < argc )
		{
			s_benchWorkers = atoi( argv[++i] );
		}
		else if ( strcmp( argv[i], "--health-scan" ) == 0 )
		{
			s_healthScan = 1;
		}
		else if ( strcmp( argv[i], "--hide-ui" ) == 0 )
		{
			s_hideUI = 1;
		}
		else if ( strcmp( argv[i], "--switch-sample-name" ) == 0 && i + 1 < argc )
		{
			snprintf( s_switchName, sizeof s_switchName, "%s", argv[++i] );
		}
		else if ( strcmp( argv[i], "--switch-after" ) == 0 && i + 1 < argc )
		{
			s_switchAfter = atoi( argv[++i] );
		}
		else if ( strcmp( argv[i], "--sample" ) == 0 && i + 1 < argc )
		{
			s_sampleOverride = atoi( argv[++i] );
		}
		else if ( strcmp( argv[i], "--replay" ) == 0 && i + 1 < argc )
		{
			const char* path = argv[++i];

			if ( g_replayIndex >= 0 )
			{
				snprintf( s_context.replayFile, sizeof( s_context.replayFile ), "%s", path );
				s_sampleOverride = g_replayIndex;
			}
		}
		else if ( strcmp( argv[i], "--zup" ) == 0 )
		{
			s_context.viewZUp = true;
		}
	}

	sapp_desc desc{};
	desc.init_cb = OnInit;
	desc.frame_cb = OnFrame;
	desc.event_cb = OnEvent;
	desc.cleanup_cb = OnCleanup;
	desc.logger.func = OnAppLog;

	// GL 4.5 for glClipControl (reverse-Z). Ignored on D3D11 / Metal.
	desc.gl.major_version = 4;
	desc.gl.minor_version = 5;

	desc.width = getenv("GPU_BENCH_WIDTH") ? atoi(getenv("GPU_BENCH_WIDTH")) : 1920;
	desc.height = getenv("GPU_BENCH_HEIGHT") ? atoi(getenv("GPU_BENCH_HEIGHT")) : 1080;

	// No swap-chain MSAA. The renderer runs MSAA in its own scene target.
	desc.sample_count = 1;

	// Static so the pointer outlives sokol_main; sokol keeps it for the window lifetime.
	b3Version version = b3GetVersion();
	static char title[64];
	snprintf( title, sizeof( title ), "Box3D %d.%d.%d - %s precision", version.major, version.minor,
			  version.revision, b3IsDoublePrecision() ? "double" : "single" );
	desc.window_title = title;

	// Vsync off: the software limiter in OnFrame owns the cadence. A hard 60 Hz
	// cap under vsync would beat against a non-60 display and pace to the wrong rate.
	desc.swap_interval = 0;
	desc.high_dpi = true;

	return desc;
}

// We own main (SOKOL_NO_ENTRY) so the leak dump can run after sapp_run returns,
// once sokol has torn down its window and context. On Windows sapp_run returns
// after that teardown; on macOS it may not return, so the dump is Windows only.
int main( int argc, char** argv )
{
#if defined( _MSC_VER )
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
#endif

	sapp_desc desc = BuildAppDesc( argc, argv );
	sapp_run( &desc );

#if defined( _MSC_VER )
	_CrtDumpMemoryLeaks();
#endif

	return s_exitCode;
}
