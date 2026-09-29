"""Portable cube sweep data: retain raw files and verify them before plotting."""
import gzip
import hashlib
import json
import math
from pathlib import Path
import runpy
import statistics
import tempfile
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[1]
BENCH = runpy.run_path(str(ROOT / 'scripts/bench-falling-cubes.py'))
MODES = ['physics-cpu', 'physics-gpu', 'direct-cpu', 'direct-gpu']
COUNTS = [100, 1000, 5000, 10000, 25000, 50000, 100000, 150000, 200000]
FIELDS = ['counts', 'trials', 'warmup', 'timed', 'workers', 'width', 'height',
          'scene', 'gpu_solver', 'gpu_color_prefix', 'global_replay', 'backend', 'min_rate']
DEFAULTS = dict(scene='falling-cubes', gpu_solver='component', gpu_color_prefix='20',
                global_replay=None, backend='native', min_rate=10)

def digest(data):
    return hashlib.sha256(data).hexdigest()

def protocol(manifest):
    a = manifest['arguments']
    return {'workload': manifest['workload'], 'substeps': 4, 'sleep': False,
            **{k: a.get(k, DEFAULTS.get(k)) for k in FIELDS}}

def read_dataset(path):
    data = json.loads(path.read_text())
    assert data['schema'] == 'cube-machine-v1'
    name = data['raw_bundle']['file']
    assert Path(name).name == name, 'bundle must be next to the dataset'
    raw = path.with_name(name).read_bytes()
    assert digest(raw) == data['raw_bundle']['sha256'], 'raw bundle hash mismatch'
    bundle = json.loads(gzip.decompress(raw))
    validate(data, bundle)
    return data, bundle

def validate(data, bundle):
    identities = set()
    with tempfile.TemporaryDirectory(prefix='cube-data-') as temp:
        directory = Path(temp)
        for name, text in bundle['files'].items():
            assert Path(name).name == name, 'unsafe raw filename'
            (directory / name).write_bytes(text.encode())
        for row in data['trials']:
            identity = (row['count'], row['mode'], row['trial'])
            assert identity not in identities, 'duplicate trial'
            identities.add(identity)
            batch = data['batches'][row['batch']]
            assert protocol(batch) == data['protocol'], 'incompatible measurement protocols'
            assert row['count'] in data['protocol']['counts']
            assert 1 <= row['trial'] <= data['protocol']['trials']
            assert row['mode'] in MODES and row['mode'] in batch['arguments']['modes']
            if row['status'] != 'ok':
                assert row['status'] == 'invalid'
                continue
            assert Path(row['raw_file']).name == row['raw_file'], 'unsafe trial filename'
            path = directory / row['raw_file']
            assert digest(path.read_bytes()) == row['raw_sha256']
            raw = json.loads(path.read_text())
            args = SimpleNamespace(**batch['arguments'])
            measured = BENCH['metrics'](raw, row['mode'], path, args, row['count'])
            if row['mode'].startswith('direct-'):
                assert measured['framebuffer'] == [args.width, args.height], 'actual framebuffer differs from shared protocol'
            if row['mode'] == 'physics-cpu':
                assert (raw['sub_steps'], raw['warmup_steps'], raw['timed_steps']) == (4, args.warmup, args.timed)
            for key in ['mean_ms', 'p50_ms', 'p95_ms']:
                assert math.isfinite(row[key]) and row[key] > 0
                assert math.isclose(row[key], measured[key], abs_tol=1e-9, rel_tol=1e-9), (identity, key)
        assert identities, 'no trials to publish'

def summaries(data):
    result = []
    for mode in MODES:
        for count in data['protocol']['counts']:
            group = [r for r in data['trials'] if r['status']=='ok' and r['mode']==mode and r['count']==count]
            if len(group) != data['protocol']['trials']:
                continue
            means = [r['mean_ms'] for r in group]
            result.append(dict(mode=mode, count=count, rate=1000/statistics.median(means),
                rate_low=1000/max(means), rate_high=1000/min(means),
                mean_ms=statistics.median(means), p50_ms=statistics.median(r['p50_ms'] for r in group),
                p95_ms=statistics.median(r['p95_ms'] for r in group)))
    return result
