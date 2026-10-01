#!/usr/bin/env python3
"""Inventory native compatibility gaps. Symbol coverage is not a physics correctness claim."""
import argparse
import os
import platform
import shutil
import importlib.util
import subprocess
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def functions(path):
    if not path.exists():
        return set()
    text = path.read_text()
    return {m.group(1) for m in re.finditer(
        r'^B3_API\s+[^;{]*?\b(b3\w+)\([^;]*?\)\s*\{', text, re.M | re.S)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--json', type=Path, help='Optional artifact output (use an ignored artifacts directory).')
    parser.add_argument('--require-complete', action='store_true')
    parser.add_argument('--require-built', action='store_true', help='Fail if linked artifacts or generated wrappers are absent')
    parser.add_argument('--gpu-build-dir', type=Path, help='Portable GPU CMake build directory (defaults to launcher policy)')
    parser.add_argument('--both-build-dir', type=Path, help='Portable combined CMake build directory (defaults to launcher policy)')
    parser.add_argument('--nm', default=shutil.which('llvm-nm') or shutil.which('nm') or 'nm')
    args = parser.parse_args()
    stubs = functions(ROOT / 'c_abi/samples_stubs.c')
    shim = functions(ROOT / 'c_abi/shim.c')
    samples = functions(ROOT / 'c_abi/samples_api.c')
    dual = functions(ROOT / 'c_abi/both_dual.c')
    # Explicitly inspected additional placeholders. Keep these here until tested
    # real implementations replace them; absence from samples_stubs is not coverage.
    placeholders = [
        'b3CreateRecording', 'b3DestroyRecording', 'b3World_StartRecording',
        'b3World_StopRecording', 'b3SaveRecordingToFile', 'b3World_GetProfile',
        'b3World_GetMaxCapacity', 'b3World_DumpMemoryStats',
    ]
    extra = sorted(n for n in placeholders if n in samples and n not in shim)
    generator_spec = importlib.util.spec_from_file_location("both_generator", ROOT / 'scripts/gen-both-cpu.py')
    generator = importlib.util.module_from_spec(generator_spec)
    generator_spec.loader.exec_module(generator)
    unavailable = generator.UNAVAILABLE
    remaining_stubs = stubs - unavailable
    extra = sorted(set(extra) - unavailable)
    error_sources = '\n'.join((ROOT / ('c_abi/' + name)).read_text()
                              for name in ['samples_stubs.c', 'samples_api.c'])
    missing_error_hooks = sorted(name for name in unavailable if not re.search(
        r'B3_API\s+[^;{]*?\b' + re.escape(name) +
        r'\([^;]*?\)\s*\{\s*gpu_native_api_unavailable\(__func__\);', error_sources, re.S))
    headers = generator.parse_headers(ROOT.parents[1] / 'box3d/include')
    required = {n for n, (_, args_text) in headers.items()
                if re.search(r'\bb3(?:World|Body|Shape|Joint)Id\b', args_text)
                or re.match(r'b3(?:RecPlayer|Recording)_', n)
                or n in ('b3CreateWorld', 'b3CreateRecording', 'b3DestroyRecording',
                         'b3CreatePlayer', 'b3DestroyPlayer', 'b3SaveRecordingToFile',
                         'b3LoadRecordingFromFile', 'b3ValidateReplay')}
    launch_spec = importlib.util.spec_from_file_location('launcher', ROOT / 'scripts/run-native-samples.py')
    launcher = importlib.util.module_from_spec(launch_spec)
    launch_spec.loader.exec_module(launcher)
    cache = launcher.choose_cache(platform.system(), os.environ.get('GPU_PHYSICS_SAMPLES_NATIVE_CACHE', 'auto'),
                                  os.environ.get('GPU_PHYSICS_BACKEND', 'auto'),
                                  all(shutil.which(tool) for tool in ('bash', 'patch', 'flock')))
    suffix = '-native-cache-portable' if cache else '-portable'
    gpu_build = (args.gpu_build_dir or ROOT / 'native-samples' / ('build-gpu' + suffix)).resolve()
    both_build = (args.both_build_dir or ROOT / 'native-samples' / ('build-both' + suffix)).resolve()
    missing_inputs = []
    config = gpu_build / 'CMakeCache.txt'
    library = None
    if config.is_file():
        match = re.search(r'^GPU_PHYSICS_LIB:[^=]+=(.+)$', config.read_text(), re.M)
        if match:
            library = Path(match[1])
    if library is None or not library.is_file():
        missing_inputs.append('Rust library from ' + str(config))
    archives = sorted([*gpu_build.glob('**/libgpu_samples_api.a'), *gpu_build.glob('**/gpu_samples_api.lib')])
    if len(archives) != 1:
        missing_inputs.append('Exactly one gpu_samples_api archive in ' + str(gpu_build))
    passthrough_path = both_build / 'both_passthrough.c'
    if not passthrough_path.is_file():
        missing_inputs.append(str(passthrough_path))
    passthrough = functions(passthrough_path)
    stateful = (sorted(n for n in passthrough if re.match(r'b3(?:World|Body|Shape|Joint|\w+Joint)_', n)
                       and n not in dual and n not in generator.DUAL)
                if passthrough_path.is_file() else None)
    providers = set()
    linked_symbols = set()
    libraries = ([library] if library and library.is_file() else []) + (archives if len(archives) == 1 else [])
    for archive in libraries:
        symbols = subprocess.run([args.nm, '-A', '-g', '--defined-only', str(archive)],
                                 capture_output=True, text=True, check=True).stdout
        for line in symbols.splitlines():
            if re.search(r' [TW] ', line):
                linked_symbols.add(line.split()[-1].removeprefix('_'))
            if 'samples_stubs.c.' not in line and re.search(r' [TW] _?b3', line):
                providers.add(line.split()[-1].removeprefix('_'))
    # Missing artifacts are unknown coverage, never an empty/successful result.
    missing = sorted(required - providers - set(stubs) - set(extra)) if not missing_inputs else None
    required_placeholders = (set(stubs) | set(extra)) - unavailable
    # Counter topology fields are implemented, but contact/island and other
    # diagnostic fields require PR02 review. Symbol presence is not coverage.
    required_diagnostics = required_placeholders | {'b3World_GetCounters'}
    contract = []
    for name in sorted(required):
        category = ('declared-unavailable' if name in unavailable else
                    'required-diagnostic-gap' if name in required_diagnostics else
                    'cpu-only-routing-review' if name in (stateful or []) else
                    'missing-or-unverified-link' if missing_inputs or name not in linked_symbols else
                    'implemented-behavior-unqualified')
        contract.append({'symbol': name, 'classification': category,
                         'linked_in_gpu_inputs': name in linked_symbols if not missing_inputs else None})
    result = {
        'status': 'incomplete' if remaining_stubs or extra or stateful or missing or missing_inputs or missing_error_hooks else 'complete',
        'build_artifacts_verified': not missing_inputs,
        'missing_build_inputs': missing_inputs,
        'gpu_build_dir': str(gpu_build), 'both_build_dir': str(both_build),
        'libraries': [str(p) for p in libraries],
        'required_stateful_header_symbols': len(required),
        'additional_missing_stateful_symbols': missing,
        'remaining_stub_definitions': sorted(remaining_stubs),
        'additional_known_placeholders': extra,
        'duplicate_stub_definitions': sorted(set(stubs) & (set(shim) | set(samples))),
        'cpu_only_comparison_passthrough': stateful,
        'declared_unavailable_symbols': sorted(unavailable),
        'unavailable_source_error_hooks_missing': missing_error_hooks,
        'unavailable_error_interface_linked': (all(name in linked_symbols for name in
            ['gpu_b3_native_api_last_error', 'gpu_b3_native_api_clear_error',
             'gpu_b3_native_api_is_unavailable']) if not missing_inputs else None),
        'required_stub_or_placeholder_definitions': sorted(required_placeholders),
        'initial_release_stateful_inventory': contract,
        'note': 'Source inventory. CPU-only passthrough requires review; linked symbols and semantic behavior need separate tests.',
    }
    print(f'Declared unavailable: {len(unavailable)}; remaining stubs: {len(remaining_stubs)}; additional known placeholders: {len(extra)}; '
          f'CPU-only comparison APIs to review: {len(stateful) if stateful is not None else "unknown"}; '
          f'other missing symbols: {len(missing) if missing is not None else "unknown"}')
    for category in ['additional_missing_stateful_symbols', 'remaining_stub_definitions', 'additional_known_placeholders', 'cpu_only_comparison_passthrough']:
        print(category + ':')
        for name in result[category] or []:
            print('  ' + name)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(result, indent=2) + '\n')
    if result['duplicate_stub_definitions']:
        raise SystemExit('Duplicate stub definitions: ' + ', '.join(result['duplicate_stub_definitions']))
    for missing_input in missing_inputs:
        print('Missing build input: ' + missing_input)
    if args.require_built and missing_inputs:
        raise SystemExit(1)
    if args.require_complete and result['status'] != 'complete':
        raise SystemExit(1)


if __name__ == '__main__':
    main()
