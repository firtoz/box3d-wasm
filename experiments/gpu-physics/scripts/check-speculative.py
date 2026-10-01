#!/usr/bin/env python3
"""Focused PR01 controls; separate from timing. Never overwrite a result directory."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path, value):
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(value, indent=2) + '\n')
    temporary.replace(path)


def logged(command, path, env=None):
    with path.open('w') as log:
        result = subprocess.run(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
    save(path.with_suffix('.receipt.json'), {'command': command, 'exit': result.returncode,
                                            'log_sha256': sha(path)})
    if result.returncode:
        raise SystemExit(f'{path}: exit {result.returncode}')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--stage', choices=['baseline', 'cpu', 'candidate'], required=True)
    p.add_argument('--backend', choices=['ordinary', 'native'], default='ordinary')
    p.add_argument('--linkage', choices=['gpu', 'both'], default='gpu')
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--library', type=Path, help='Frozen candidate Rust library')
    p.add_argument('--build-receipt', type=Path, help='Receipt for the compiled candidate library')
    p.add_argument('--cpu-ccd-on', action='store_true', help='Independent native default-on CCD reference; preserve recorded shape-off failure')
    a = p.parse_args()
    out = a.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    label = a.stage if a.stage != 'candidate' else f'{a.backend}-{a.linkage}'
    result_dir = out / label
    result_dir.mkdir()  # Preserve every prior result, including failures.
    env = os.environ.copy()
    env.update(WGPU_BACKEND='vulkan', GPU_PHYSICS_BACKEND='vulkan', GPU_PHYSICS_ADAPTER='nvidia',
               GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out / 'pipelines'))
    if a.stage == 'candidate' and a.backend == 'native':
        policy = subprocess.check_output(['bash', '-c', 'source scripts/native-samples-cache-env.sh; env -0'], cwd=ROOT)
        for item in policy.split(b'\0'):
            if item.startswith(b'GPU_PHYSICS_') or item.startswith(b'WGPU_'):
                key, value = item.decode().split('=', 1)
                env[key] = value
    lib = None
    extra = []
    cpu = []
    if a.stage == 'baseline':
        build = out / 'baseline-cmake'
        lib = out / 'baseline-ordinary.a'
    elif a.stage == 'cpu':
        build = out / 'cpu-cmake'
        extra = ['-DCPU_REFERENCE']
    else:
        live_lib = a.library.resolve() if a.library else ROOT / ('target/samples/release/libgpu_physics.a' if a.backend == 'ordinary'
                           else 'target/native-cache-build/release/libgpu_physics.a')
        if a.build_receipt:
            build_receipt = json.loads(a.build_receipt.read_text())
            if sha(live_lib) != build_receipt['binary_sha256']:
                raise SystemExit('Frozen library does not match its build receipt')
            for name, expected in build_receipt['engine_sources'].items():
                if sha(ROOT / name) != expected:
                    raise SystemExit('Compiled engine source changed: ' + name)
        lib = result_dir / 'libgpu_physics.a'
        lib.write_bytes(live_lib.read_bytes())
        build = result_dir / 'cmake'
        logged(['cmake', '-S', str(ROOT / 'native-samples'), '-B', str(build),
                '-DGPU_PHYSICS_LIB=' + str(lib), '-DGPU_PORTABLE_API=ON',
                '-DGPU_SAMPLES=' + ('ON' if a.linkage == 'gpu' else 'OFF'),
                '-DBOTH_SAMPLES=' + ('ON' if a.linkage == 'both' else 'OFF'),
                '-DCMAKE_BUILD_TYPE=Release'], result_dir / 'configure.log')
        targets = ['gpu_samples_api', 'box3d']
        if a.linkage == 'both':
            targets.insert(0, 'gpu_both_api')
            extra = ['-DGPU_API_DUAL', '-Wl,--wrap=cpu_b3World_EnableSpeculative',
                     '-Wl,--whole-archive', str(build / 'libgpu_both_api.a'), '-Wl,--no-whole-archive']
            cpu = [str(build / 'libbox3d_cpu.a')]
        logged(['cmake', '--build', str(build), '--target', *targets, '-j4'], result_dir / 'build.log')
    exe = result_dir / 'fixture'
    command = ['g++', '-O2', '-std=c++17', str(ROOT / 'c_abi/speculative_fixture.cpp'),
               '-I', str(ROOT.parents[1] / 'box3d/include'), *extra]
    inputs = [ROOT / 'c_abi/speculative_fixture.cpp', build / 'box3d_src/libbox3d.a']
    if lib:
        command += ['-Wl,--whole-archive', str(build / 'libgpu_samples_api.a'), '-Wl,--no-whole-archive', str(lib), *cpu]
        inputs += [build / 'libgpu_samples_api.a', lib, *map(Path, cpu)]
    command += [str(build / 'box3d_src/libbox3d.a'), '-ldl', '-lpthread', '-lm', '-lgcc_s', '-lGL', '-o', str(exe)]
    logged(command, result_dir / 'link.log')
    sources = [path for directory in ['src', 'shaders', 'c_abi', 'native-samples', 'scripts']
               for path in (ROOT / directory).rglob('*')
               if path.is_file() and not any(part.startswith('build-') or part == '__pycache__' for part in path.parts)
               and path.suffix in ('.rs', '.wgsl', '.c', '.cpp', '.h', '.py', '.sh', '.txt')]
    sources += [ROOT / name for name in ['Cargo.toml', 'Cargo.lock', 'build.rs']]
    receipt = {'stage': a.stage, 'backend': a.backend, 'linkage': a.linkage,
               'revision_at_invocation': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
               'executable_sha256': sha(exe), 'linked_inputs': {str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path): sha(path) for path in inputs},
               'current_source_hashes': {str(path.relative_to(ROOT)): sha(path) for path in sorted(sources)},
               'environment': {key: value for key, value in sorted(env.items()) if key.startswith(('GPU_PHYSICS_', 'WGPU_'))},
               'toolchain': subprocess.check_output(['rustc', '-Vv'], text=True),
               'adapter_driver': subprocess.check_output(['nvidia-smi', '--query-gpu=name,driver_version', '--format=csv,noheader'], text=True).strip(),
               'runs': []}
    if a.build_receipt:
        receipt['library_build_receipt_sha256'] = sha(a.build_receipt)
    # Baseline engine identity comes from its frozen build receipt, not current sources.
    if a.stage == 'baseline':
        receipt['baseline_engine_build_receipt_sha256'] = sha(out / 'baseline-ordinary-build.json')
        receipt['baseline_adapter_revision'] = 'bd4f7966151028542297be81a330e80e83687531'
    save(result_dir / 'receipt.json', receipt)
    expected = 1 if a.stage != 'candidate' else 5
    for trial in range(1, expected + 1):
        command = [str(exe)] + (['reproduce'] if a.stage == 'baseline' else ['ccd-on'] if a.cpu_ccd_on else [])
        start = time.monotonic()
        stdout = result_dir / f'trial-{trial}.stdout'
        stderr = result_dir / f'trial-{trial}.stderr'
        with stdout.open('w') as stream, stderr.open('w') as errors:
            proc = subprocess.Popen(command, cwd=ROOT, env=env, stdout=stream, stderr=errors)
            receipt['running'] = {'pid': proc.pid, 'trial': trial, 'command': command}
            save(result_dir / 'receipt.json', receipt)
            code = proc.wait()
        receipt.pop('running', None)
        receipt['runs'].append({'trial': trial, 'exit': code, 'elapsed_seconds': time.monotonic() - start,
                                'stdout_sha256': sha(stdout), 'stderr_sha256': sha(stderr)})
        save(result_dir / 'receipt.json', receipt)
        print(label, trial, code, flush=True)
        if a.stage == 'baseline':
            if code == 0 or 'world-off-reproducer contacts=1' not in stdout.read_text() or 'Assertion' not in stderr.read_text():
                raise SystemExit('Baseline did not reproduce the intended unchanged contact count')
        elif code != 0:
            raise SystemExit(f'Failed fixture retained in {result_dir}')
    if a.stage == 'candidate':
        hashes = {run['stdout_sha256'] for run in receipt['runs']}
        if len(hashes) != 1:
            raise SystemExit('Public-result repeat mismatch; full-state qualification remains separate PR07')
    receipt['result'] = 'expected-baseline-failure' if a.stage == 'baseline' else 'pass'
    save(result_dir / 'receipt.json', receipt)


if __name__ == '__main__':
    main()
