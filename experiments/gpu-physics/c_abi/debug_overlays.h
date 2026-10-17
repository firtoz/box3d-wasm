// Viewer-only overlays from actual GPU diagnostic data. No solver state writes.
#pragma once
#include "box3d/constants.h"

extern void gpu_b3_body_get_world_center(b3BodyId, float*);
extern const char* gpu_b3_body_get_name(b3BodyId);
extern bool gpu_b3_body_is_awake(b3BodyId);
extern int gpu_b3_body_get_joint_count(b3BodyId);
extern int gpu_b3_body_get_joints(b3BodyId, b3JointId*, int);
extern b3BodyId gpu_b3_joint_get_body(b3JointId, bool);
extern void gpu_b3_joint_get_local_frame(b3JointId, bool, float*);
extern void gpu_b3_joint_get_constraint_force(b3JointId, float*);
extern void gpu_b3_joint_get_constraint_torque(b3JointId, float*);

typedef struct JointVis { b3JointId id; float scale; } JointVis;
static GpuSlots g_joint_vis[GPU_SAMPLES_WORLD_CAP];
static void overlay_release_joints(unsigned world) { gpu_slots_release(&g_joint_vis[world]); }

void gpu_samples_on_joint_created(b3JointId id, const b3JointDef* def)
{
	if (id.world0 == 0 || id.world0 >= GPU_SAMPLES_WORLD_CAP || !def) return;
	JointVis* v = gpu_slots_get(&g_joint_vis[id.world0], id.index1, sizeof(JointVis), true);
	if (v) { v->id = id; v->scale = def->drawScale; }
}

void gpu_samples_on_world_step(b3WorldId id, float dt)
{
	WorldVis* v = world_vis(id);
	if (v) v->inv_dt = dt > 0.0f ? 1.0f / dt : 0.0f;
}

// Full identities, including the 32-bit contact generation. Scratch storage
// grows with this draw's visible records, never with lifetime contact IDs.
typedef struct DrawKey { uint32_t index, world, generation; } DrawKey;
typedef struct DrawSet { DrawKey* keys; size_t capacity, count; } DrawSet;
static size_t draw_key_hash(DrawKey k)
{
	uint64_t h = (uint64_t)k.index * 0x9e3779b185ebca87ULL;
	h ^= (uint64_t)k.generation * 0xc2b2ae3d27d4eb4fULL;
	h ^= (uint64_t)k.world * 0x165667b19e3779f9ULL;
	return (size_t)(h ^ (h >> 32));
}
static bool draw_set_add(DrawSet* set, DrawKey key)
{
	if (!key.index) return false;
	if (2 * (set->count + 1) >= set->capacity)
	{
		size_t cap = set->capacity ? 2 * set->capacity : 32;
		if (cap > SIZE_MAX / sizeof(DrawKey)) gpu_slots_oom();
		DrawKey* keys = calloc(cap, sizeof(DrawKey));
		if (!keys) gpu_slots_oom();
		for (size_t i = 0; i < set->capacity; ++i) if (set->keys[i].index)
		{
			size_t slot = draw_key_hash(set->keys[i]) & (cap - 1);
			while (keys[slot].index) slot = (slot + 1) & (cap - 1);
			keys[slot] = set->keys[i];
		}
		free(set->keys); set->keys = keys; set->capacity = cap;
	}
	size_t slot = draw_key_hash(key) & (set->capacity - 1);
	while (set->keys[slot].index)
	{
		DrawKey old = set->keys[slot];
		if (old.index == key.index && old.world == key.world && old.generation == key.generation) return false;
		slot = (slot + 1) & (set->capacity - 1);
	}
	set->keys[slot] = key; ++set->count; return true;
}

static b3WorldTransform overlay_body_transform(b3BodyId id, bool center)
{
	float p[3], q[4]; gpu_b3_body_get_transform(id, p, q);
	if (center) gpu_b3_body_get_world_center(id, p);
	return (b3WorldTransform){{p[0], p[1], p[2]}, {{q[0], q[1], q[2]}, q[3]}};
}
static void overlay_segment(b3DebugDraw* d, b3Pos a, b3Pos b, b3HexColor color)
{
	if (d->DrawSegmentFcn) d->DrawSegmentFcn(a, b, color, d->context);
}
static void overlay_axes(b3DebugDraw* d, b3WorldTransform xf, float scale)
{
	const b3Vec3 axes[3] = {{1,0,0}, {0,1,0}, {0,0,1}};
	const b3HexColor colors[3] = {b3_colorRed, b3_colorGreen, b3_colorBlue};
	for (int i = 0; i < 3; ++i)
		overlay_segment(d, xf.p, b3TransformWorldPoint(xf, b3MulSV(scale, axes[i])), colors[i]);
}
static void overlay_joints(b3BodyId body, b3DebugDraw* d, DrawSet* seen)
{
	int capacity = gpu_b3_body_get_joint_count(body);
	if (capacity <= 0) return;
	if ((size_t)capacity > SIZE_MAX / sizeof(b3JointId)) gpu_slots_oom();
	b3JointId* joints = malloc((size_t)capacity * sizeof(b3JointId));
	if (!joints) gpu_slots_oom();
	int count = gpu_b3_body_get_joints(body, joints, capacity);
	for (int i = 0; i < count; ++i)
	{
		b3JointId id = joints[i];
		if (!gpu_b3_joint_is_valid(id) || !draw_set_add(seen, (DrawKey){id.index1,id.world0,id.generation})) continue;
		b3WorldTransform frames[2];
		for (int side = 0; side < 2; ++side)
		{
			b3BodyId b = gpu_b3_joint_get_body(id, side != 0);
			float f[7]; gpu_b3_joint_get_local_frame(id, side != 0, f);
			b3Transform local = {{f[0],f[1],f[2]}, {{f[3],f[4],f[5]},f[6]}};
			b3WorldTransform xf = overlay_body_transform(b, false);
			frames[side] = (b3WorldTransform){b3TransformWorldPoint(xf, local.p), b3MulQuat(xf.q, local.q)};
		}
		JointVis* v = gpu_slots_get(&g_joint_vis[id.world0], id.index1, sizeof(JointVis), false);
		// Internal mouse joints bypass public creation; native units are their fallback.
		float localScale = v && v->id.generation == id.generation ? v->scale : b3GetLengthUnitsPerMeter();
		float scale = b3MaxFloat(0.0001f, d->jointScale * localScale);
		overlay_segment(d, frames[0].p, frames[1].p, b3_colorGold);
		overlay_axes(d, frames[0], 0.3f * scale);
		overlay_axes(d, frames[1], 0.3f * scale);
		if (d->DrawPointFcn) { d->DrawPointFcn(frames[0].p, 5, b3_colorGold, d->context); d->DrawPointFcn(frames[1].p, 5, b3_colorGold, d->context); }
		if (d->drawJointExtras)
		{
			b3Vec3 force, torque;
			gpu_b3_joint_get_constraint_force(id, &force.x); gpu_b3_joint_get_constraint_torque(id, &torque.x);
			overlay_segment(d, frames[0].p, b3OffsetPos(frames[0].p, b3MulSV(d->forceScale, force)), b3_colorMagenta);
			overlay_segment(d, frames[0].p, b3OffsetPos(frames[0].p, b3MulSV(d->forceScale, torque)), b3_colorYellow);
			if (d->DrawStringFcn) { char text[80]; snprintf(text,sizeof(text),"F %.2f T %.2f",b3Length(force),b3Length(torque)); d->DrawStringFcn(frames[0].p,text,b3_colorWhite,d->context); }
		}
	}
	free(joints);
}
static void overlay_contacts(b3WorldId world, b3BodyId body, b3DebugDraw* d, DrawSet* seen)
{
	if (gpu_b3_body_get_type(body) != b3_dynamicBody || !gpu_b3_body_is_awake(body)) return;
	int capacity = b3Body_GetContactCapacity(body);
	if (capacity <= 0) return;
	if ((size_t)capacity > SIZE_MAX / sizeof(b3ContactData)) gpu_slots_oom();
	b3ContactData* contacts = malloc((size_t)capacity * sizeof(b3ContactData));
	if (!contacts) gpu_slots_oom();
	int count = b3Body_GetContactData(body, contacts, capacity);
	float inv_dt = world_vis(world)->inv_dt;
	if (inv_dt <= 0) inv_dt = 60;
	float unit = b3GetLengthUnitsPerMeter();
	for (int i = 0; i < count; ++i)
	{
		b3ContactData* c = contacts + i;
		b3ContactId id = c->contactId;
		if (!draw_set_add(seen, (DrawKey){id.index1,id.world0,id.generation})) continue;
		b3BodyId anchorBody = gpu_b3_shape_get_body(d->drawAnchorA ? c->shapeIdA : c->shapeIdB);
		float center[3]; gpu_b3_body_get_world_center(anchorBody, center);
		b3Pos origin = {center[0], center[1], center[2]};
		// Borrowed manifolds are consumed before the next contact getter.
		for (int j = 0; j < c->manifoldCount; ++j)
		{
			const b3Manifold* m = c->manifolds + j;
			b3Vec3 weighted = b3Vec3_zero; float totalWeight = 0;
			for (int k = 0; k < m->pointCount; ++k)
			{
				const b3ManifoldPoint* p = m->points + k;
				b3Vec3 anchor = d->drawAnchorA ? p->anchorA : p->anchorB;
				b3Pos point = b3OffsetPos(origin, anchor);
				float weight = b3ClampFloat(2.0f - p->separation / B3_SPECULATIVE_DISTANCE, B3_MIN_FRICTION_WEIGHT, 1.0f);
				weighted = b3MulAdd(weighted, weight, anchor); totalWeight += weight;
				char text[32]; bool label = true;
				if (d->drawContactNormals) { overlay_segment(d,point,b3OffsetPos(point,b3MulSV(0.3f*unit,m->normal)),b3_colorLightGray); snprintf(text,sizeof(text),"   %.2f",100*p->separation); }
				else if (d->drawContactForces) { float force = 0.5f*p->totalNormalImpulse*inv_dt; overlay_segment(d,point,b3OffsetPos(point,b3MulSV(d->forceScale*force,m->normal)),b3_colorMagenta); snprintf(text,sizeof(text),"  %.1f",force); }
				else if (d->drawContactFeatures) snprintf(text,sizeof(text),"   %#X",p->featureId);
				else label = false;
				if (label && d->DrawStringFcn) d->DrawStringFcn(point,text,b3_colorWhite,d->context);
				b3HexColor color = !p->persisted ? b3_colorLimeGreen : p->separation > B3_SPECULATIVE_DISTANCE ? b3_colorDarkBlue : p->separation > 0 ? b3_colorBlue : b3_colorLightBlue;
				if (d->DrawPointFcn) d->DrawPointFcn(point,p->persisted ? 10 : 20,color,d->context);
			}
			if (d->drawContactForces && totalWeight > 0)
			{
				b3Pos p = b3OffsetPos(origin,b3MulSV(1/totalWeight,weighted));
				b3Vec3 force = b3MulSV(0.5f*inv_dt,m->frictionImpulse);
				overlay_segment(d,p,b3OffsetPos(p,b3MulSV(d->forceScale,force)),b3_colorYellow);
				if (d->DrawPointFcn) d->DrawPointFcn(p,5,b3_colorYellow,d->context);
				if (d->DrawStringFcn) { char text[32]; snprintf(text,sizeof(text),"%.2f",b3Length(force)); d->DrawStringFcn(b3OffsetPos(p,b3MulSV(0.05f*unit,m->normal)),text,b3_colorWhite,d->context); }
			}
		}
	}
	free(contacts);
}
static void overlay_body(b3WorldId world, b3BodyId id, b3DebugDraw* d, DrawSet* joints, DrawSet* contacts)
{
	if (d->drawBodyNames || d->drawMass)
	{
		b3WorldTransform xf = overlay_body_transform(id, true);
		b3Pos label = b3TransformWorldPoint(xf,(b3Vec3){0.05f,0.05f,0.05f});
		if (d->drawBodyNames && d->DrawStringFcn) { const char* name = gpu_b3_body_get_name(id); if (name && name[0]) d->DrawStringFcn(label,name,b3_colorWhite,d->context); }
		if (d->drawMass)
		{
			if (d->DrawTransformFcn) d->DrawTransformFcn(xf,d->context);
			if (gpu_b3_body_get_type(id) == b3_dynamicBody && d->DrawStringFcn) { char text[32]; snprintf(text,sizeof(text),"%.2f",gpu_b3_body_get_mass(id)); d->DrawStringFcn(label,text,b3_colorWhite,d->context); }
		}
	}
	if (d->drawJoints) overlay_joints(id,d,joints);
	if (d->drawContacts) overlay_contacts(world,id,d,contacts);
}
