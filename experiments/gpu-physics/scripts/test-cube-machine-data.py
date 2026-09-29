#!/usr/bin/env python3
"""Synthetic test-only evidence checks; these numbers are never published."""
import copy
import json
import unittest
from cube_machine_data import digest, protocol, validate, summaries

class DataTests(unittest.TestCase):
    def fixture(self):
        args=dict(counts=[100],trials=1,warmup=2,timed=3,workers=8,width=1280,height=720,
                  scene='falling-cubes',modes=['physics-cpu'],gpu_solver='component',gpu_color_prefix='20',
                  global_replay=None,backend='native',min_rate=0)
        batch={'arguments':args,'workload':'falling-cubes-v1'}
        raw=dict(sub_steps=4,warmup_steps=2,timed_steps=3,scenes={'falling-cubes':dict(
            bodies=101,workers=8,wall_samples_ms=[1.,2.,3.],wall_p50_ms=2.,wall_p95_ms=3.)})
        text=json.dumps(raw)
        row=dict(count=100,mode='physics-cpu',trial=1,status='ok',batch='b',
                 raw_file='100-physics-cpu-1.json',raw_sha256=digest(text.encode()),mean_ms=2.,p50_ms=2.,p95_ms=3.)
        return {'protocol':protocol(batch),'batches':{'b':batch},'trials':[row]}, {'files':{row['raw_file']:text}}
    def test_valid_raw_and_rate(self):
        d,b=self.fixture();validate(d,b)
        self.assertEqual(summaries(d)[0]['rate'],500.)
    def test_rejects_tampered_summary(self):
        d,b=self.fixture();d['trials'][0]['mean_ms']=1.
        with self.assertRaises(AssertionError):validate(d,b)
    def test_rejects_tampered_raw(self):
        d,b=self.fixture();b['files']['100-physics-cpu-1.json']+=' '
        with self.assertRaises(AssertionError):validate(d,b)
    def test_rejects_duplicate_trial(self):
        d,b=self.fixture();d['trials']*=2
        with self.assertRaises(AssertionError):validate(d,b)
    def test_rejects_different_window(self):
        d,b=self.fixture();d['batches']['b']['arguments']['warmup']=90
        with self.assertRaises(AssertionError):validate(d,b)
    def test_incomplete_repeat_is_missing_not_a_point(self):
        d,b=self.fixture();d['protocol']['trials']=3
        self.assertEqual(summaries(d),[])
    def test_rejects_mismatched_actual_framebuffer(self):
        d,b=self.fixture()
        d['batches']['b']['arguments']['modes']=['direct-gpu']
        row=d['trials'][0];row['mode']='direct-gpu';row['raw_file']='100-direct-gpu-1.json'
        raw=json.dumps(dict(timed=3,sleep=False,physics_step=5,physics_submit={'p50_ms':1.}))
        row['raw_sha256']=digest(raw.encode())
        b['files']={row['raw_file']:raw,'100-direct-gpu-1.cadence.json':json.dumps(dict(samples=3,p50_ms=2.,p95_ms=3.,sum_ms=6.)),
            '100-direct-gpu-1.log':'GPU presentation mode: Immediate\nmatched-window-start: 1280x720\nmatched-window-end: 1280x720\n'}
        validate(d,b)
        b['files']['100-direct-gpu-1.log']=b['files']['100-direct-gpu-1.log'].replace('1280x720','640x480')
        with self.assertRaises(AssertionError):validate(d,b)
    def test_rejects_raw_path_escape(self):
        d,b=self.fixture();b['files']['../outside']='not allowed'
        with self.assertRaises(AssertionError):validate(d,b)
    def test_rejects_trial_path_escape(self):
        d,b=self.fixture();d['trials'][0]['raw_file']='../outside'
        with self.assertRaises(AssertionError):validate(d,b)

if __name__=='__main__':unittest.main()
