#!/usr/bin/env python3
"""PR02 host-only exclusion checks. Preserve every trial; no GPU initialization."""
import argparse
import hashlib
import importlib.util
import json
import re
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path, value):
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(value, indent=2) + '\n')
    temporary.replace(path)


def logged(command, path):
    with path.open('w') as log:
        result = subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    save(path.with_suffix('.receipt.json'), {'command': command, 'exit': result.returncode,
                                           'log_sha256': sha(path)})
    if result.returncode:
        raise SystemExit(f'Build failed; retained {path}')


def sources():
    paths = list((ROOT / 'c_abi').glob('*'))
    paths += [ROOT / p for p in ['scripts/gen-both-cpu.py', 'scripts/prepare-samples-link.py',
                                'scripts/check-native-unavailable.py', 'native-samples/CMakeLists.txt']]
    box = ROOT.parents[1] / 'box3d'
    names = subprocess.check_output(['git', '-C', str(box), 'ls-files', '-z']).decode().split('\0')
    paths += [box / p for p in names if p and ((box / p).suffix in ('.c', '.h', '.cmake')
                                             or (box / p).name == 'CMakeLists.txt')]
    return {str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else str(p): sha(p)
            for p in sorted(paths) if p.is_file()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--protocol', type=Path, required=True)
    args = parser.parse_args()
    protocol = json.loads(args.protocol.read_text())
    trials = protocol['budget'].get('trial_numbers',
        {name: [1, 2] for name in protocol['budget']['candidate_configurations']})
    assert sum(map(len, trials.values())) == protocol['budget']['total_candidate_processes']
    assert protocol['budget']['total_candidate_processes'] <= 8
    assert protocol['budget']['gpu_processes'] == 0
    out = args.out.resolve()
    out.mkdir(parents=True)  # A result directory is never reused.
    spec = importlib.util.spec_from_file_location('generator', ROOT / 'scripts/gen-both-cpu.py')
    generator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(generator)
    assert sorted(generator.UNAVAILABLE) == protocol['excluded_symbols']
    for config in protocol['budget']['candidate_configurations']:
        backend, linkage = config.split('-')
        result = out / config
        result.mkdir()
        before = sources()
        previous = ROOT / 'artifacts/production-readiness/pr01-speculative/handoff'
        build_receipt = previous / f'build-{backend}.json'
        engine = json.loads(build_receipt.read_text())
        library = previous / f'{backend}-frozen.a'
        assert sha(library) == engine['binary_sha256']
        for name, expected in engine['engine_sources'].items():
            assert sha(ROOT / name) == expected, name
        build = result / 'cmake'
        receipt = {'configuration': config, 'scope': 'Host-only API errors; no GPU initialized',
                   'revision_at_adapter_build': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                   'adapter_sources_before': before, 'engine_build_receipt': str(build_receipt),
                   'engine_build_receipt_sha256': sha(build_receipt),
                   'engine_library_sha256': sha(library), 'protocol_sha256': sha(args.protocol),
                   'runs': []}
        save(result / 'receipt.json', receipt)
        logged(['cmake', '-S', str(ROOT / 'native-samples'), '-B', str(build),
                '-DGPU_PHYSICS_LIB=' + str(library), '-DGPU_PORTABLE_API=ON',
                '-DGPU_SAMPLES=' + ('ON' if linkage == 'gpu' else 'OFF'),
                '-DBOTH_SAMPLES=' + ('ON' if linkage == 'both' else 'OFF'),
                '-DFETCHCONTENT_FULLY_DISCONNECTED=ON', '-DCMAKE_BUILD_TYPE=Release'], result / 'configure.log')
        targets = ['gpu_samples_api', 'box3d']
        if linkage == 'both':
            targets.insert(0, 'gpu_both_api')
        logged(['cmake', '--build', str(build), '--target', *targets, '-j4'], result / 'build.log')
        generated = sorted((build / 'api-sources').glob('*.c'))
        if linkage == 'both':
            generated.append(build / 'both_passthrough.c')
            definitions = set(re.findall(r'B3_API\s+[^;{]*?\b(b3\w+)\([^;]*?\)\s*\{',
                                         generated[-1].read_text(), re.S))
            assert not definitions & generator.UNAVAILABLE
        receipt['generated_compilation_sources'] = {str(p.relative_to(build)): sha(p) for p in generated}
        receipt['compile_commands'] = json.loads((build / 'compile_commands.json').read_text())
        fixture = ROOT / 'c_abi/native_unavailable_fixture.cpp'
        exe = result / 'fixture'
        command = ['g++', '-O2', '-std=c++17', str(fixture), '-I', str(ROOT.parents[1] / 'box3d/include'),
                   '-I', str(ROOT / 'c_abi')]
        inputs = [fixture, library, build / 'libgpu_samples_api.a', build / 'box3d_src/libbox3d.a']
        if linkage == 'both':
            command += ['-DGPU_API_DUAL', '-Wl,--whole-archive', str(build / 'libgpu_both_api.a'),
                        '-Wl,--no-whole-archive']
            inputs += [build / 'libgpu_both_api.a', build / 'libbox3d_cpu.a']
        command += ['-Wl,--whole-archive', str(build / 'libgpu_samples_api.a'), '-Wl,--no-whole-archive', str(library)]
        if linkage == 'both':
            command.append(str(build / 'libbox3d_cpu.a'))
        command += [str(build / 'box3d_src/libbox3d.a'), '-ldl', '-lpthread', '-lm', '-lgcc_s', '-lGL', '-o', str(exe)]
        logged(command, result / 'link.log')
        after = sources()
        assert before == after, 'Adapter input changed during compilation'
        receipt['adapter_sources_after'] = after
        receipt['linked_inputs'] = {str(p): sha(p) for p in inputs}
        receipt['executable_sha256'] = sha(exe)
        save(result / 'receipt.json', receipt)
        for trial in trials[config]:
            stdout, stderr = result / f'trial-{trial}.stdout', result / f'trial-{trial}.stderr'
            with stdout.open('w') as output, stderr.open('w') as errors:
                process = subprocess.Popen([str(exe), str(result / f'unavailable-{trial}.recording')],
                                           cwd=ROOT, stdout=output, stderr=errors)
                receipt['running'] = {'pid': process.pid, 'trial': trial}
                save(result / 'receipt.json', receipt)
                code = process.wait()
            receipt.pop('running', None)
            receipt['runs'].append({'trial': trial, 'exit': code, 'stdout_sha256': sha(stdout),
                                    'stderr_sha256': sha(stderr)})
            save(result / 'receipt.json', receipt)
            print(config, trial, code, flush=True)
            if code:
                raise SystemExit('Failed candidate retained; protocol stopped')
        assert len({x['stdout_sha256'] for x in receipt['runs']}) == 1
        receipt['result'] = 'pass'
        save(result / 'receipt.json', receipt)


if __name__ == '__main__':
    main()
