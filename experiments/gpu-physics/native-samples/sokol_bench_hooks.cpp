#include "sokol_bench_hooks.h"
#include "contact_metrics.h"
#include "sokol_capacity.h"

#include "box3d/box3d.h"
#include "sample.h"
#include "gfx/keycodes.h"
#include "sokol_app.h"

#include <cstdint>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#if defined(GPU_PHYSICS_SAMPLES) || defined(BOTH_SAMPLES)
extern "C" uint32_t gpu_samples_last_draw_shape_count(void);
#endif
#if defined(__linux__)
#include <dlfcn.h>
#include <GL/glx.h>
#endif

struct BodyHealth { int id; b3Pos p; b3Quat q; b3Vec3 v; b3Vec3 w; };
static std::vector<BodyHealth> g_body_health;
struct JointHealth { int id; bool anchor_constrained; float anchor_error; bool angular_constrained; float angular_error; };
static std::vector<JointHealth> g_joint_health;
static uint32_t g_scene_seed;
static bool g_village_drop_performed = false;

// Diagnostic omissions must never be accepted as native parity evidence.
uint32_t gpu_sokol_bench_gear_diagnostic(void)
{
    const char* value = getenv("GPU_GEAR_DIAGNOSTIC");
    if (!value || !*value) return 0;
    char* end = nullptr;
    unsigned long bits = strtoul(value, &end, 10);
    if (*end || bits > 7) {
        fprintf(stderr, "GPU_GEAR_DIAGNOSTIC expects bits 1=no debris, 2=no terrain, 4=no driver motor\n");
        abort();
    }
    return (uint32_t)bits;
}

uint32_t gpu_sokol_bench_scene_seed(uint32_t fallback)
{
	const char* seed = getenv("GPU_SOKOL_SEED");
	g_scene_seed = seed ? (uint32_t)strtoul(seed, nullptr, 10) : fallback;
	return g_scene_seed;
}

enum
{
	kMaxTimed = 4096,
	kMaxName = 128,
};

struct FrameRec
{
	char sample[128];
	int framebuffer_width;
	int framebuffer_height;
	GpuContactMetrics gpu_contacts;
	bool contact_count_known;
	uint64_t cadence_ns;
	uint64_t physics_ns;
	uint64_t setters_ns;
	uint64_t profile_ns;
	uint64_t pick_ns;
	uint64_t draw_ns;
	uint64_t render_ns;
	uint64_t ui_ns;
	uint64_t commit_ns;
	uint64_t limiter_ns;
	int submitted_step;
	int completed_step;
	int rendered_pose;
	uint32_t gpu_draw_shapes;
	uint64_t renderer_instances;
	int in_flight;
	int pause;
	int single_step;
	float query_wait_ms;
	float query_dispatch_ms;
	float query_map_ms;
	float query_encode_ms;
	uint64_t query_copied_bytes;
	int body_count;
	int joint_count;
	int contact_count;
	int nan_count;
	int worst_body;
	float min_y;
	float max_y;
	float max_speed;
	int exploded;
	uint64_t health_ns;
	std::vector<BodyHealth> bodies;
	std::vector<JointHealth> joints;
};

struct HealthAcc
{
	int dynamic_count;
	int nan_count;
	int worst_body;
	float min_y;
	float max_y;
	float max_speed;
};

static std::vector<uint8_t> g_health_seen;

static void scan_body(HealthAcc* acc, b3BodyId body)
{
	if (!b3Body_IsValid(body) || b3Body_GetType(body) != b3_dynamicBody)
	{
		return;
	}
	unsigned idx = (unsigned)body.index1;
	if (idx >= g_health_seen.size()) g_health_seen.resize((size_t)idx + 1, 0);
	if (g_health_seen[idx]) return;
	g_health_seen[idx] = 1;
	b3Pos p = b3Body_GetPosition(body);
	b3Vec3 v = b3Body_GetLinearVelocity(body);
	b3Quat q = b3Body_GetRotation(body);
	b3Vec3 w = b3Body_GetAngularVelocity(body);
	g_body_health.push_back({body.index1, p, q, v, w});
#if !defined(GPU_PHYSICS_SAMPLES)
	std::vector<b3JointId> joints(b3Body_GetJointCount(body));
	int joint_count = b3Body_GetJoints(body, joints.data(), (int)joints.size());
	for (int j = 0; j < joint_count; ++j) {
		auto id = joints[j];
		bool seen = false;
		for (const auto& old : g_joint_health) { if (old.id == id.index1) { seen = true; break; } }
		if (seen) { continue; }
		auto a = b3Body_GetWorldPoint(b3Joint_GetBodyA(id), b3Joint_GetLocalFrameA(id).p);
		auto b = b3Body_GetWorldPoint(b3Joint_GetBodyB(id), b3Joint_GetLocalFrameB(id).p);
		auto type = b3Joint_GetType(id);
		bool constrained = type == b3_revoluteJoint || type == b3_sphericalJoint || type == b3_weldJoint || type == b3_wheelJoint || type == b3_prismaticJoint;
		double dx = a.x-b.x, dy = a.y-b.y, dz = a.z-b.z;
		b3Quat qa = b3MulQuat(b3Body_GetRotation(b3Joint_GetBodyA(id)), b3Joint_GetLocalFrameA(id).q);
		b3Quat qb = b3MulQuat(b3Body_GetRotation(b3Joint_GetBodyB(id)), b3Joint_GetLocalFrameB(id).q);
		b3Vec3 axis = b3RotateVector(qa, {1,0,0});
		if (type == b3_wheelJoint || type == b3_prismaticJoint) {
			double axial = dx*axis.x + dy*axis.y + dz*axis.z;
			dx -= axial*axis.x; dy -= axial*axis.y; dz -= axial*axis.z;
		}
		float angular = 0;
		bool angular_constrained = type == b3_revoluteJoint || type == b3_weldJoint || type == b3_wheelJoint || type == b3_prismaticJoint;
		if (type == b3_revoluteJoint) {
			angular = acosf(fmaxf(-1, fminf(1, b3Dot(b3RotateVector(qa,{0,0,1}), b3RotateVector(qb,{0,0,1})))));
		} else if (type == b3_wheelJoint) {
			angular = asinf(fminf(1, fabsf(b3Dot(axis, b3RotateVector(qb,{0,0,1})))));
		} else if (angular_constrained) {
			angular = 2*acosf(fminf(1, fabsf(qa.s*qb.s + b3Dot(qa.v,qb.v))));
		}
		g_joint_health.push_back({id.index1, constrained, (float)sqrt(dx*dx+dy*dy+dz*dz), angular_constrained, angular});
	}
#endif
	float speed = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	bool finite = isfinite(p.x) && isfinite(p.y) && isfinite(p.z) && isfinite(speed) && isfinite(q.s)
		&& isfinite(q.v.x) && isfinite(q.v.y) && isfinite(q.v.z) && isfinite(w.x) && isfinite(w.y) && isfinite(w.z)
		&& isfinite(v.x) && isfinite(v.y) && isfinite(v.z);
	acc->dynamic_count += 1;
	if (!finite)
	{
		acc->nan_count += 1;
		acc->worst_body = body.index1;
	}
	if (p.y < acc->min_y)
	{
		acc->min_y = p.y;
	}
	if (p.y > acc->max_y)
	{
		acc->max_y = p.y;
	}
	if (speed > acc->max_speed)
	{
		acc->max_speed = speed;
		if (speed > 120.0f)
		{
			acc->worst_body = body.index1;
		}
	}
}

static bool health_overlap(b3ShapeId shapeId, void* context)
{
	HealthAcc* acc = static_cast<HealthAcc*>(context);
	scan_body(acc, b3Shape_GetBody(shapeId));
	return true;
}

static char g_json[1024];
static char g_sample[kMaxName];
static int g_warmup;
static int g_timed;
static bool g_unpaced;
static bool g_completed_step;
static bool g_pause_script;
static bool g_active;
static int g_swap_interval = -1;
static bool g_swap_configured;

// Sokol treats a zero descriptor interval as its default (one). Override the
// current GLX drawable after context creation and report the queried value.
// Other platforms retain unknown (-1), never an invented zero.
static void configure_bench_swap_interval()
{
    if (!g_active || g_swap_configured) return;
    g_swap_configured = true;
#if defined(__linux__)
    fprintf(stderr, "sokol-renderer: %s / %s; framebuffer=%dx%d\n", reinterpret_cast<const char*>(glGetString(GL_VENDOR)), reinterpret_cast<const char*>(glGetString(GL_RENDERER)), sapp_width(), sapp_height());
    void* library = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!library) return;
    auto currentDisplay = reinterpret_cast<Display* (*)()>(dlsym(library, "glXGetCurrentDisplay"));
    auto currentDrawable = reinterpret_cast<GLXDrawable (*)()>(dlsym(library, "glXGetCurrentDrawable"));
    auto extensions = reinterpret_cast<const char* (*)(Display*, int)>(dlsym(library, "glXQueryExtensionsString"));
    auto getProc = reinterpret_cast<__GLXextFuncPtr (*)(const GLubyte*)>(dlsym(library, "glXGetProcAddressARB"));
    auto query = reinterpret_cast<void (*)(Display*, GLXDrawable, int, unsigned int*)>(dlsym(library, "glXQueryDrawable"));
    Display* display = currentDisplay ? currentDisplay() : nullptr;
    GLXDrawable drawable = currentDrawable ? currentDrawable() : 0;
    const char* supported = display && extensions ? extensions(display, DefaultScreen(display)) : nullptr;
    if (display && drawable && getProc && query && supported && strstr(supported, "GLX_EXT_swap_control")) {
        auto setInterval = reinterpret_cast<void (*)(Display*, GLXDrawable, int)>(getProc(reinterpret_cast<const GLubyte*>("glXSwapIntervalEXT")));
        if (setInterval) {
            setInterval(display, drawable, 0);
            unsigned int actual = 1;
            query(display, drawable, GLX_SWAP_INTERVAL_EXT, &actual);
            g_swap_interval = static_cast<int>(actual);
        }
    }
    dlclose(library);
#endif
}
static int g_worker = 1;
static int g_sleep = 1;
static int g_frame;
static int g_measured;
static FrameRec g_frames[kMaxTimed];
static uint64_t g_last_entry;
static uint64_t g_mark_ns;
static const char* g_mark_phase = "";
static FrameRec g_acc;
static int g_requested_step;
static int g_status_ok = 1;
static int g_submitted_step;
static int g_completed_step_id;
static uint64_t g_snapshot_gen;
static int g_in_flight;
static int g_pause_advances;
static int g_single_advances;
static uint64_t g_pause_latency_ns[16];
static int g_pause_latencies;
static uint64_t g_pause_request_ns[16];
static int g_pause_requests;
static int g_last_seen_submitted;
static int g_cur_pause;
static int g_cur_single;
static char g_gpu_fail_msg[512];
static bool g_health_scan = false;
static int g_switch_count = 0;
static uint64_t g_health_ns;

static uint64_t now_ns(void)
{
	return (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

static double pctl(uint64_t* v, int n, double q)
{
	if (n <= 0)
	{
		return 0.0;
	}
	for (int i = 0; i < n; ++i)
	{
		for (int j = i + 1; j < n; ++j)
		{
			if (v[j] < v[i])
			{
				uint64_t t = v[i];
				v[i] = v[j];
				v[j] = t;
			}
		}
	}
	int idx = (int)(q * (n - 1));
	if (idx < 0)
	{
		idx = 0;
	}
	if (idx >= n)
	{
		idx = n - 1;
	}
	return (double)v[idx] / 1.0e6;
}

static void json_escape(FILE* f, const char* s)
{
	if (!s)
	{
		return;
	}
	for (; *s; ++s)
	{
		unsigned char c = (unsigned char)*s;
		if (c == '"' || c == '\\')
		{
			fputc('\\', f);
			fputc(c, f);
		}
		else if (c < 0x20)
		{
			fprintf(f, "\\u%04x", c);
		}
		else
		{
			fputc(c, f);
		}
	}
}

static void add_mark(uint64_t dt)
{
	if (strcmp(g_mark_phase, "setters") == 0)
	{
		g_acc.setters_ns += dt;
	}
	else if (strcmp(g_mark_phase, "physics") == 0)
	{
		g_acc.physics_ns += dt;
	}
	else if (strcmp(g_mark_phase, "profile") == 0)
	{
		g_acc.profile_ns += dt;
	}
	else if (strcmp(g_mark_phase, "pick") == 0)
	{
		g_acc.pick_ns += dt;
	}
	else if (strcmp(g_mark_phase, "draw") == 0)
	{
		g_acc.draw_ns += dt;
	}
	else if (strcmp(g_mark_phase, "render") == 0)
	{
		g_acc.render_ns += dt;
	}
	else if (strcmp(g_mark_phase, "ui") == 0)
	{
		g_acc.ui_ns += dt;
	}
	else if (strcmp(g_mark_phase, "commit") == 0)
	{
		g_acc.commit_ns += dt;
	}
	else if (strcmp(g_mark_phase, "limiter") == 0)
	{
		g_acc.limiter_ns += dt;
	}
}

void gpu_sokol_bench_configure(const char* json_path, int warmup, int timed, bool unpaced,
	bool completed_step, bool pause_script, int worker_count, int enable_sleep)
{
	g_active = json_path && json_path[0];
	if (!g_active)
	{
		return;
	}
	std::filesystem::path p(json_path);
	if (p.is_relative())
	{
		p = std::filesystem::absolute(p);
	}
	snprintf(g_json, sizeof g_json, "%s", p.string().c_str());
	g_warmup = warmup < 0 ? 0 : warmup;
	g_timed = timed < 1 ? 1 : timed;
	if (g_timed > kMaxTimed)
	{
		g_timed = kMaxTimed;
	}
	g_unpaced = unpaced;
	g_completed_step = completed_step;
	g_pause_script = pause_script;
	g_worker = worker_count > 0 ? worker_count : 1;
	g_sleep = enable_sleep;
}

void gpu_sokol_bench_set_health_scan(bool enable)
{
	g_health_scan = enable;
}

void gpu_sokol_bench_note_switch(void)
{
	g_switch_count += 1;
}

bool gpu_sokol_bench_active(void)
{
	return g_active;
}

bool gpu_sokol_bench_unpaced(void)
{
	return g_unpaced;
}

bool gpu_sokol_bench_completed_step(void)
{
	return g_completed_step;
}

int gpu_sokol_bench_swap_interval(void)
{
	return g_swap_interval;
}

int gpu_sokol_bench_sample_index_by_name(const char* name)
{
	if (!name || !name[0])
	{
		return -1;
	}
	const char* slash = strrchr(name, '/');
	const char* want_category = nullptr;
	const char* want_name = name;
	char category[kMaxName];
	if (slash && slash != name)
	{
		size_t n = (size_t)(slash - name);
		if (n >= sizeof category)
		{
			n = sizeof category - 1;
		}
		memcpy(category, name, n);
		category[n] = 0;
		want_category = category;
		want_name = slash + 1;
	}
	int found = -1;
	int matches = 0;
	for (int i = 0; i < g_sampleCount; ++i)
	{
		if (strcmp(g_sampleEntries[i].Name, want_name) != 0)
		{
			continue;
		}
		if (want_category && strcmp(g_sampleEntries[i].Category, want_category) != 0)
		{
			continue;
		}
		found = i;
		matches += 1;
	}
	if (matches == 1)
	{
		snprintf(g_sample, sizeof g_sample, "%s/%s", g_sampleEntries[found].Category, g_sampleEntries[found].Name);
		return found;
	}
	if (matches > 1)
	{
		fprintf(stderr, "sokol-bench: sample '%s' is ambiguous; use Category/Name\n", name);
	}
	else
	{
		fprintf(stderr, "sokol-bench: unknown sample '%s'\n", name);
	}
	g_status_ok = 0;
	return -1;
}

void gpu_sokol_bench_apply_script(bool* pause, int* single_step, int frame_index)
{
	if (g_active && !g_pause_script && pause && single_step)
	{
		*pause = false;
		*single_step = 0;
	}
	g_cur_pause = pause && *pause;
	g_cur_single = single_step ? *single_step : 0;
	if (!g_pause_script || !pause || !single_step)
	{
		return;
	}
	int after = g_warmup;
	if (frame_index == after)
	{
		*pause = 1;
		*single_step = 0;
	}
	else if (frame_index > after && frame_index <= after + 10)
	{
		*pause = 1;
		*single_step = 1;
	}
	else if (frame_index == after + 11)
	{
		*pause = 0;
		*single_step = 0;
	}
	g_cur_pause = *pause;
	g_cur_single = *single_step;
}

// Opt-in asynchronous GPU graphics probe. Missing/full slots drop samples;
// reading a timestamp never waits for rendering or changes the frame cadence.
static void graphics_probe(const char* phase)
{
#if defined(__linux__)
    if (!g_active || !getenv("GPU_SOKOL_GL_TIMESTAMPS")) return;
    static auto counter = reinterpret_cast<void (*)(GLuint, GLenum)>(glXGetProcAddressARB((const GLubyte*)"glQueryCounter"));
    static auto generate = reinterpret_cast<void (*)(GLsizei, GLuint*)>(glXGetProcAddressARB((const GLubyte*)"glGenQueries"));
    static auto available = reinterpret_cast<void (*)(GLuint, GLenum, GLint*)>(glXGetProcAddressARB((const GLubyte*)"glGetQueryObjectiv"));
    static auto result = reinterpret_cast<void (*)(GLuint, GLenum, GLuint64*)>(glXGetProcAddressARB((const GLubyte*)"glGetQueryObjectui64v"));
    if (!counter || !generate || !available || !result) return;
    struct Slot { GLuint ids[2]; int frame; bool pending; };
    static Slot slots[16] = {};
    static bool initialized = false;
    static int active = -1;
    if (!initialized) {
        for (auto& slot : slots) generate(2, slot.ids);
        initialized = true;
    }
    if (!strcmp(phase, "render")) {
        active = -1;
        for (int i = 0; i < 16; ++i) {
            auto& slot = slots[i];
            if (slot.pending) {
                GLint ready = 0;
                available(slot.ids[1], GL_QUERY_RESULT_AVAILABLE, &ready);
                if (ready) {
                    GLuint64 begin = 0, end = 0;
                    result(slot.ids[0], GL_QUERY_RESULT, &begin);
                    result(slot.ids[1], GL_QUERY_RESULT, &end);
                    fprintf(stderr, "gl-gpu-frame %d %.6f\n", slot.frame, double(end - begin) / 1e6);
                    slot.pending = false;
                }
            }
            if (!slot.pending && active < 0) active = i;
        }
        if (active >= 0) {
            slots[active].frame = g_frame;
            counter(slots[active].ids[0], GL_TIMESTAMP);
        }
    } else if (!strcmp(phase, "limiter") && active >= 0) {
        counter(slots[active].ids[1], GL_TIMESTAMP);
        slots[active].pending = true;
        active = -1;
    }
#endif
}

void gpu_sokol_bench_begin_frame(void)
{
    configure_bench_swap_interval();
	uint64_t t = now_ns();
	g_mark_ns = t;
	g_mark_phase = "entry";
	g_acc = FrameRec{};
	g_acc.framebuffer_width = sapp_width();
	g_acc.framebuffer_height = sapp_height();
	if (g_last_entry != 0 && g_frame >= g_warmup && g_measured < g_timed)
	{
		g_acc.cadence_ns = t - g_last_entry;
	}
	g_last_entry = t;
}

void gpu_sokol_bench_mark(const char* phase)
{
    graphics_probe(phase ? phase : "");
	uint64_t t = now_ns();
	add_mark(t - g_mark_ns);
	g_mark_ns = t;
	g_mark_phase = phase ? phase : "";
}

void gpu_sokol_bench_note_step_request(int requested_step)
{
	g_requested_step = requested_step;
	if (g_pause_script && g_cur_single > 0 && g_pause_requests < 16)
	{
		g_pause_request_ns[g_pause_requests++] = now_ns();
	}
}

void gpu_sokol_bench_note_identities(int submitted_step, uint64_t snapshot_gen, int completed_step,
	int in_flight)
{
	g_submitted_step = submitted_step;
	g_snapshot_gen = snapshot_gen;
	g_completed_step_id = completed_step;
	g_in_flight = in_flight;
}

void gpu_sokol_bench_note_query(float wait_ms, float dispatch_ms, float map_ms, uint64_t copied_bytes,
	float encode_ms)
{
	g_acc.query_wait_ms = wait_ms;
	g_acc.query_dispatch_ms = dispatch_ms;
	g_acc.query_map_ms = map_ms;
	g_acc.query_copied_bytes = copied_bytes;
	g_acc.query_encode_ms = encode_ms;
}

#if defined(GPU_PHYSICS_SAMPLES)
struct GpuHealthScan
{
	int32_t dynamic_count;
	int32_t nan_count;
	int32_t worst_body;
	float min_y;
	float max_y;
	float max_speed;
	int32_t exploded;
};
extern "C" GpuHealthScan gpu_b3_world_health_scan(b3WorldId world);
extern "C" void gpu_b3_world_visit_dynamic_bodies(b3WorldId, void (*)(b3BodyId, void*), void*);
extern "C" void gpu_b3_world_visit_joint_health(b3WorldId, void (*)(int, bool, float, bool, float));
static void health_joint_visit(int id, bool constrained, float error, bool angular_constrained, float angular_error) { g_joint_health.push_back({id, constrained, error, angular_constrained, angular_error}); }
static void health_visit(b3BodyId body, void* context) { scan_body(static_cast<HealthAcc*>(context), body); }
extern "C" const char* gpu_b3_world_gpu_fail(b3WorldId world);
#endif

// Explicit interaction fixture on the unchanged native Village geometry.
// Disabled for ordinary viewing and all other samples; identical CPU/GPU setup.
void gpu_sokol_bench_village_drop(b3WorldId world, const char* sample, int frame)
{
    const char* enabled = getenv("GPU_SOKOL_VILLAGE_DROP");
    if (!g_active || frame != 0 || !enabled || strcmp(enabled, "1") != 0 ||
        !sample || strcmp(sample, "Compound/Village") != 0) return;
    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = b3_dynamicBody;
    bodyDef.position = {0.0f, 30.0f, 0.0f};
    b3BodyId body = b3CreateBody(world, &bodyDef);
    b3ShapeDef shapeDef = b3DefaultShapeDef();
    b3Sphere sphere = {b3Vec3_zero, 0.5f};
    b3ShapeId shape = b3CreateSphereShape(body, &shapeDef, &sphere);
    g_village_drop_performed = true;
    fprintf(stderr, "village-drop body=%d shape=%d position=0,30,0 radius=0.5\n", body.index1, shape.index1);
}

void gpu_sokol_bench_note_world(b3WorldId world)
{
	if (!g_active || !b3World_IsValid(world))
	{
		return;
	}
	snprintf(g_acc.sample, sizeof g_acc.sample, "%s", g_sample);
	// Native Bounce House deliberately launches at sqrt(120^2 + 120^2) m/s.
	const float speed_limit = strcmp(g_sample, "Continuous/Bounce House") == 0 ? 180.0f : 120.0f;
	b3Counters counters = b3World_GetCounters(world);
	g_acc.body_count = counters.bodyCount;
	g_acc.joint_count = counters.jointCount;
    g_acc.contact_count = counters.contactCount;
#if defined(GPU_PHYSICS_SAMPLES) || defined(BOTH_SAMPLES)
    g_acc.contact_count_known = false; // Native public contact IDs are not implemented.
    gpu_b3_world_contact_metrics(world, &g_acc.gpu_contacts);
#else
    g_acc.contact_count_known = true;
#endif
	if (!g_health_scan)
	{
		return;
	}
	uint64_t t0 = now_ns();
	HealthAcc acc = {};
	acc.min_y = 1.0e9f;
	acc.max_y = -1.0e9f;
	std::fill(g_health_seen.begin(), g_health_seen.end(), 0);
	g_body_health.clear();
	g_joint_health.clear();
#if defined(GPU_PHYSICS_SAMPLES)
	GpuHealthScan scan = gpu_b3_world_health_scan(world);
	HealthAcc body_scan = {};
	gpu_b3_world_visit_dynamic_bodies(world, health_visit, &body_scan);
	gpu_b3_world_visit_joint_health(world, health_joint_visit);
	acc.dynamic_count = scan.dynamic_count;
	acc.nan_count = scan.nan_count;
	acc.worst_body = scan.worst_body;
	acc.min_y = scan.min_y;
	acc.max_y = scan.max_y;
	acc.max_speed = scan.max_speed;
	g_acc.exploded = scan.nan_count > 0 || scan.max_speed > speed_limit || scan.max_y > 200.0f;
	const char* fail = gpu_b3_world_gpu_fail(world);
	if (fail && fail[0])
	{
		snprintf(g_gpu_fail_msg, sizeof g_gpu_fail_msg, "%s", fail);
		g_status_ok = 0;
	}
#else
	b3AABB aabb = { { -1.0e5f, -1.0e5f, -1.0e5f }, { 1.0e5f, 1.0e5f, 1.0e5f } };
	b3World_OverlapAABB(world, aabb, b3DefaultQueryFilter(), health_overlap, &acc);
	g_acc.exploded = acc.nan_count > 0 || acc.max_speed > speed_limit || acc.max_y > 200.0f;
#endif
	g_acc.nan_count = acc.nan_count;
	g_acc.bodies = g_body_health;
	g_acc.joints = g_joint_health;
	g_acc.worst_body = acc.worst_body;
	g_acc.min_y = acc.dynamic_count > 0 ? acc.min_y : 0.0f;
	g_acc.max_y = acc.dynamic_count > 0 ? acc.max_y : 0.0f;
	g_acc.max_speed = acc.max_speed;
	if (g_acc.exploded)
	{
		g_status_ok = 0;
	}
	g_health_ns = now_ns() - t0;
	g_acc.health_ns = g_health_ns;
}

void gpu_sokol_bench_end_frame(void)
{
	gpu_sokol_bench_mark("end");
	int rendered = g_snapshot_gen != 0 ? (int)g_snapshot_gen : g_submitted_step;
	if (g_pause_script)
	{
		if (g_cur_pause && g_cur_single == 0 && g_submitted_step > g_last_seen_submitted
			&& g_last_seen_submitted != 0)
		{
			g_pause_advances += 1;
		}
		if (g_cur_single > 0)
		{
			g_single_advances += 1;
			if (g_pause_latencies < 16 && g_pause_latencies < g_pause_requests)
			{
				uint64_t t = now_ns();
				if (t >= g_pause_request_ns[g_pause_latencies])
				{
					g_pause_latency_ns[g_pause_latencies] = t - g_pause_request_ns[g_pause_latencies];
				}
				g_pause_latencies += 1;
			}
		}
	}
	if (g_submitted_step > g_last_seen_submitted)
	{
		g_last_seen_submitted = g_submitted_step;
	}
	if (g_frame >= g_warmup && g_measured < g_timed)
	{
		FrameRec rec = g_acc;
		rec.renderer_instances = sample_renderer_instance_count();
#if defined(GPU_PHYSICS_SAMPLES) || defined(BOTH_SAMPLES)
		rec.gpu_draw_shapes = gpu_samples_last_draw_shape_count();
#endif
		rec.submitted_step = g_submitted_step;
		rec.completed_step = g_completed_step_id;
		rec.rendered_pose = rendered;
		rec.in_flight = g_in_flight;
		rec.pause = g_cur_pause;
		rec.single_step = g_cur_single;
		g_frames[g_measured] = rec;
		g_measured += 1;
	}
	g_frame += 1;
}

void gpu_sokol_bench_finish(int frames, int sokol_errors, const char* sample_name, const char* mode)
{
	if (!g_active)
	{
		return;
	}
	if (sokol_errors != 0 || g_measured < g_timed || g_sample[0] == 0)
	{
		g_status_ok = 0;
	}
	if (g_pause_script)
	{
		if (g_pause_advances != 0 || g_single_advances != 10)
		{
			fprintf(stderr, "sokol-bench pause script: idle_advances=%d single=%d\n", g_pause_advances,
				g_single_advances);
			g_status_ok = 0;
		}
	}
	std::error_code ec;
	std::filesystem::path out(g_json);
	if (out.has_parent_path())
	{
		std::filesystem::create_directories(out.parent_path(), ec);
	}
	FILE* f = fopen(g_json, "w");
	if (!f)
	{
		perror("sokol-bench json");
		g_status_ok = 0;
		return;
	}
	uint64_t cadence[kMaxTimed];
	uint64_t physics[kMaxTimed];
	uint64_t draw[kMaxTimed];
	uint64_t render[kMaxTimed];
	uint64_t ui[kMaxTimed];
	uint64_t commit[kMaxTimed];
	uint64_t limiter[kMaxTimed];
	uint64_t setters[kMaxTimed];
	uint64_t profile[kMaxTimed];
	uint64_t pick[kMaxTimed];
	uint64_t health[kMaxTimed];
	for (int i = 0; i < g_measured; ++i)
	{
		cadence[i] = g_frames[i].cadence_ns;
		physics[i] = g_frames[i].physics_ns;
		draw[i] = g_frames[i].draw_ns;
		render[i] = g_frames[i].render_ns;
		ui[i] = g_frames[i].ui_ns;
		commit[i] = g_frames[i].commit_ns;
		limiter[i] = g_frames[i].limiter_ns;
		setters[i] = g_frames[i].setters_ns;
		profile[i] = g_frames[i].profile_ns;
		pick[i] = g_frames[i].pick_ns;
		health[i] = g_frames[i].health_ns;
	}
	uint64_t pause_copy[16];
	memcpy(pause_copy, g_pause_latency_ns, sizeof pause_copy);
	fprintf(f, "{\n  \"village_drop\": %s,\n", g_village_drop_performed ? "true" : "false");
	fprintf(f, "  \"gear_diagnostic\": %u,\n  \"scene_seed\": %u,\n  \"status\": \"%s\",\n  \"mode\": \"", gpu_sokol_bench_gear_diagnostic(), g_scene_seed, g_status_ok ? "ok" : "incomplete");
	json_escape(f, mode ? mode : "unknown");
	fprintf(f, "\",\n  \"sample\": \"");
	json_escape(f, sample_name && sample_name[0] ? sample_name : g_sample);
	fprintf(f,
		"\",\n"
		"  \"frames_observed\": %d,\n"
		"  \"warmup\": %d,\n"
		"  \"timed\": %d,\n"
		"  \"measured\": %d,\n"
		"  \"unpaced\": %s,\n"
		"  \"completed_step_mode\": %s,\n"
		"  \"pause_script\": %s,\n"
		"  \"pause_idle_advances\": %d,\n"
		"  \"pause_single_advances\": %d,\n"
		"  \"pause_request_to_render_p50_ms\": %.4f,\n"
		"  \"worker_count\": %d,\n"
		"  \"enable_sleep\": %s,\n"
		"  \"cpu_win_validated\": false,\n"
		"  \"swap_interval\": %d,\n"
		"  \"presentation\": \"queried swap interval (-1 unknown); limiter %s\",\n"
		"  \"cadence_p50_ms\": %.4f,\n"
		"  \"cadence_p95_ms\": %.4f,\n"
		"  \"physics_p50_ms\": %.4f,\n"
		"  \"physics_p95_ms\": %.4f,\n"
		"  \"setters_p50_ms\": %.4f,\n"
		"  \"profile_p50_ms\": %.4f,\n"
		"  \"pick_p50_ms\": %.4f,\n"
		"  \"draw_p50_ms\": %.4f,\n"
		"  \"render_p50_ms\": %.4f,\n"
		"  \"ui_p50_ms\": %.4f,\n"
		"  \"commit_p50_ms\": %.4f,\n"
		"  \"limiter_p50_ms\": %.4f,\n"
		"  \"step_p50_ms\": %.4f,\n"
		"  \"step_p95_ms\": %.4f,\n"
		"  \"last_submitted_step\": %d,\n"
		"  \"last_completed_step\": %d,\n"
		"  \"last_rendered_pose\": %d,\n"
		"  \"snapshot_generation\": %" PRIu64 ",\n"
		"  \"in_flight\": %d,\n"
		"  \"requested_step\": %d,\n"
		"  \"health_scan\": %s,\n"
		"  \"switch_count\": %d,\n"
		"  \"health_p50_ms\": %.4f,\n"
		"  \"gpu_fail\": \"%s\",\n"
		"  \"notes\": \"cadence is callback-entry to next entry. physics is b3World_Step plus optional completed wait. Health scan is correctness-only and is timed separately as health_p50_ms; omit it for performance. draw is b3World_Draw/pose list. render is RenderFrame. ui is StartUIFrame+RenderUI. commit is sg_commit, not photon time. GPU draw execution is unknown without GPU timestamps. both-mode is two worlds. pause latencies are request-to-rendered-pose.\",\n"
		"  \"frames\": [\n",
		frames,
		g_warmup,
		g_timed,
		g_measured,
		g_unpaced ? "true" : "false",
		g_completed_step ? "true" : "false",
		g_pause_script ? "true" : "false",
		g_pause_advances,
		g_single_advances,
		pctl(pause_copy, g_pause_latencies, 0.50),
		g_worker,
		g_sleep ? "true" : "false",
		g_swap_interval,
		g_unpaced ? "off" : "LimitFrameRate",
		pctl(cadence, g_measured, 0.50),
		pctl(cadence, g_measured, 0.95),
		pctl(physics, g_measured, 0.50),
		pctl(physics, g_measured, 0.95),
		pctl(setters, g_measured, 0.50),
		pctl(profile, g_measured, 0.50),
		pctl(pick, g_measured, 0.50),
		pctl(draw, g_measured, 0.50),
		pctl(render, g_measured, 0.50),
		pctl(ui, g_measured, 0.50),
		pctl(commit, g_measured, 0.50),
		pctl(limiter, g_measured, 0.50),
		pctl(physics, g_measured, 0.50),
		pctl(physics, g_measured, 0.95),
		g_measured > 0 ? g_frames[g_measured - 1].submitted_step : 0,
		g_measured > 0 ? g_frames[g_measured - 1].completed_step : 0,
		g_measured > 0 ? g_frames[g_measured - 1].rendered_pose : 0,
		g_snapshot_gen,
		g_in_flight,
		g_requested_step,
		g_health_scan ? "true" : "false",
		g_switch_count,
		pctl(health, g_measured, 0.50),
		g_gpu_fail_msg[0] ? g_gpu_fail_msg : "");
	for (int i = 0; i < g_measured; ++i)
	{
		const FrameRec& r = g_frames[i];
		fputs("    {\"sample\":\"", f);
        json_escape(f, r.sample);
        const auto& m = r.gpu_contacts;
        fprintf(f, "\",\"contact_count_known\":%s,\"gpu_contact_metrics\":{\"phase\":\"contact_scheduling\",\"known\":%s,\"current\":%s,\"capacity_loss\":%s,\"snapshot_step\":%llu,\"submitted_step\":%llu,\"snapshot_topology\":%llu,\"current_topology\":%llu,\"snapshot_state\":%llu,\"current_state\":%llu,\"candidate_pairs\":%u,\"allocated_roots\":%u,\"allocated_manifold_slots\":%u,\"touching_roots\":%u,\"non_sensor_roots\":%u}",
            r.contact_count_known ? "true" : "false", m.known ? "true" : "false", m.current ? "true" : "false", m.capacity_loss ? "true" : "false",
            (unsigned long long)m.snapshot_step,(unsigned long long)m.submitted_step,
            (unsigned long long)m.snapshot_topology,(unsigned long long)m.current_topology,
            (unsigned long long)m.snapshot_state,(unsigned long long)m.current_state,
            m.candidate_pairs,m.allocated_roots,m.allocated_manifold_slots,m.touching_roots,m.non_sensor_roots);
        fprintf(f,
			",\"i\":%d,\"framebuffer_width\":%d,\"framebuffer_height\":%d,\"cadence_ms\":%.4f,\"physics_ms\":%.4f,\"setters_ms\":%.4f,\"profile_ms\":%.4f,"
			"\"pick_ms\":%.4f,\"draw_ms\":%.4f,\"render_ms\":%.4f,\"ui_ms\":%.4f,\"commit_ms\":%.4f,"
			"\"limiter_ms\":%.4f,\"submitted_step\":%d,\"completed_step\":%d,\"rendered_pose\":%d,"
			"\"gpu_draw_shapes\":%u,\"renderer_instances\":%" PRIu64 ",\"in_flight\":%d,\"pause\":%s,\"single_step\":%s,"
			"\"query_wait_ms\":%.4f,\"query_dispatch_ms\":%.4f,\"query_map_ms\":%.4f,\"query_encode_ms\":%.4f,"
			"\"query_copied_bytes\":%" PRIu64
			",\"body_count\":%d,\"joint_count\":%d,\"contact_count\":%d,\"nan_count\":%d,"
			"\"worst_body\":%d,\"min_y\":%.4f,\"max_y\":%.4f,\"max_speed\":%.4f,\"exploded\":%s,"
			"\"health_ms\":%.4f,\"bodies\":[",
			i,
			r.framebuffer_width,
			r.framebuffer_height,
			r.cadence_ns / 1e6,
			r.physics_ns / 1e6,
			r.setters_ns / 1e6,
			r.profile_ns / 1e6,
			r.pick_ns / 1e6,
			r.draw_ns / 1e6,
			r.render_ns / 1e6,
			r.ui_ns / 1e6,
			r.commit_ns / 1e6,
			r.limiter_ns / 1e6,
			r.submitted_step,
			r.completed_step,
			r.rendered_pose,
			r.gpu_draw_shapes,
			r.renderer_instances,
			r.in_flight,
			r.pause ? "true" : "false",
			r.single_step ? "true" : "false",
			r.query_wait_ms,
			r.query_dispatch_ms,
			r.query_map_ms,
			r.query_encode_ms,
			r.query_copied_bytes,
			r.body_count,
			r.joint_count,
			r.contact_count,
			r.nan_count,
			r.worst_body,
			r.min_y,
			r.max_y,
			r.max_speed,
			r.exploded ? "true" : "false",
			r.health_ns / 1e6);
		for (size_t j = 0; j < r.bodies.size(); ++j) {
			const auto& b = r.bodies[j];
			fprintf(f, "%s{\"id\":%d,\"p\":[%.9g,%.9g,%.9g],\"q\":[%.9g,%.9g,%.9g,%.9g],\"v\":[%.9g,%.9g,%.9g],\"w\":[%.9g,%.9g,%.9g]}",
				j ? "," : "", b.id, b.p.x, b.p.y, b.p.z, b.q.v.x, b.q.v.y, b.q.v.z, b.q.s,
				b.v.x, b.v.y, b.v.z, b.w.x, b.w.y, b.w.z);
		}
		fprintf(f, "],\"joints\":[");
		for (size_t j = 0; j < r.joints.size(); ++j) {
			const auto& joint = r.joints[j];
			fprintf(f, "%s{\"id\":%d,\"anchor_constrained\":%s,\"anchor_error\":%.9g,\"angular_constrained\":%s,\"angular_error\":%.9g}",
				j ? "," : "", joint.id, joint.anchor_constrained ? "true" : "false", joint.anchor_error,
				joint.angular_constrained ? "true" : "false", joint.angular_error);
		}
		fprintf(f, "]}%s\n", i + 1 == g_measured ? "" : ",");
	}
	fprintf(f, "  ]\n}\n");
	fclose(f);
	if (!g_status_ok)
	{
		fprintf(stderr, "sokol-bench: incomplete %s\n", g_json);
	}
}

// Exercise the actual default Shift-click handler after startup. Disabled unless
// explicitly requested for a benchmark; no alternate projectile implementation.
void gpu_sokol_bench_shoot(void* sample, int frame, int width, int height)
{
    const char* schedule = getenv("GPU_SOKOL_SHOOT_FRAMES");
    if (!gpu_sokol_bench_active() || !schedule) return;
    for (const char* p = schedule; *p; ) {
        char* end;
        long shot = strtol(p, &end, 10);
        if (end == p || (*end && *end != ',')) abort();
        p = *end ? end + 1 : end;
        if (shot != frame) continue;
        const uint64_t start = b3GetTicks();
        auto* instance = static_cast<Sample*>(sample);
        instance->MouseDown({0.5f * width, 0.5f * height}, 0, MOD_SHIFT);
        instance->MouseUp({0.5f * width, 0.5f * height}, 0);
        fprintf(stderr, "sokol-shoot frame=%d input_ms=%.3f\n", frame, b3GetMilliseconds(start));
    }
}
