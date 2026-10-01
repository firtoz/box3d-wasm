#!/usr/bin/env python3
"""Generate one definition per C API, without platform-specific linker interposition.

Only generated copies are edited. Explicit combined wrappers take precedence over
fallbacks; the CPU oracle archive is built from untouched upstream sources.
"""
import argparse
import importlib.util
from pathlib import Path
import re

spec = importlib.util.spec_from_file_location('both', Path(__file__).with_name('gen-both-cpu.py'))
both = importlib.util.module_from_spec(spec)
spec.loader.exec_module(both)


def mask_c(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text, flags=re.S)


def definitions(text):
    masked = mask_c(text)
    pattern = re.compile(r'(?m)^[ \t]*(?:(?:[A-Za-z_]\w*)[ \t\n*]+)+(?P<name>[A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{')
    end = 0
    for m in pattern.finditer(masked):
        if m.start() < end or re.search(r'\bstatic\b', m[0]):
            continue
        # All matched declarations must be at translation-unit scope.
        if masked[:m.start()].count('{') != masked[:m.start()].count('}'):
            continue
        depth = 1
        end = m.end()
        while depth and end < len(masked):
            depth += (masked[end] == '{') - (masked[end] == '}')
            end += 1
        if depth:
            raise ValueError('unbalanced C function: ' + m['name'])
        yield m['name'], m.start(), end


def filter_source(text, occupied):
    spans = list(definitions(text))
    names = {name for name, _, _ in spans}
    for name, start, end in reversed(spans):
        if name in occupied:
            text = text[:start] + '\n' * text[start:end].count('\n') + text[end:]
    return text, names - occupied


def write_changed(path, text):
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, required=True)
    p.add_argument('--box3d', type=Path, required=True)
    p.add_argument('--rust-lib', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--nm', required=True)
    p.add_argument('--both', action='store_true')
    args = p.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    occupied = both.defined_symbols(args.rust_lib, args.nm)
    occupied = {n[1:] if n.startswith('_b3') else n for n in occupied}
    if 'b3CreateWorld' in occupied:
        raise SystemExit('Samples need a Rust library built with --features external-c-shim; rerun the samples launcher')
    if args.both:
        dual = (args.root/'c_abi/both_dual.c').read_text()
        occupied.update(n for n, _, _ in definitions(dual))
        occupied.update(both.DUAL)
        # Passthroughs delegate the remaining public APIs to the CPU oracle.
        sigs = both.parse_headers(args.box3d/'include')
        explicit = {n for n, _, _ in definitions(dual)}
        # Unavailable GPU operations live in the shared C adapter. Keep them
        # out of CPU passthroughs without claiming they are dual wrappers.
        passthrough = set(sigs) - both.DUAL - both.UNAVAILABLE - both.SHARED_GPU_API - explicit
        # The guarded compound factory remains authoritative.
        passthrough.discard('b3CreateCompound')
        occupied.update(passthrough)
    for name in ['samples_api.c', 'shim.c', 'samples_stubs.c'] if args.both else ['shim.c', 'samples_api.c', 'samples_stubs.c']:
        text = (args.root/'c_abi'/name).read_text()
        if name == 'shim.c':
            text = text.replace('__wrap_b3CreateCompound', 'b3CreateCompound')
            # Expand this API-generating macro before selecting definitions.
            text = re.sub(r'^MOTOR_SCALAR_API\((\w+),\s*(\w+)\)$', lambda m:
                f'B3_API void b3MotorJoint_Set{m[1]}(b3JointId id, float value) {{ gpu_b3_motor_set_{m[2]}(id, value); }}\n'
                f'B3_API float b3MotorJoint_Get{m[1]}(b3JointId id) {{ return gpu_b3_motor_get_{m[2]}(id); }}', text, flags=re.M)
            # These hooks are always linked by the samples app; standalone Rust
            # builds retain the existing optional hooks.
            text = text.replace('__attribute__((weak))', '')
        text, added = filter_source(text, occupied)
        occupied.update(added)
        write_changed(args.out/name, text)
    # Normal Box3D geometry/math fallbacks resolve to the same public API as
    # before, without duplicate definitions. The CPU oracle is never filtered.
    for source in (args.box3d/'src').glob('*.c'):
        text = source.read_text()
        if source.name == 'compound.c':
            text = re.sub(r'\bb3CreateCompound\b', '__real_b3CreateCompound', text)
        text, _ = filter_source(text, occupied)
        write_changed(args.out/source.name, text)


if __name__ == '__main__':
    main()
