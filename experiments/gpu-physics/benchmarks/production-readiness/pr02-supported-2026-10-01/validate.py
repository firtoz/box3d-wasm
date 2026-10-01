#!/usr/bin/env python3
"""Audit portable evidence only; never launch or repeat this closed GPU campaign."""
import hashlib
import json
from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parent


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(path):
    return json.loads((ROOT / path).read_text())


def main():
    entries = load('raw-index.json')['entries']
    assert len({entry['path'] for entry in entries}) == len(entries)
    for entry in entries:
        assert sha(ROOT / entry['path']) == entry['sha256'] == entry['original_sha256']
    protocol = load('protocol.json')
    receipt = load('raw/receipt.json')
    assert (ROOT / 'protocol.json').read_bytes() == (ROOT / 'raw/protocol-before-runs.json').read_bytes()
    assert receipt['protocol_sha256'] == sha(ROOT / 'protocol.json')
    assert receipt['status'] == 'pass' and 'running' not in receipt
    assert len(receipt['builds']) == len(receipt['C_runs']) == protocol['budget']['C_fresh_processes'] == 16
    assert len(receipt['Rust_runs']) == protocol['budget']['Rust_test_invocations'] == 20
    assert protocol['budget']['timing_runs'] == protocol['budget']['production_candidates'] == 0
    assert receipt['fixture_inputs_before'] == receipt['fixture_inputs_after']
    with tarfile.open(ROOT / 'raw/compiled-fixture-inputs.tar.gz') as archive:
        for name, expected in receipt['fixture_inputs_before'].items():
            source = archive.extractfile(name.replace('../../box3d/', 'box3d/'))
            assert source and hashlib.sha256(source.read()).hexdigest() == expected, name
    for name, expected in protocol['fixture_hashes'].items():
        assert receipt['fixture_inputs_before'][name] == expected
    adapter = 'GPU adapter: NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan driver=NVIDIA'
    for case, build, run in zip(protocol['C_cases'], receipt['builds'], receipt['C_runs']):
        assert (case['configuration'], case['fixture']) == (build['configuration'], build['fixture']) == (run['configuration'], run['fixture'])
        assert build['exit'] == run['exit'] == 0
        assert build['binary_sha256'] == run['binary_sha256']
        directory = ROOT / 'raw/C' / run['configuration'] / run['fixture']
        assert sha(directory / 'build.log') == build['log_sha256']
        assert sha(directory / 'stdout.log') == run['stdout_sha256']
        assert sha(directory / 'stderr.log') == run['stderr_sha256']
        assert adapter in (directory / 'stderr.log').read_text()
        if run['fixture'] == 'warm_start_fixture':
            rows = [json.loads(line) for line in (directory / 'stdout.log').read_text().splitlines()]
            assert len(rows) == 4 and all(row['timing_only'] is False for row in rows)
        prior = load('../pr02-diagnostics-2026-10-01/raw/c/' + run['configuration'] + '/receipt.json')
        assert sha(ROOT / '../pr02-diagnostics-2026-10-01/raw/c' / run['configuration'] / 'receipt.json') == build['adapter_receipt_sha256']
        assert all(prior['linked_inputs'].get(name) == expected for name, expected in build['linked_inputs'].items())
    for backend in ['ordinary', 'native']:
        proof = protocol['source_proof'][backend]
        build_path = ROOT / proof['build_receipt']
        assert sha(build_path) == proof['receipt_sha256']
        build = json.loads(build_path.read_text())
        assert build['engine_sources'] == build['engine_sources_after']
        assert build['binary_sha256'] == proof['library_sha256']
        assert build['test_binary_sha256'] == proof['test_sha256']
        for i, selector in enumerate(protocol['Rust_selectors'], 1):
            run = receipt['Rust_runs'][(i - 1) * 2 + (backend == 'native')]
            assert (run['configuration'], run['selector'], run['exit']) == (backend, selector, 0)
            assert run['binary_sha256'] == proof['test_sha256']
            log = ROOT / 'raw' / f'{backend}-Rust-{i}.log'
            assert sha(log) == run['log_sha256']
            content = log.read_text()
            assert 'test ' + selector + ' ...' in content and '1 passed; 0 failed' in content
            if i != 10:
                assert adapter in content
    applicability = load('applicability.json')
    assert all(all(values.values()) for values in applicability['engine_source_matches'].values())
    assert all(applicability['fixture_source_matches'].values())
    print(f'Validated {len(entries)} portable files: 16 C processes, 18 GPU and 2 host Rust checks; unchanged assertions. PR02 population/UI and final qualification remain open.')


if __name__ == '__main__':
    main()
