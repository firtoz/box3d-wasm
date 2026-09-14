#!/usr/bin/env python3
"""Inventory native compatibility gaps. Symbol coverage is not a physics correctness claim."""
import argparse
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
        'b3World_GetMaxCapacity', 'b3World_EnableWarmStarting',
        'b3World_IsWarmStartingEnabled', 'b3World_DumpMemoryStats',
    ]
    extra = sorted(n for n in placeholders if n in samples and n not in shim)
    passthrough = functions(ROOT / 'native-samples/build-both/both_passthrough.c')
    stateful = sorted(n for n in passthrough if re.match(r'b3(?:World|Body|Shape|Joint|\w+Joint)_', n)
                      and n not in dual)
    generator_spec = importlib.util.spec_from_file_location("both_generator", ROOT / 'scripts/gen-both-cpu.py')
    generator = importlib.util.module_from_spec(generator_spec)
    generator_spec.loader.exec_module(generator)
    headers = generator.parse_headers(ROOT.parents[1] / 'box3d/include')
    required = {n for n, (_, args_text) in headers.items()
                if re.search(r'\bb3(?:World|Body|Shape|Joint)Id\b', args_text)
                or re.match(r'b3(?:RecPlayer|Recording)_', n)
                or n in ('b3CreateWorld', 'b3CreateRecording', 'b3DestroyRecording',
                         'b3CreatePlayer', 'b3DestroyPlayer', 'b3SaveRecordingToFile',
                         'b3LoadRecordingFromFile', 'b3ValidateReplay')}
    libraries = [ROOT / 'target/release/libgpu_physics.a',
                 ROOT / 'native-samples/build-gpu/libgpu_samples_api.a']
    providers = set()
    for library in libraries:
        if not library.exists():
            continue
        symbols = subprocess.run(['nm', '-A', '-g', '--defined-only', str(library)],
                                 capture_output=True, text=True, check=True).stdout
        for line in symbols.splitlines():
            if 'samples_stubs.c.o:' not in line and re.search(r' [TW] b3', line):
                providers.add(line.split()[-1])
    # Explicit source placeholders remain gaps even if they provide a linkable symbol.
    missing = sorted(required - providers - set(stubs) - set(extra))
    result = {
        'status': 'incomplete' if stubs or extra or stateful or missing else 'complete',
        'required_stateful_header_symbols': len(required),
        'additional_missing_stateful_symbols': missing,
        'remaining_stub_definitions': sorted(stubs),
        'additional_known_placeholders': extra,
        'duplicate_stub_definitions': sorted(set(stubs) & (set(shim) | set(samples))),
        'cpu_only_comparison_passthrough': stateful,
        'note': 'Source inventory. CPU-only passthrough requires review; linked symbols and semantic behavior need separate tests.',
    }
    print(f'Remaining stubs: {len(stubs)}; additional known placeholders: {len(extra)}; '
          f'CPU-only comparison APIs to review: {len(stateful)}; other missing symbols: {len(missing)}')
    for category in ['additional_missing_stateful_symbols', 'remaining_stub_definitions', 'additional_known_placeholders', 'cpu_only_comparison_passthrough']:
        print(category + ':')
        for name in result[category]:
            print('  ' + name)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(result, indent=2) + '\n')
    if result['duplicate_stub_definitions']:
        raise SystemExit('Duplicate stub definitions: ' + ', '.join(result['duplicate_stub_definitions']))
    if args.require_complete and result['status'] != 'complete':
        raise SystemExit(1)


if __name__ == '__main__':
    main()
