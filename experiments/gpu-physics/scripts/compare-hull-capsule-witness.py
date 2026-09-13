#!/usr/bin/env python3
"""Compare exact contact geometry, with an explicit symmetry rule for the centered core."""
import hashlib, json, math, pathlib, sys
p=pathlib.Path(sys.argv[1]); root=pathlib.Path(__file__).resolve().parent.parent
cpu=[json.loads(s) for s in (p/'cpu.jsonl').read_text().splitlines()]
gpu=[json.loads(s) for s in (p/'gpu.jsonl').read_text().splitlines()]
assert [x['case'] for x in cpu]==list(range(7))==[x['case'] for x in gpu]
worst=0.0
for c,g in zip(cpu,gpu):
    assert len(c['points'])==len(g['points']), (c,g)
    cp=sorted(c['points'],key=lambda x:x['p']);gp=sorted(g['points'],key=lambda x:x['p'])
    for a,b in zip(cp,gp):
        values=b['p']+b['n']+[b['s']];assert all(math.isfinite(v) for v in values)
        if c['case']==6:
            # Centered segment inside a cube has equally valid +Y/-Y escape faces.
            # Require the same depth, X positions, surface midpoint and unit axis.
            assert abs(b['s']-a['s'])<1e-5
            assert abs(b['p'][0]-a['p'][0])<1e-5 and abs(b['p'][2])<1e-5
            assert abs(abs(b['n'][1])-1)<1e-5 and abs(b['n'][0])+abs(b['n'][2])<1e-5
            assert abs(b['p'][1]-.475*b['n'][1])<1e-5
            continue
        delta=max(abs(x-y) for x,y in zip(a['p']+a['n']+[a['s']],values))
        worst=max(worst,delta);assert delta<1e-5,(c['case'],delta,a,b)
files=['c_abi/hull_capsule_witness_reference.cpp','shaders/physics/collide.wgsl','target/release/libgpu_physics.a','native-samples/build-gpu/libgpu_samples_api.a','oracle/build/box3d-build/src/libbox3d.a']
result={'status':'pass','cases':7,'max_unique_geometry_delta':worst,'core_rule':'symmetric +/-Y escape face allowed; equal depth and face points required','sha256':{f:hashlib.sha256((root/f).read_bytes()).hexdigest() for f in files}}
(p/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
