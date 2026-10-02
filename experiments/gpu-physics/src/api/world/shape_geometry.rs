//! In-place geometry changes. Body mass properties remain explicitly controlled
//! by ApplyMassFromShapes, even though shape mass data changes immediately.
use super::*;

struct GeometryMass {
    mass: f32,
    inertia: [f32; 9],
}

fn replace(id: ShapeId, edit: impl FnOnce(&mut CpuShape) -> bool) -> bool {
    if id.index1 <= 0 {
        return false;
    }
    let world = world_id_from_shape(id);
    // Upstream refuses geometry edits from locked callbacks.
    let mut worlds = match WORLDS.try_lock() {
        Ok(worlds) => worlds,
        Err(std::sync::TryLockError::Poisoned(error)) => error.into_inner(),
        Err(std::sync::TryLockError::WouldBlock) => return false,
    };
    let Some(w) = slot_mut(&mut worlds, world) else {
        return false;
    };
    ensure_cpu_mirror_world(w, world);
    {
        let index = id.index1 as usize - 1;
        let Some(shape) = w
            .shapes
            .get_mut(index)
            .and_then(Option::as_mut)
            .filter(|s| s.generation == id.generation)
        else {
            return false;
        };
        let compound = shape.public_kind == PUBLIC_KIND_COMPOUND;
        if !edit(shape) {
            return false;
        }
        shape.mesh_vertices = Arc::default();
        shape.mesh_triangles = Arc::default();
        shape.mesh_triangle_ids = Arc::default();
        shape.mesh_nodes = Arc::default();
        shape.mesh_instance = None;
        shape.mesh_scale = [1.0; 3];
        if let Some(sim) = w.sim.as_mut() {
            sim.invalidate_idle_proof();
            sim.retire_shape_contacts(index as u32, id.generation);
        }
        if compound {
            for (child_index, slot) in w.shapes.iter_mut().enumerate() {
                if slot
                    .as_ref()
                    .is_some_and(|s| s.compound_parent == id.index1)
                {
                    let child = slot.take().unwrap();
                    if let Some(sim) = w.sim.as_mut() {
                        sim.retire_shape_contacts(child_index as u32, child.generation);
                    }
                }
            }
        }
        // Only touching contacts wake their two owners, matching ResetProxy.
        let mut wake = Vec::new();
        w.live_contacts.retain(|key, c| {
            let remove = c.shape_id_a == id || c.shape_id_b == id;
            if remove {
                for sid in [c.shape_id_a, c.shape_id_b] {
                    if let Some(s) = w
                        .shapes
                        .get(sid.index1 as usize - 1)
                        .and_then(Option::as_ref)
                    {
                        wake.push(s.body_index);
                    }
                }
                if c.events_enabled {
                    w.deferred_contact_end_events.push((*key, ContactEndTouchEvent {
                        shape_id_a: c.shape_id_a,
                        shape_id_b: c.shape_id_b,
                        contact_id: c.contact_id,
                    }));
                }
            }
            !remove
        });
        w.contact_registry
            .retain(|_, e| e.live.shape_id_a != id && e.live.shape_id_b != id);
        w.contact_snapshot_key = None;
        // Native WakeBody wakes the sleeping solver set, including other
        // bodies connected through contacts or joints. Static supports do not
        // merge the independent islands resting on them.
        let islands: Vec<u32> = wake
            .iter()
            .filter_map(|&index1| {
                let body = w.bodies.get(index1 as usize - 1)?.as_ref()?;
                (body.gpu.flags & (FLAG_STATIC | FLAG_DISABLED) == 0
                    && body.gpu.island_id != u32::MAX)
                    .then_some(body.gpu.island_id)
            })
            .collect();
        let epoch = snapshot_epoch(w).saturating_add(1);
        for (index, slot) in w.bodies.iter_mut().enumerate() {
            if let Some(body) = slot {
                if body.gpu.flags & (FLAG_STATIC | FLAG_DISABLED) == 0
                    && (wake.contains(&(index as i32 + 1)) || islands.contains(&body.gpu.island_id))
                {
                    wake_body(body);
                    body.host_epoch = epoch;
                }
            }
        }
        mark_scene_dirty(w);
        // Native setters leave body CCD extents with the stored mass data;
        // ApplyMassFromShapes recomputes them around the new center of mass.
        true
    }
}

fn primitive(
    s: &mut CpuShape,
    kind: u32,
    half: [f32; 3],
    center: [f32; 3],
    axis: [f32; 3],
    unit: GeometryMass,
    mass: GeometryMass,
) {
    s.kind = kind;
    s.public_kind = kind;
    s.half = half;
    s.axis = axis;
    s.local_center = center;
    s.geometry_center = center;
    s.inner_radius = half[0];
    s.unit_mass = unit.mass;
    s.unit_inertia = unit.inertia;
    s.mass = mass.mass;
    s.local_inertia = mass.inertia;
    s.hull_points.clear();
    s.hull_planes.clear();
    s.hull_edges.clear();
    s.hull_topology.clear();
}

pub fn b3_shape_set_sphere(id: ShapeId, sphere: &Sphere) -> bool {
    replace(id, |s| {
        let r = sphere.radius;
        let data = |density| {
            let m = sphere_mass(r, density);
            let i = 0.4 * m * r * r;
            GeometryMass {
                mass: m,
                inertia: [i, i, i, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            }
        };
        primitive(
            s,
            KIND_SPHERE,
            [r; 3],
            sphere.center,
            [0.0; 3],
            data(1.0),
            data(s.density),
        );
        true
    })
}

pub fn b3_shape_set_capsule(id: ShapeId, capsule: &Capsule) -> bool {
    replace(id, |s| {
        let delta = std::array::from_fn::<_, 3, _>(|i| capsule.center2[i] - capsule.center1[i]);
        let half_length =
            0.5 * (delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]).sqrt();
        let unit = crate::types::compute_capsule_mass(
            capsule.center1,
            capsule.center2,
            capsule.radius,
            1.0,
        );
        let mass = crate::types::compute_capsule_mass(
            capsule.center1,
            capsule.center2,
            capsule.radius,
            s.density,
        );
        primitive(
            s,
            KIND_CAPSULE,
            [capsule.radius, half_length, capsule.radius],
            mass.center,
            delta.map(|v| 0.5 * v),
            GeometryMass {
                mass: unit.mass,
                inertia: pack_inertia(unit.full_inertia),
            },
            GeometryMass {
                mass: mass.mass,
                inertia: pack_inertia(mass.full_inertia),
            },
        );
        s.hull_points = vec![capsule.center1, capsule.center2];
        true
    })
}

pub fn b3_shape_set_hull(id: ShapeId, hull: &BoxHull) -> bool {
    set_box_hull_geometry(id, hull, false)
}

pub(crate) fn set_box_hull_geometry(id: ShapeId, hull: &BoxHull, force: bool) -> bool {
    replace(id, |s| {
        if !force
            && s.kind == KIND_BOX
            && s.half == hull.half_extents
            && s.geometry_center == hull.center
        {
            return false;
        }
        let data = |density| GeometryMass {
            mass: box_mass(hull.half_extents, density),
            inertia: expand_inertia(box_central_inertia(hull.half_extents, density)),
        };
        primitive(
            s,
            KIND_BOX,
            hull.half_extents,
            hull.center,
            [0.0; 3],
            data(1.0),
            data(s.density),
        );
        s.inner_radius = 0.0;
        true
    })
}

pub fn b3_shape_set_convex_hull(id: ShapeId, hull: &ConvexHull) -> bool {
    set_convex_hull_geometry(id, hull, false)
}

pub(crate) fn set_convex_hull_geometry(id: ShapeId, hull: &ConvexHull, force: bool) -> bool {
    replace(id, |s| {
        if !force
            && s.kind == KIND_CONVEX_HULL
            && s.hull_points == hull.points
            && s.hull_planes == hull.planes
            && s.hull_topology == hull.half_edges
            && s.hull_edges == hull.edge_directions
            && s.half == hull.half_extents
            && s.local_center == hull.center
            && s.geometry_center == hull.aabb_center
            && s.inner_radius == hull.inner_radius
            && s.unit_mass == hull.volume
            && s.unit_inertia == expand_inertia(hull.central_inertia)
        {
            return false;
        }
        s.kind = KIND_CONVEX_HULL;
        s.public_kind = KIND_CONVEX_HULL;
        s.half = hull.half_extents;
        s.axis = [0.0; 3];
        s.local_center = hull.center;
        s.geometry_center = hull.aabb_center;
        s.inner_radius = hull.inner_radius;
        s.unit_mass = hull.volume;
        s.unit_inertia = expand_inertia(hull.central_inertia);
        s.mass = hull.volume * s.density;
        s.local_inertia = expand_inertia(hull.central_inertia.map(|v| v * s.density));
        s.hull_points.clone_from(&hull.points);
        s.hull_planes.clone_from(&hull.planes);
        s.hull_edges.clone_from(&hull.edge_directions);
        s.hull_topology.clone_from(&hull.half_edges);
        true
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::api::*;

    #[test]
    fn replacement_reuses_storage_and_preserves_body_mass_and_metadata() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu, &b3_default_world_def());
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.position = [0.0, 10.0, 0.0];
        let body = b3_create_body(world, &bd);
        let mut sd = b3_default_shape_def();
        sd.user_data = 789;
        sd.enable_contact_events = true;
        let shape = b3_create_hull_shape(body, &sd, &b3_make_box_hull(0.5, 0.5, 0.5));
        let mass = b3_body_get_mass_data(body);
        let extents = with_world(world, |w| {
            let b = body_ref(w, body).unwrap();
            (b.min_extent, b.max_extent)
        });
        for index in 0..300 {
            match index % 3 {
                0 => assert!(b3_shape_set_sphere(
                    shape,
                    &Sphere {
                        center: [0.3, 0.0, 0.0],
                        radius: 0.7
                    }
                )),
                1 => assert!(b3_shape_set_capsule(
                    shape,
                    &Capsule {
                        center1: [0.0, -0.4, 0.0],
                        center2: [0.0, 0.4, 0.0],
                        radius: 0.2,
                    }
                )),
                _ => assert!(b3_shape_set_hull(shape, &b3_make_box_hull(0.6, 0.3, 0.2))),
            }
            let current = b3_body_get_mass_data(body);
            assert_eq!(mass.mass, current.mass);
            assert_eq!(
                extents,
                with_world(world, |w| {
                    let b = body_ref(w, body).unwrap();
                    (b.min_extent, b.max_extent)
                })
            );
            assert_eq!(mass.center, current.center);
            assert_eq!(mass.inertia, current.inertia);
            assert_eq!(b3_shape_get_user_data(shape), 789);
            assert!(b3_shape_are_contact_events_enabled(shape));
            assert_eq!(with_world(world, |w| w.shapes.len()), Some(1));
        }
        // A pending step must be completed before replacing its uploaded shape.
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        assert!(b3_shape_set_sphere(
            shape,
            &Sphere {
                center: [0.0; 3],
                radius: 0.2
            }
        ));
        b3_body_apply_mass_from_shapes(body);
        assert!(b3_body_get_mass_data(body).mass < mass.mass);
        b3_destroy_shape(shape, false);
        assert!(!b3_shape_set_sphere(
            shape,
            &Sphere {
                center: [0.0; 3],
                radius: 2.0
            }
        ));
        b3_destroy_world(world);
    }
}
