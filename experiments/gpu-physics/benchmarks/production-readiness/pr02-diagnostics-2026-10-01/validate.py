#!/usr/bin/env python3
"""Validate this exhausted campaign's portable receipts; never run GPU trials."""
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parent


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(path):
    return json.loads((ROOT / path).read_text())


def passing_runs(path, expected):
    receipt = load(path)
    assert receipt['status'] == 'pass', path
    assert len(receipt['runs']) == expected, path
    assert all(run['exit'] == 0 for run in receipt['runs']), path
    assert 'running' not in receipt, path
    return receipt


def main():
    index = load('raw-index.json')['entries']
    assert len({entry['path'] for entry in index}) == len(index)
    for entry in index:
        assert sha(ROOT / entry['path']) == entry['portable_sha256'], entry['path']
    passing_runs('raw/rust/receipt.json', 4)
    passing_runs('raw/regressions/receipt.json', 16)
    for backend in ['ordinary', 'native']:
        passing_runs(f'raw/c/baseline-{backend}/receipt.json', 1)
        build = load(f'raw/candidate-complete-inputs/{backend}-build.json')
        assert build['status'] == 'built'
        assert build['engine_sources'] == build['engine_sources_after']
        with tarfile.open(ROOT / f'raw/candidate-complete-inputs/{backend}-compiled-sources.tar.gz') as archive:
            for path, expected in build['engine_sources'].items():
                source = archive.extractfile(path.replace('../../box3d/', 'box3d/'))
                assert source and hashlib.sha256(source.read()).hexdigest() == expected, path
        with gzip.open(ROOT / f'raw/rust/{backend}-2.jsonl.gz', 'rt') as source:
            frame = json.load(source)
        assert frame['schema'] == 'gpu-core-state-v24' and frame['frame'] == 5
        assert frame['host_state']['maximum_capacity'] == [1, 35, 1, 35, 1]
        assert frame['gpu_policy']['native_contact_peak'] == 1
        assert frame['contact_allocation']['native_contact_peak_device'] == 1
        assert frame['idle_state']['physics_invalid'] is False
        audit = load(f'raw/audit-{backend}.json')
        assert audit['required_stateful_header_symbols'] == 415
        assert len(audit['declared_unavailable_symbols']) == 39
        for field in ['remaining_stub_definitions', 'additional_known_placeholders',
                      'cpu_only_comparison_passthrough', 'additional_missing_stateful_symbols',
                      'duplicate_stub_definitions']:
            assert audit[field] == [], (backend, field)
        for linkage in ['gpu', 'both']:
            config = backend + '-' + linkage
            passing_runs(f'raw/c/{config}/receipt.json', 2)
            viewer = load(f'raw/viewers/{config}/receipt.json')
            assert viewer['status'] == 'built' and viewer['exit'] == 0
            assert viewer['inputs_before'] == viewer['inputs_after']
            assert viewer['campaign_adapter_sources_unchanged'] is True
    assert 'Ran 16 tests' in (ROOT / 'raw/storage-controls.log').read_text()
    progress = load('progress.json')
    assert [progress[key] for key in ['baseline_gpu_processes', 'candidate_C_processes',
                                      'candidate_Rust_processes', 'selected_regression_invocations']] == [2, 8, 4, 16]
    assert progress['timing_runs'] == 0
    clips = load('raw/recording-checks.json')['clips']
    assert len(clips) == 40
    for clip in clips:
        path = ROOT / 'raw/recordings' / clip['engine'] / (clip['scene'] + '.mp4')
        assert sha(path) == clip['sha256']
        assert clip['stream']['nb_frames'] == '300'
    recording = load('raw/standard-recordings.json')
    assert recording['status'] == 'complete' and len(recording['runs']) == 4
    assert all(run['exit'] == 0 for run in recording['runs'])
    for name in ['receipt.json', 'oracle-receipt.json']:
        receipt = load('raw/recording-build/' + name)
        assert receipt['status'] == 'built'
        assert receipt['inputs_before'] == receipt['inputs_after']
    interaction = load('raw/viewer-interaction-result.json')
    assert interaction['consumed_apps'] == 2 and interaction['budget_apps'] == 5
    assert interaction['status'] == 'stopped: tab-selection harness failure'
    for config in ['cpu', 'ordinary-gpu']:
        receipt = load('raw/viewer-interactions/' + config + '/receipt.json')
        assert receipt['exit'] == 0 and receipt['capture_exit'] == 0
        assert sha(ROOT / 'raw/viewer-interactions' / config / 'clip.mp4') == receipt['video_sha256']

    print(f'Validated {len(index)} portable files; 30 GPU processes, 16 host controls, 4 viewer builds, 40 scene clips; retained tab-selection failure. PR02 remains open.')


if __name__ == '__main__':
    main()
