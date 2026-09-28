#!/usr/bin/env python3
"""Compare every frame of diagnostic core-state traces; not a full-state gate."""
import argparse,gzip,json
from itertools import zip_longest
from pathlib import Path


def difference(a,b,path='$'):
    if type(a) is not type(b): return path,a,b
    if isinstance(a,dict):
        if a.keys()!=b.keys(): return path+'.keys',sorted(a),sorted(b)
        for k in sorted(a):
            d=difference(a[k],b[k],path+'.'+k)
            if d:return d
    elif isinstance(a,list):
        if len(a)!=len(b):return path+'.length',len(a),len(b)
        for i,(x,y) in enumerate(zip(a,b)):
            d=difference(x,y,f'{path}[{i}]')
            if d:return d
    elif a!=b:return path,a,b
    return None


def audited_storage_view(frame):
    """Normalize only audited membership storage; never mutate a raw trace.

    Occupied root order is atomic append order. Previous-touching storage is
    consumed as a union of pair identities. Generation/history/owner records,
    hash placement, solver order, public arrays and physical state stay exact.
    See docs/gpu-solver-qualification.md and the perturbation regressions.
    """
    if frame.get('schema') != 'gpu-core-state-v19':
        raise ValueError('audited storage comparison requires schema v19')
    key = lambda value: json.dumps(value, sort_keys=True, separators=(',', ':'))
    allocation = frame.get('contact_allocation')
    history = frame.get('event_history')
    if not isinstance(allocation, dict) or not isinstance(history, dict):
        raise ValueError('missing audited storage state')
    occupied = allocation.get('occupied_order')
    previous = history.get('previous_touching')
    if not isinstance(occupied, list) or not isinstance(previous, list):
        raise ValueError('invalid audited membership arrays')
    identities = [key(value) for value in occupied]
    if len(set(identities)) != len(identities):
        raise ValueError('duplicate occupied root identity')
    for value in occupied:
        if not isinstance(value, list) or len(value) != 2 or value[1] != 0:
            raise ValueError('occupied membership must contain root identities')
    unique_previous = {}
    for value in previous:
        if value is None:
            continue
        if not isinstance(value, list) or len(value) != 2 or any(
            not isinstance(endpoint, dict) or 'shape' not in endpoint or 'child' not in endpoint
            for endpoint in value
        ):
            raise ValueError('invalid previous-touching pair identity')
        unique_previous[key(value)] = value
    return {**frame,
        'contact_allocation': {**allocation, 'occupied_order': sorted(occupied, key=key)},
        'event_history': {**history, 'previous_touching': [unique_previous[k] for k in sorted(unique_previous)]}}


def compare(paths,frames,*,audited_storage=False):
    mode = "audited-membership-storage" if audited_storage else "raw"
    handles=[gzip.open(p, 'rt') if p.suffix == '.gz' else p.open() for p in paths]
    try:
        count=0
        for count,lines in enumerate(zip_longest(*handles),1):
            if any(line is None for line in lines):raise ValueError(f'frame {count}: truncated trace')
            docs=[json.loads(line) for line in lines]
            for d in docs:
                if d.get('schema') not in ['gpu-core-state-v1','gpu-core-state-v2','gpu-core-state-v3','gpu-core-state-v4','gpu-core-state-v5','gpu-core-state-v6','gpu-core-state-v7','gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19'] or d.get('frame')!=count:
                    raise ValueError(f'frame {count}: schema or step mismatch')
                for key in ['bodies','joints','contacts']:
                    if not isinstance(d.get(key),list):raise ValueError(f'frame {count}: missing {key}')
                if d['schema'] in ['gpu-core-state-v2','gpu-core-state-v3','gpu-core-state-v4','gpu-core-state-v5','gpu-core-state-v6','gpu-core-state-v7','gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    for key,kind in [('contact_history',dict),('contact_color_order',list),
                                     ('fat_bounds',list),('pending_transforms',list),('host_flags',dict)]:
                        if not isinstance(d.get(key),kind):
                            raise ValueError(f'frame {count}: missing or invalid {key}')
                if d['schema'] in ['gpu-core-state-v3','gpu-core-state-v4','gpu-core-state-v5','gpu-core-state-v6','gpu-core-state-v7','gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    for body in d['bodies']:
                        if not isinstance(body.get('gpu_motion'),dict):
                            raise ValueError(f'frame {count}: missing GPU body motion state')
                if d['schema'] in ['gpu-core-state-v4','gpu-core-state-v5','gpu-core-state-v6','gpu-core-state-v7','gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19'] and not isinstance(d.get('body_storage_order'),list):
                    raise ValueError(f'frame {count}: missing body processing order')
                if d['schema'] in ['gpu-core-state-v5','gpu-core-state-v6','gpu-core-state-v7','gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    for key,kind in [('shapes',list),('geometries',list),('shape_storage_order',list),('body_allocation',dict)]:
                        if not isinstance(d.get(key),kind):raise ValueError(f'frame {count}: missing {key}')
                    for shape in d['shapes']:
                        index=shape.get('geometry')
                        if type(index) is not int or not 0<=index<len(d['geometries']):
                            raise ValueError(f'frame {count}: invalid shape geometry reference')
                if d['schema'] in ['gpu-core-state-v6','gpu-core-state-v7','gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19'] and not isinstance(d.get('gpu_colliders'),list):
                    raise ValueError(f'frame {count}: missing GPU collider state')
                if d['schema'] in ['gpu-core-state-v7','gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    geometry=d.get('gpu_geometry')
                    if not isinstance(geometry,dict):raise ValueError(f'frame {count}: missing GPU geometry')
                    for key in ['hull_points','hull_planes','hull_edges','hull_topology','mesh_vertices','mesh_triangles','mesh_nodes','mix_table']:
                        if not isinstance(geometry.get(key),list):raise ValueError(f'frame {count}: missing GPU {key}')
                if d['schema'] in ['gpu-core-state-v8','gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    for collider in d['gpu_colliders']:
                        for key in ['bounds_shape','query_shape']:
                            if type(collider.get(key)) is not int:raise ValueError(f'frame {count}: missing GPU {key} reference')
                if d['schema'] in ['gpu-core-state-v9','gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19'] and not isinstance(d.get('idle_state'),dict):
                    raise ValueError(f'frame {count}: missing idle proof state')
                if d['schema'] in ['gpu-core-state-v10','gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19'] and not isinstance(d.get('graph_cache'),dict):
                    raise ValueError(f'frame {count}: missing graph cache state')
                if d['schema'] in ['gpu-core-state-v11','gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19'] and not isinstance(d.get('contact_allocation'),dict):
                    raise ValueError(f'frame {count}: missing contact allocation state')
                if d['schema'] in ['gpu-core-state-v12','gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    for key in ['contact_hash','event_history']:
                        if not isinstance(d.get(key),dict):raise ValueError(f'frame {count}: missing {key}')
                if d['schema'] in ['gpu-core-state-v13','gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    events=d.get('contact_end_state')
                    if not isinstance(events,dict):raise ValueError(f'frame {count}: missing contact end state')
                    for key in ['published','deferred','ended_keys']:
                        if not isinstance(events.get(key),list):raise ValueError(f'frame {count}: missing contact end {key}')
                if d['schema'] in ['gpu-core-state-v14','gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    contacts=d.get('host_contacts')
                    if not isinstance(contacts,dict):raise ValueError(f'frame {count}: missing host contacts')
                    for key in ['registry','live','by_body','by_shape','by_id','begins','ends','hits','deferred_ends']:
                        if not isinstance(contacts.get(key),list):raise ValueError(f'frame {count}: missing host contacts {key}')
                if d['schema'] in ['gpu-core-state-v15','gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    events=d.get('host_events')
                    if not isinstance(events,dict):raise ValueError(f'frame {count}: missing host events')
                    for key in ['sensor_overlaps','sensor_begins','sensor_ends','deferred_sensor_ends','continuous_sensor_hits','body_moves','joint_events']:
                        if not isinstance(events.get(key),list):raise ValueError(f'frame {count}: missing host events {key}')
                if d['schema'] in ['gpu-core-state-v16','gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    host=d.get('host_state')
                    if not isinstance(host,dict):raise ValueError(f'frame {count}: missing host state')
                    for key,kind in [('definition',dict),('step_start_bodies',list),('body_mirrors',list),('callbacks_present',dict)]:
                        if not isinstance(host.get(key),kind):raise ValueError(f'frame {count}: missing host {key}')
                    for key in ['cached_topology','scene_capabilities','query_index','gpu_ccd_cache_key']:
                        if key not in host:raise ValueError(f'frame {count}: missing host {key}')
                if d['schema'] in ['gpu-core-state-v17','gpu-core-state-v18','gpu-core-state-v19']:
                    policy=d.get('gpu_policy')
                    if not isinstance(policy,dict):raise ValueError(f'frame {count}: missing GPU policy')
                    for key,kind in [('parameters',dict),('capacities',dict),('body_sleep_thresholds',list),('fat_geometry',list)]:
                        if not isinstance(policy.get(key),kind):raise ValueError(f'frame {count}: missing GPU policy {key}')
                    if 'native' not in policy:raise ValueError(f'frame {count}: missing native GPU cache state')
                if d['schema'] in ['gpu-core-state-v18','gpu-core-state-v19']:
                    if not isinstance(d.get('adapter'),dict):raise ValueError(f'frame {count}: missing selected adapter')
                    if 'convex_ccd' not in d:raise ValueError(f'frame {count}: missing convex CCD state')
                    ccd=d['convex_ccd']
                    if ccd is not None:
                        if not isinstance(ccd,dict):raise ValueError(f'frame {count}: invalid convex CCD state')
                        for key in ['config','points','bodies','shapes','indices','start']:
                            if not isinstance(ccd.get(key),list):raise ValueError(f'frame {count}: missing CCD {key}')
                if d['schema']=='gpu-core-state-v19':
                    if not isinstance(d['body_allocation'].get('generations'),list):raise ValueError(f'frame {count}: missing body generations')
                    for contact in d['contacts']:
                        if not isinstance(contact.get('sat_cache_words'),list) or len(contact['sat_cache_words'])!=2:
                            raise ValueError(f'frame {count}: missing contact SAT cache')
                    for joint in d['joints']:
                        if 'motor_spring_angular_impulse' not in joint:raise ValueError(f'frame {count}: missing motor spring state')
            if audited_storage:
                docs = [audited_storage_view(d) for d in docs]
            for path,d in zip(paths[1:],docs[1:]):
                delta=difference(docs[0],d)
                if delta:return {'status':'fail','frame':count,'file':str(path),'difference':delta,'comparison_mode':mode}
        if count!=frames:raise ValueError(f'expected {frames} frames, got {count}')
        return {'status':'pass','frames':count,'runs':len(paths),'comparison_mode':mode,'scope':'diagnostic state only; coverage depends on schema and documented omissions remain unqualified'}
    finally:
        for f in handles:f.close()


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--frames',type=int,required=True);p.add_argument('traces',type=Path,nargs='+');p.add_argument('--audited-storage',action='store_true',help='canonicalize only audited occupied/history membership storage; all other fields remain exact');a=p.parse_args()
    if len(a.traces)<2 or a.frames<1:p.error('need at least two traces and positive frame count')
    result=compare(a.traces,a.frames,audited_storage=a.audited_storage);print(json.dumps(result,indent=2));raise SystemExit(result['status']!='pass')
