from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import tarfile
import difflib

REPO = Path('/home/firtoz/work/2026/box3d-wasm')
ENGINE = REPO / 'experiments/gpu-physics'
A = ENGINE / 'artifacts/production-readiness/pr04-native-ccd-boundary'
OLD = ENGINE / 'artifacts/production-readiness/pr04-drag-ccd-boundary'
W = A / 'workspace/experiments/gpu-physics'
assert not (A / 'protocol.json').exists()
W.mkdir(parents=True)
sha = lambda f: hashlib.sha256(Path(f).read_bytes()).hexdigest()
old = json.loads((OLD / 'protocol.json').read_text())
original = {n: sha(ENGINE / n) for n in old['compiled_Rust_inputs_current']}
assert original == old['compiled_Rust_inputs_current']
for n in original:
    dest = W / n
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(ENGINE / n, dest)
# Same isolated manifest/pinned lock as the prior native compiler provider.
template = ENGINE / 'target/native-workspace/experiments/gpu-physics'
for name in ['Cargo.toml', 'Cargo.lock']:
    shutil.copyfile(template / name, W / name)
(A / 'workspace/box3d').symlink_to(REPO / 'box3d', target_is_directory=True)
(W / 'c_abi').symlink_to(ENGINE / 'c_abi', target_is_directory=True)

# Additional writes touch a dedicated observer buffer only. Preserve original
# classification, sweep, interpolation and body writes verbatim.
f = W / 'shaders/physics/ccd_world.wgsl'
s = f.read_text()
anchor = '@group(0) @binding(6) var<uniform> ccd_config:CcdConfig;'
assert s.count(anchor) == 1
s = s.replace(anchor, anchor + '''
@group(0) @binding(7) var<storage,read_write> ccd_boundary:array<u32>;
fn boundary_begin(body:u32,end:CcdState,flags:u32) {
    let off=32u*body;
    let begin=ccd_start[body];
    ccd_boundary[off]=body; ccd_boundary[off+1u]=flags;
    ccd_boundary[off+2u]=1u; ccd_boundary[off+3u]=bitcast<u32>(1.0);
    ccd_boundary[off+4u]=0u; ccd_boundary[off+5u]=0u;
    ccd_boundary[off+6u]=0u; ccd_boundary[off+7u]=0u;
    for (var i=0u;i<4u;i++) {
        ccd_boundary[off+8u+i]=bitcast<u32>(begin.p[i]);
        ccd_boundary[off+12u+i]=bitcast<u32>(end.p[i]);
        ccd_boundary[off+16u+i]=bitcast<u32>(end.p[i]);
        ccd_boundary[off+20u+i]=bitcast<u32>(begin.q[i]);
        ccd_boundary[off+24u+i]=bitcast<u32>(end.q[i]);
        ccd_boundary[off+28u+i]=bitcast<u32>(end.q[i]);
    }
}
fn boundary_finish(body:u32) {
    let off=32u*body;
    for (var i=0u;i<4u;i++) {
        ccd_boundary[off+16u+i]=bitcast<u32>(ccd_finish[body].p[i]);
        ccd_boundary[off+28u+i]=bitcast<u32>(ccd_finish[body].q[i]);
    }
}
''')
anchor = '    let end=ccd_finish[body]; let flags=bitcast<u32>(end.v.w);'
assert s.count(anchor) == 1
s = s.replace(anchor, anchor + '\n    boundary_begin(body,end,flags);')
anchor = '    if (motion<=min(0.5*body_info.min_extent,0.02)) { return; }'
assert s.count(anchor) == 1
s = s.replace(anchor, '''    ccd_boundary[32u*body+4u]=bitcast<u32>(motion);
    ccd_boundary[32u*body+5u]=bitcast<u32>(min(0.5*body_info.min_extent,0.02));
    ccd_boundary[32u*body+2u]=2u;
''' + anchor + '\n    ccd_boundary[32u*body+2u]=3u;')
anchor = '    if (fraction<1.0) {'
assert s.count(anchor) == 1
s = s.replace(anchor, '    ccd_boundary[32u*body+3u]=bitcast<u32>(fraction);\n' + anchor + '\n        ccd_boundary[32u*body+2u]=4u;')
assert s.endswith('    }\n}\n')
s = s[:-2] + '    boundary_finish(body);\n}\n'
f.write_text(s)

f = W / 'src/ccd.rs'
s = f.read_text()
anchor = '    pub(crate) start:wgpu::Buffer, binding:wgpu::BindGroup, pipeline:wgpu::ComputePipeline, count:u32,'
assert s.count(anchor) == 1
s = s.replace(anchor, anchor + '\n    pub(crate) boundary:wgpu::Buffer,')
anchor = '        let buffers=[&start,bodies,&points,&shapes,&metadata,&indices,&config];'
assert s.count(anchor) == 1
s = s.replace(anchor, '''        let boundary=device.create_buffer(&wgpu::BufferDescriptor {
            label:Some("artifact-native-ccd-boundary"),size:u64::from(count)*128,
            usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_SRC,
            mapped_at_creation:false,
        });
        let buffers=[&start,bodies,&points,&shapes,&metadata,&indices,&config,&boundary];''')
anchor = '        Self{start,binding,pipeline,count,'
assert s.count(anchor) == 1
s = s.replace(anchor, '        Self{start,binding,pipeline,count,boundary,')
f.write_text(s)

f = W / 'src/sim.rs'
s = f.read_text()
anchor = '''        self.completed_step = self.physics_step;
        self.completed_known = true;
    }

    pub fn poll_completion'''
assert s.count(anchor) == 1
s = s.replace(anchor, '''        self.completed_step = self.physics_step;
        self.completed_known = true;
        #[cfg(all(feature="replay-diagnostics",not(target_arch="wasm32")))]
        if (221..=233).contains(&self.physics_step)
            && std::env::var("GPU_PHYSICS_NATIVE_CCD_BOUNDARY").as_deref()==Ok("1") {
            let body=1u32;
            let buffer=self.convex_ccd.as_ref().map(|ccd|ccd.boundary.clone());
            if let Some(buffer)=buffer {
                let records=self.read_diagnostic_records::<[u32;32]>(&buffer,body*32,1);
                eprintln!("gpu-native-ccd-boundary {}",serde_json::json!({
                    "step":self.physics_step,"body_slot":body,"convex_ccd_present":true,
                    "joint_count":self.params.joint_count,"diagnostic_flags":self.params.diagnostic_flags,
                    "solver_mode":self.params.solver_mode,"words":records[0]}));
            } else {
                eprintln!("gpu-native-ccd-boundary {}",serde_json::json!({
                    "step":self.physics_step,"body_slot":body,"convex_ccd_present":false}));
            }
        }
    }

    pub fn poll_completion''')
f.write_text(s)

changed = ['src/ccd.rs', 'src/sim.rs', 'shaders/physics/ccd_world.wgsl']
for name in changed:
    dest = A / 'original' / name
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(ENGINE / name, dest)
    diff = ''.join(difflib.unified_diff((ENGINE/name).read_text().splitlines(True), (W/name).read_text().splitlines(True), fromfile='original/'+name, tofile='observer/'+name))
    (A / (Path(name).stem + '.patch')).write_text(diff)
compiled = {n: sha(W/n) for n in original}
with tarfile.open(A/'observer-source-inputs.tar.gz','w:gz') as tar:
    for n in compiled:
        tar.add(W/n,arcname=n,recursive=False)

references = {str(OLD/n): sha(OLD/n) for n in ['protocol.json','receipt.json','drag-prefix.cpp','libbox3d_cpu_observer.a']}
native = {}
for f in (ENGINE/'target/native-backend').rglob('*'):
    if f.is_file() and not f.is_symlink() and f.suffix in ['.rs','.toml','.lock','.py','.patch']:
        native[str(f.resolve())] = sha(f)
assert len(native)>300
with tarfile.open(A/'native-backend-inputs.tar.gz','w:gz') as tar:
    for n in native:
        tar.add(n,arcname=n.split('/target/native-backend/',1)[1],recursive=False)

link = next(x for x in old['links'] if x['name']=='native')
link_command = list(link['command'])
link_command[link_command.index(str(ENGINE/'artifacts/production-readiness/pr04-distance-clamp/native-candidate.a'))] = str(A/'native-observer.a')
link_command[-1] = str(A/'native-drag-prefix')
reused = {n:h for n,h in link['reused_inputs'].items() if not n.endswith('/native-candidate.a')}
reused[str(OLD/'libbox3d_cpu_observer.a')] = sha(OLD/'libbox3d_cpu_observer.a')
case = dict(next(x for x in old['cases'] if x['name']=='native'))
case['command'] = [str(A/'native-drag-prefix'),'--ground']
case['environment_overrides'] = dict(case['environment_overrides'])
case['environment_overrides'].update(
    GPU_PHYSICS_NATIVE_CCD_BOUNDARY='1',
    GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'native-pipelines'),
    BOTH_DRAG_TRACE=str(A/'native/comparison.txt'))
build = ['cargo','--config',str(ENGINE/'target/native-backend/cargo-config.toml'),'build',
         '--manifest-path',str(W/'Cargo.toml'),'--target-dir',str(ENGINE/'target/native-cache-build'),
         '--release','--locked','--lib','--features','native-command-cache,external-c-shim,replay-diagnostics','--message-format=json']
C = {}
for base in [REPO/'box3d/src',REPO/'box3d/include']:
    for f in base.rglob('*'):
        if f.is_file() and f.suffix in ['.c','.h']: C[str(f.resolve())]=sha(f)
protocol = dict(
    purpose='Artifact-only native GPU CCD boundary observer; exact original native240step prefix equality required. No physics candidate or full dragging/performance acceptance.',
    budget=dict(native_Rust_library_builds=1, fixture_CPP_compilations_and_links=1,
                fresh_native_diagnostic_processes=1, completed_steps=240, observer_sources=3,
                CPU_solver_builds=0, production_candidates=0, retries=0, headline_timing=0),
    original_Rust_inputs=original, observer_Rust_inputs=compiled, changed_observer_sources=changed,
    workspace=str(W), native_backend_inputs=native, C_engine_build_inputs=C,
    references=references, parent=str(OLD/'protocol.json'), parent_sha256=sha(OLD/'protocol.json'),
    reused_link_inputs=reused, build_command=build, link_command=link_command, case=case,
    build_watchdog_seconds=900, link_watchdog_seconds=120, process_watchdog_seconds=300,
    boundary_record='32u32 per original GPU body slot: body,flags,branch,fraction,motion,cutoff,2unused; vec4 start/preend/post position, start/preend/post quaternion. Branch1 flags-skipped,2motion-below,3swept-nohit,4corrected. Selected slot1/steps221..233. Reads after wait_completion; duplicates retained and must agree.',
    criteria='Actual NVIDIA4070SUPER Vulkan, completed original240step prefix, every B/F/M/P line exactly matches original printed9sig baseline, actual convex presence and13 finite selected GPU boundary records; original CPUclass prefix remains unchanged. No final physics pass.',
    stop_retain='Stop on first build/link/adapter/watchdog/neutrality/coverage failure, retain outputs/unlaunched cells. No repeats, original sources/defaults/inputs/archives never edited. Preserve prior budgets and all full dragging limits.',
    invocation_revision_context_only=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip())
(A/'protocol.json').write_text(json.dumps(protocol,indent=2)+'\n')
print('Frozen protocol',sha(A/'protocol.json'),'workspace',len(compiled),'native backend',len(native),'C',len(C))
