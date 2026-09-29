#!/usr/bin/env python3
"""Evidence-integrity and interrupted-compression controls for the sample runner."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
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

    def test_failed_receipt_cannot_be_finalized(self):
        r.save(self.path / 'exit.json', {'exit': 53})
        before = (self.path / 'exit.json').read_bytes()
        with self.assertRaises(ValueError):
            r.finalize_attempt(self.path, 2)
        self.assertEqual((self.path / 'exit.json').read_bytes(), before)
        self.assertFalse((self.path / 'complete.json').exists())

    def test_child_receipt_is_required_and_hashed(self):
        r.save(self.path / 'exit.json', {'exit': 0, 'child_exit': 0, 'launcher_exit': 0})
        with self.assertRaises(FileNotFoundError):
            r.finalize_attempt(self.path, 2)
        r.save(self.path / 'child-exit.json', {'exit': 0})
        r.finalize_attempt(self.path, 2)
        self.assertIsNotNone(r.completed_trace(self.path, 2))
        r.save(self.path / 'child-exit.json', {'exit': 1})
        with self.assertRaises(ValueError):
            r.completed_trace(self.path, 2)


class LauncherTests(unittest.TestCase):
    def test_launcher_and_child_failures_are_separate_and_preserved(self):
        # Real bounded subprocesses, with a fake Xvfb launcher so this regression
        # needs neither a display nor GPU. Cover cleanup failure, sample failure,
        # and launchers that never execute their child (even if they exit zero).
        for child_exit, launcher_exit, launch in [(0, 53, True), (7, 0, True),
                                                  (0, 53, False), (0, 0, False)]:
            with self.subTest(child=child_exit, launcher=launcher_exit, launch=launch):
                with tempfile.TemporaryDirectory() as tmp:
                    root = Path(tmp)
                    fixture = root / 'sample'
                    fixture.write_text(f'#!/bin/sh\nexit {child_exit}\n')
                    fixture.chmod(0o755)
                    launcher = root / 'xvfb-run'
                    launcher.write_text('#!/bin/sh\nshift 3\n' +
                                        ('"$@"\n' if launch else '') + f'exit {launcher_exit}\n')
                    launcher.chmod(0o755)
                    cmd = [sys.executable, str(Path(r.__file__)), '--binary', str(fixture),
                           '--out', str(root / 'batch'), '--configuration', 'ordinary',
                           '--steps', '1', '--runs', '5']
                    env = dict(os.environ, PATH=str(root) + os.pathsep + os.environ['PATH'])
                    result = subprocess.run(cmd, env=env, capture_output=True, timeout=10)
                    self.assertNotEqual(result.returncode, 0)
                    attempt = root / 'batch/run-1/attempt-0001'
                    receipt = json.loads((attempt / 'exit.json').read_text())
                    self.assertEqual(receipt['launcher_exit'], launcher_exit)
                    self.assertEqual(receipt['child_exit'], child_exit if launch else None)
                    self.assertNotEqual(receipt['exit'], 0)
                    self.assertFalse((attempt / 'complete.json').exists())
                    before = {p.name: (p.stat().st_mtime_ns, p.read_bytes()) for p in attempt.iterdir()}
                    retry = subprocess.run(cmd, env=env, capture_output=True, timeout=10)
                    self.assertNotEqual(retry.returncode, 0)
                    self.assertEqual(before, {p.name: (p.stat().st_mtime_ns, p.read_bytes())
                                              for p in attempt.iterdir()})
                    self.assertEqual(len(list(attempt.parent.glob('attempt-*'))), 1)

    def test_child_exit_and_signal_receipts_preserve_lock(self):
        for signal in [False, True]:
            with self.subTest(signal=signal), tempfile.TemporaryDirectory() as tmp:
                receipt = Path(tmp) / 'child-exit.json'
                with (Path(tmp) / 'lock').open('w') as lock:
                    code = f'import os; os.fstat({lock.fileno()})'
                    if signal:
                        code += '; os.kill(os.getpid(), 15)'
                    command = [sys.executable, str(Path(r.__file__)), '--record-child-exit',
                               str(receipt), str(lock.fileno()), sys.executable, '-c', code]
                    result = subprocess.run(command, pass_fds=(lock.fileno(),), timeout=10)
                self.assertEqual(result.returncode, 143 if signal else 0)
                self.assertEqual(json.loads(receipt.read_text())['exit'], -15 if signal else 0)


if __name__ == '__main__':
    unittest.main()
