#!/usr/bin/env python3
"""Evidence-integrity and interrupted-compression controls for the sample runner."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('runner', Path(__file__).with_name('check-sample-state-repeats.py'))
r = importlib.util.module_from_spec(spec)
spec.loader.exec_module(r)


class RecoveryTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name)
        self.raw = b''.join((json.dumps({'schema': 'gpu-core-state-v19', 'frame': i,
            'bodies': [], 'joints': [], 'contacts': []}) + '\n').encode() for i in [1, 2])
        (self.path / 'state.jsonl').write_bytes(self.raw)
        r.save(self.path / 'health.json', {'frames': [{}, {}]})
        r.save(self.path / 'exit.json', {'exit': 0})

    def test_interrupted_compression_recovered_losslessly(self):
        (self.path / 'state.jsonl.gz').write_bytes(b'interrupted gzip')
        self.assertIsNone(r.completed_trace(self.path, 2))
        trace = r.finalize_attempt(self.path, 2)
        self.assertEqual(r.completed_trace(self.path, 2), trace)
        with r.gzip.open(trace, 'rb') as f:
            self.assertEqual(f.read(), self.raw)
        self.assertFalse((self.path / 'state.jsonl').exists())

    def test_completed_trace_is_reused_without_rewriting(self):
        trace = r.finalize_attempt(self.path, 2)
        before = {p.name: (p.stat().st_mtime_ns, r.digest(p)) for p in self.path.iterdir()}
        self.assertEqual(r.completed_trace(self.path, 2), trace)
        self.assertEqual(before, {p.name: (p.stat().st_mtime_ns, r.digest(p)) for p in self.path.iterdir()})

    def test_corrupt_completed_trace_rejected(self):
        trace = r.finalize_attempt(self.path, 2)
        trace.write_bytes(trace.read_bytes()[:-4])
        with self.assertRaises(ValueError):
            r.completed_trace(self.path, 2)

    def test_incomplete_trace_not_marked_complete(self):
        (self.path / 'state.jsonl').write_bytes(self.raw.splitlines(keepends=True)[0])
        with self.assertRaises(ValueError):
            r.finalize_attempt(self.path, 2)
        self.assertFalse((self.path / 'complete.json').exists())
        self.assertTrue((self.path / 'state.jsonl').exists())

    def test_missing_health_not_marked_complete(self):
        r.save(self.path / 'health.json', {'frames': [{}]})
        with self.assertRaises(ValueError):
            r.finalize_attempt(self.path, 2)
        self.assertFalse((self.path / 'complete.json').exists())


if __name__ == '__main__':
    unittest.main()
