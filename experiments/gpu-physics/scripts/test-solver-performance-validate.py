#!/usr/bin/env python3
"""Bounded corruption controls for scheduling-phase performance validation."""
import copy
import json
import sys
import unittest
from pathlib import Path
from solver_performance_validate import scheduling_metrics, validate


class MetricsTests(unittest.TestCase):
    def setUp(self):
        self.row = dict(submitted_step=61, gpu_contact_metrics=dict(
            phase='contact_scheduling', known=True, current=False, capacity_loss=False,
            snapshot_step=61, submitted_step=61, snapshot_topology=10, current_topology=10,
            snapshot_state=62, current_state=63, candidate_pairs=5, allocated_roots=5,
            allocated_manifold_slots=5, touching_roots=3, non_sensor_roots=5))

    def test_ccd_revision_keeps_stale_label(self):
        before = copy.deepcopy(self.row)
        self.assertFalse(scheduling_metrics(self.row)['current'])
        self.assertEqual(self.row, before)

    def test_no_correction_is_current(self):
        self.row['gpu_contact_metrics'].update(current=True, current_state=62)
        self.assertTrue(scheduling_metrics(self.row)['current'])

    def test_invalid_metric_records_rejected(self):
        corruptions = [dict(known=False), dict(capacity_loss=True),
                       dict(snapshot_step=60), dict(submitted_step=60),
                       dict(snapshot_topology=9), dict(current_state=64),
                       dict(current_state=61), dict(current=True),
                       dict(current_state=62), dict(current=0),
                       dict(phase='completed'), dict(candidate_pairs=-1),
                       dict(snapshot_state=True)]
        for change in corruptions:
            with self.subTest(change=change):
                bad = copy.deepcopy(self.row)
                bad['gpu_contact_metrics'].update(change)
                with self.assertRaises(ValueError):
                    scheduling_metrics(bad)
        for key in self.row['gpu_contact_metrics']:
            with self.subTest(missing=key):
                bad = copy.deepcopy(self.row)
                del bad['gpu_contact_metrics'][key]
                with self.assertRaises(ValueError):
                    scheduling_metrics(bad)

    def test_between_step_mutation_and_topology_rejected(self):
        previous = dict(submitted_step=60, current_topology=10, current_state=61)
        scheduling_metrics(self.row, previous)
        for change in (dict(current_state=60), dict(current_topology=9), dict(submitted_step=59)):
            with self.subTest(change=change):
                with self.assertRaises(ValueError):
                    scheduling_metrics(self.row, previous | change)


def check_real_report(path):
    data = json.loads(path.read_text())
    kwargs = dict(sample='Determinism/Falling Ragdolls', bodies=116, joints=112, sleep=True)
    before = path.read_bytes()
    result = validate(data, **kwargs)
    corruptions = [lambda d: d.update(health_scan=True),
                   lambda d: d.update(completed_step_mode=False),
                   lambda d: d.update(measured=179),
                   lambda d: d.update(gpu_fail='capacity failure'),
                   lambda d: d.update(switch_count=1),
                   lambda d: d['frames'].pop(),
                   lambda d: d['frames'][0].update(in_flight=1),
                   lambda d: d['frames'][0].update(completed_step=60),
                   lambda d: d['frames'][0].update(physics_ms=float('nan')),
                   lambda d: d['frames'][0].update(body_count=115),
                   lambda d: d['frames'][0].update(framebuffer_width=1),
                   lambda d: d['frames'][0].update(health_ms=1),
                   lambda d: d['frames'][0].update(bodies=[{}]),
                   lambda d: d['frames'][0]['gpu_contact_metrics'].update(capacity_loss=True),
                   lambda d: d['frames'][1]['gpu_contact_metrics'].update(current_state=999)]
    for index, mutate in enumerate(corruptions):
        bad = copy.deepcopy(data)
        mutate(bad)
        try:
            validate(bad, **kwargs)
        except (ValueError, KeyError):
            pass
        else:
            raise AssertionError(f'accepted corruption {index}')
    assert path.read_bytes() == before
    print(json.dumps(dict(validation=result, rejected_report_corruptions=len(corruptions),
                          preserved_input=True)))


if __name__ == '__main__':
    report = Path(sys.argv.pop(1)) if len(sys.argv) > 1 else None
    result = unittest.TextTestRunner().run(unittest.defaultTestLoader.loadTestsFromTestCase(MetricsTests))
    if not result.wasSuccessful():
        sys.exit(1)
    if report:
        check_real_report(report)
