#!/usr/bin/env python3
"""Compare the private CPU viewer bridge with the standalone Box3D oracle."""
import ctypes as C
from pathlib import Path
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
build = root / 'oracle/build-viewer'
subprocess.run(['cmake', '--build', str(build), '--target', 'box3d_viewer_cpu', 'box3d_oracle', '-j', '6'], check=True)
libpath = build / 'libbox3d_viewer_cpu.so'
exports = subprocess.check_output(['nm', '-D', '--defined-only', str(libpath)], text=True)
assert all(line.split()[-1].startswith('viewer_cpu_') for line in exports.splitlines()), exports
lib = C.CDLL(str(libpath))
lib.viewer_cpu_abi.restype = C.c_uint32
assert lib.viewer_cpu_abi() == 2
lib.viewer_cpu_create.argtypes = [C.c_char_p, C.c_uint32]
lib.viewer_cpu_create.restype = C.c_void_p
lib.viewer_cpu_destroy.argtypes = [C.c_void_p]
lib.viewer_cpu_count.argtypes = [C.c_void_p]
lib.viewer_cpu_count.restype = C.c_uint32
lib.viewer_cpu_step.argtypes = [C.c_void_p, C.c_float, C.c_int]
lib.viewer_cpu_states.argtypes = [C.c_void_p, C.c_void_p, C.c_uint32]
lib.viewer_cpu_states.restype = C.c_uint32
assert not lib.viewer_cpu_create(b'unsupported', 1)
for name, scale, expected in [('mixed-stacks', 600, 602), ('dominoes', 30, 5431), ('falling-cubes', 50, 51)]:
    with tempfile.TemporaryDirectory(prefix='viewer-cpu-') as tmp:
        subprocess.run([str(build / 'box3d_oracle'), '--scene', name, '--bodies', str(scale), '--frames', '121', '--warmup', '0', '--dump-dir', tmp], check=True, stdout=subprocess.DEVNULL)
        data = (Path(tmp) / (name + '.bin')).read_bytes()
        assert struct.unpack_from('<4s4I', data) == (b'B3OR', 1, 121, expected, 160)
        world = lib.viewer_cpu_create(name.encode(), scale)
        assert world and lib.viewer_cpu_count(world) == expected
        states = C.create_string_buffer(expected * 112)
        try:
            assert lib.viewer_cpu_states(world, states, expected - 1) == 0
            for frame in range(121):
                assert lib.viewer_cpu_states(world, states, expected) == expected
                raw = states.raw
                # Both target scenes have zero local COM. Compare every position,
                # orientation and velocity, not only a finite/bounded envelope.
                for i in range(expected):
                    src = 20 + (frame * expected + i) * 160
                    dst = i * 112
                    for a, b, length in [(0, 0, 28), (48, 32, 28)]:
                        assert data[src+a:src+a+length] == raw[dst+b:dst+b+length], (name, frame, i, a)
                lib.viewer_cpu_step(world, 0, 4)
                assert lib.viewer_cpu_states(world, states, expected) == expected
                assert states.raw == raw
                lib.viewer_cpu_step(world, 1 / 60, 4)
        finally:
            lib.viewer_cpu_destroy(world)
        print(f'{name}: {expected} bodies, 121 frames exactly match standalone Box3D')
