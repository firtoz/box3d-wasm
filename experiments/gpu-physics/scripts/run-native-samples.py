#!/usr/bin/env python3
"""Build native samples without assuming a shell, GPU vendor or driver path."""
import argparse
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
REPO = ROOT.parents[1]


def choose_cache(system, value, backend, prerequisites):
    if value not in ('auto', '0', '1'):
        raise ValueError('GPU_PHYSICS_SAMPLES_NATIVE_CACHE must be auto, 0 or 1')
    eligible = system == 'Linux' and backend in ('auto', 'vulkan')
    if value == '1' and not eligible:
        raise ValueError('Native command caching currently requires the Linux Vulkan build; use auto or 0')
    if value == '1' and not prerequisites:
        raise ValueError('Native cache build requires bash, patch and flock; install them or set GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0')
    return value != '0' and eligible and prerequisites


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=('cpu', 'gpu', 'both'))
    parser.add_argument('--build-only', action='store_true', help='Compile without launching a window')
    args, extra = parser.parse_known_args()
    env = os.environ.copy()
    system = platform.system()
    backend = env.get('GPU_PHYSICS_BACKEND', 'auto')
    if backend not in ('auto', 'vulkan', 'metal', 'dx12'):
        parser.error('GPU_PHYSICS_BACKEND must be auto, vulkan, metal or dx12')
    try:
        cache = args.mode != 'cpu' and choose_cache(system,
            env.get('GPU_PHYSICS_SAMPLES_NATIVE_CACHE', 'auto'), backend,
            all(shutil.which(tool) for tool in ('bash', 'patch', 'flock')))
    except ValueError as error:
        parser.error(str(error))
    if args.mode != 'cpu':
        print('Physics backend: ' + ('cached Vulkan' if cache else f'ordinary wgpu ({backend})'), flush=True)
        if env.get('GPU_PHYSICS_PIPELINE_CACHE', '1') != '0':
            cache_home = Path(env.get('LOCALAPPDATA', str(Path.home() / 'AppData/Local'))) if system == 'Windows' else (
                Path.home() / 'Library/Caches' if system == 'Darwin' else Path(env.get('XDG_CACHE_HOME', str(Path.home()/'.cache'))))
            env.setdefault('GPU_PHYSICS_PIPELINE_CACHE_DIR', str(cache_home/'box3d-gpu-physics/pipelines'))
        if cache:
            # Keep the measured defaults in one file, including caller overrides.
            result = subprocess.run(['bash', '-c', 'source "$1"; env -0', 'cache-env',
                str(ROOT/'scripts/native-samples-cache-env.sh')], env=env, check=True, stdout=subprocess.PIPE)
            env.update(item.split('=',1) for item in result.stdout.decode().split('\0') if '=' in item)
            subprocess.run(['bash', str(ROOT/'scripts/build-native-cache.sh'), 'build', '--release', '--lib', '--features', 'external-c-shim'], env=env, check=True)
        else:
            subprocess.run(['cargo','build','--release','--lib','--features','external-c-shim','--target-dir',str(ROOT/'target/samples'),'--manifest-path',str(ROOT/'Cargo.toml')], env=env, check=True)
    build = ROOT/'native-samples'/f'build-{args.mode}{"-native-cache" if cache else ""}-portable'
    libdir = ROOT/'target'/('native-cache-build/release' if cache else 'samples/release')
    # Rust MSVC produces .lib; other native targets produce lib*.a.
    library = libdir/('gpu_physics.lib' if system == 'Windows' and (libdir/'gpu_physics.lib').exists() else 'libgpu_physics.a')
    subprocess.run(['cmake','-S',str(ROOT/'native-samples'),'-B',str(build),
        '-DCMAKE_BUILD_TYPE=Release','-DGPU_PORTABLE_API=ON',f'-DGPU_SAMPLES={"ON" if args.mode=="gpu" else "OFF"}',
        f'-DBOTH_SAMPLES={"ON" if args.mode=="both" else "OFF"}',f'-DBOX3D_DIR={REPO/"box3d"}',
        f'-DGPU_PHYSICS_DIR={ROOT}',f'-DGPU_PHYSICS_LIB={library}'], env=env, check=True)
    target = 'samples_'+args.mode
    subprocess.run(['cmake','--build',str(build),'--config','Release','--target',target,
        '--parallel',str(os.cpu_count() or 2)], env=env, check=True)
    binary = build/'bin'/(target+('.exe' if system=='Windows' else ''))
    for i, arg in enumerate(extra[:-1]):
        if arg == '--bench-json':
            extra[i+1] = str(Path(extra[i+1]).resolve())
    if args.build_only:
        print(f'Built {binary}', flush=True)
        return 0
    print(f'Running {binary}', flush=True)
    return subprocess.call([str(binary),*extra],cwd=REPO/'box3d',env=env)


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError) as error:
        sys.exit(str(error))
