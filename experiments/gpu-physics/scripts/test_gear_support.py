#!/usr/bin/env python3
"""Fast adversarial checks for the Gear Lift support gate."""
import copy
import itertools
import unittest
from unittest.mock import patch

import numpy as np
import gear_support as support


class GearSupportTests(unittest.TestCase):
    def setUp(self):
        self.cpu = dict(poses=144000, deep_poses=20, peak_depth=.059,
                        longest_deep=2, final_deep=0, floor_crossings=0)
        self.gpu = dict(poses=144000, deep_poses=8, peak_depth=.034,
                        longest_deep=1, final_deep=0, floor_crossings=0)

    def test_current_quality_is_accepted(self):
        self.assertEqual(support.compare_metrics(self.gpu, self.cpu)[0], 'ok')

    def test_each_physical_failure_is_rejected_independently(self):
        for key, bad in [('poses', 0), ('deep_poses', 21), ('peak_depth', .065),
                         ('longest_deep', 3), ('final_deep', 1), ('floor_crossings', 1)]:
            with self.subTest(key=key):
                candidate = dict(self.gpu, **{key: bad})
                self.assertEqual(support.compare_metrics(candidate, self.cpu)[0], 'fail')

    def test_bad_cpu_support_cannot_authorize_bad_gpu_support(self):
        for key in ['floor_crossings', 'final_deep']:
            self.assertEqual(support.compare_metrics(self.gpu, dict(self.cpu, **{key: 1}))[0], 'fail')

    def test_intersection_without_any_contained_vertex(self):
        bar = np.array(list(itertools.product([-2., 2.], [-.1, .1], [-.1, .1])))
        self.assertFalse(np.any(np.all(np.abs(bar) <= .5, axis=1)))
        self.assertAlmostEqual(support.gaps(bar, np.eye(3), np.zeros((1, 3)), np.full((1, 3), .5))[0], -.6)
        self.assertGreater(support.gaps(bar+[0, 2, 0], np.eye(3), np.zeros((1, 3)), np.full((1, 3), .5))[0], 0)

    def test_geometry_source_changes_fail_closed(self):
        support.geometry()
        bad = copy.deepcopy(support.GEOMETRY)
        bad['sources'][next(iter(bad['sources']))] = 'stale'
        with patch.object(support, 'GEOMETRY', bad):
            with self.assertRaisesRegex(ValueError, 'stale'):
                support.geometry()

    def test_short_window_is_not_long_run_evidence(self):
        with self.assertRaisesRegex(ValueError, '1200'):
            support.analyze({'warmup': 2, 'timed': 120})


if __name__ == '__main__':
    unittest.main()
