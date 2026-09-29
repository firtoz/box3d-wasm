#!/usr/bin/env python3
"""Run frozen geometry inputs through prebuilt CPU/GPU rain_frozen_contact probes."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cpu-binary', type=Path, required=True)
    parser.add_argument('--gpu-binary', type=Path, required=True)
    parser.add_argument('--configuration', choices=['ordinary', 'native-cached'], required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    inputs = root / 'c_abi/fixtures/frozen-mesh'
    manifest = json.loads((inputs / 'manifest.json').read_text())
    for name, expected in manifest['input_sha256'].items():
        if digest(inputs / name) != expected:
            raise ValueError(f'Frozen input changed: {name}')
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location('frozen_compare', root / 'scripts/compare-frozen-contacts.py')
    comparator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(comparator)
    clean = {k: v for k, v in os.environ.items()
             if not k.startswith(('GPU_PHYSICS_', 'WGPU_', 'RAIN_'))}
    gpu_env = clean.copy()
    if args.configuration == 'native-cached':
        raw = subprocess.check_output(['bash', '-c', 'source scripts/native-samples-cache-env.sh; env -0'],
                                      cwd=root, env=gpu_env)
        gpu_env.update(item.decode().split('=', 1) for item in raw.split(b'\0') if b'=' in item)
    gpu_env.update(GPU_PHYSICS_LIVE_CONTACT_ORDER='0',
                   GPU_PHYSICS_PIPELINE_CACHE_DIR=str(Path.home() / '.cache/box3d-gpu-physics/pipelines'))
    record = {'configuration': args.configuration, 'input_sha256': manifest['input_sha256'],
              'runner_sha256': digest(Path(__file__)), 'binaries': {}, 'results': {},
              'scope': 'Frozen-pose physical manifold regression; not dynamic-scene or repeat qualification.'}
    for engine, source, env in [('cpu', args.cpu_binary, clean), ('gpu', args.gpu_binary, gpu_env)]:
        binary = out / (engine + '-fixture')
        shutil.copy2(source.resolve(), binary)
        record['binaries'][engine] = {'sha256': digest(binary),
            'environment': {k: v for k, v in env.items() if k.startswith(('GPU_PHYSICS_', 'WGPU_'))}}
        save(out / 'manifest.json', record)
        for name, count in [('grid', 45), ('torus', 12)]:
            run_env = env.copy()
            if name == 'grid':
                run_env['RAIN_FROZEN_GRID'] = '1'
            data = out / f'{engine}-{name}.txt'
            with (inputs / f'{name}.txt').open() as inp, data.open('w') as stdout, (out / f'{engine}-{name}.log').open('w') as stderr:
                result = subprocess.run([str(binary)], cwd=root, env=run_env, stdin=inp, stdout=stdout, stderr=stderr)
            save(out / f'{engine}-{name}-exit.json', {'exit': result.returncode})
            result.check_returncode()
            cases, manifolds = comparator.read(data, count)
            if name == 'grid':
                if not all(row[1] > 0 for row in cases.values()):
                    raise ValueError(f'{engine}: every grid pose must exercise contacts')
                if not all(max(abs(m[0]), abs(m[1] - 1), abs(m[2])) <= 1e-5
                           for group in manifolds.values() for m in group):
                    raise ValueError(f'{engine}: angled flat-interior normal')
            if engine == 'gpu':
                comparison = comparator.compare(out / f'cpu-{name}.txt', data, count, 1e-5)
                record['results'][name] = comparison
                save(out / 'manifest.json', record)
                if comparison['status'] != 'pass':
                    return 1
    print(json.dumps(record['results'], indent=2))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
