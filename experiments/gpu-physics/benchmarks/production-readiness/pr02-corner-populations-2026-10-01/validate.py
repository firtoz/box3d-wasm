#!/usr/bin/env python3
"""Validate retained campaigns offline, including failures; never launch physics."""
import hashlib
import json
from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parent


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(root, path):
    return json.loads((root / path).read_text())


def check_dataset(root, expected):
    entries = load(root, 'raw-index.json')['entries']
    assert len({entry['path'] for entry in entries}) == len(entries)
    for entry in entries:
        assert sha(root / entry['path']) == entry['sha256'], entry['path']
    protocol = load(root, 'protocol.json')
    receipt = load(root, 'raw/receipt.json')
    assert receipt['protocol_sha256'] == sha(root / 'protocol.json')
    assert (root / 'protocol.json').read_bytes() == (root / 'raw/protocol-before-runs.json').read_bytes()
    assert receipt['status'] == 'failed' and 'running' not in receipt
    assert len(receipt['builds']) == 5
    assert [(run['configuration'], run['exit']) for run in receipt['runs']] == expected
    assert receipt['inputs_before'] == receipt['inputs_after']
    assert receipt['inputs_before']['c_abi/native_diagnostic_populations.cpp'] == protocol['fixture_sha256']
    with tarfile.open(root / 'raw/fixture-inputs.tar.gz') as archive:
        for name, value in receipt['inputs_before'].items():
            content = archive.extractfile(name.replace('../../box3d/', 'box3d/'))
            assert content and hashlib.sha256(content.read()).hexdigest() == value
    for build in receipt['builds']:
        assert build['exit'] == 0
        assert sha(root / 'raw' / build['configuration'] / 'build.log') == build['log_sha256']
        if build['configuration'] == 'cpu':
            assert set(build['linked_inputs'].values()) == {protocol['source_proof']['CPU_library_sha256']}
            prior = root / protocol['source_proof']['CPU_receipt_portable']
            assert sha(prior) == protocol['source_proof']['CPU_receipt_sha256']
            old = json.loads(prior.read_text())
            assert all(old['linked_archives'].get(name) == value for name, value in build['linked_inputs'].items())
        else:
            proof = protocol['source_proof'][build['configuration']]
            prior = root / proof['receipt_portable']
            assert sha(prior) == proof['adapter_receipt_sha256']
            old = json.loads(prior.read_text())
            assert all(old['linked_inputs'].get(name) == value for name, value in build['linked_inputs'].items())
    for run in receipt['runs']:
        directory = root / 'raw' / run['configuration']
        assert sha(directory / 'stdout.log') == run['stdout_sha256']
        assert sha(directory / 'stderr.log') == run['stderr_sha256']
        assert run['binary_sha256'] == next(build['binary_sha256'] for build in receipt['builds'] if build['configuration'] == run['configuration'])
        if run['configuration'] != 'cpu':
            assert 'GPU adapter: NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan driver=NVIDIA' in (directory / 'stderr.log').read_text()
    assert protocol['budget']['CPU_reference_processes'] == 1 and protocol['budget']['GPU_processes'] == 4
    assert protocol['budget']['timing_runs'] == protocol['budget']['production_candidates'] == 0
    return len(entries)


def main():
    total = 0
    for name in ['pr02-populations-2026-10-01', 'pr02-population-identities-2026-10-01']:
        total += check_dataset(ROOT.parent / name, [('cpu', -6)])
    total += check_dataset(ROOT, [('cpu', 0), ('ordinary-gpu', 0), ('native-gpu', 0), ('ordinary-both', -6)])
    for backend in ['ordinary', 'native']:
        content = (ROOT / 'raw' / (backend + '-gpu') / 'stdout.log').read_text()
        assert 'native diagnostic populations: sensor/compound/mesh contract passed' in content
        assert 'mesh body=3 shape=3 contact=1 manifolds=0,0,1,' in content
    fail = (ROOT / 'raw/ordinary-both/stderr.log').read_text()
    assert 'cpu.bodyCount==bodies && cpu.shapeCount==shapes && cpu.jointCount==0' in fail
    host = ROOT.parent / 'pr02-viewer-controls-2026-10-01'
    for entry in load(host, 'raw-index.json')['entries']:
        assert sha(host / entry['path']) == entry['sha256']
    receiver = load(host, 'raw/input-receiver/receipt.json')
    assert receiver['status'] == 'pass'
    assert receiver['protocol_sha256'] == sha(host / 'protocol.json')
    assert [event['type'] for event in receiver['events']] == [4, 5, 2, 3]
    assert receiver['mouse_held_ms'] == 251 and receiver['key_held_ms'] == 100
    assert load(host, 'progress.json')['viewer_apps'] == 0
    print(f'Validated {total} population files plus host input proof: 3 CPU runs, 3 GPU runs, two retained harness failures and one combined defect. No viewer app or timing acceptance.')


if __name__ == '__main__':
    main()
