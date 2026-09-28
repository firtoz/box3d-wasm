#!/usr/bin/env python3
"""Regression controls for the narrowly audited membership comparison view."""
import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('compare', Path(__file__).with_name('compare-core-state.py'))
compare = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compare)

class StorageViewTests(unittest.TestCase):
    def setUp(self):
        pair = [{'shape': {'identity': 1, 'live': True}, 'child': -1},
                {'shape': {'identity': 2, 'live': True}, 'child': -1}]
        self.frame = {'schema': 'gpu-core-state-v19',
            'contact_allocation': {'occupied_order': [[[1, 2], 0], [[1, 3], 0]],
                                   'slots': [{'generation': 7}], 'high_water': 4},
            'event_history': {'previous_touching': [None, pair, pair], 'previous_shapes': [1, 2]},
            'contact_history': {'owner': [2, 7]},
            'host_contacts': {'manifolds': [1, 2], 'owners': [2, 7]},
            'bodies': [{'velocity': ['00000000']}]}

    def test_membership_permutations_and_duplicates(self):
        before = copy.deepcopy(self.frame)
        other = copy.deepcopy(self.frame)
        other['contact_allocation']['occupied_order'].reverse()
        other['event_history']['previous_touching'] = [other['event_history']['previous_touching'][1]]
        self.assertEqual(compare.audited_storage_view(self.frame), compare.audited_storage_view(other))
        self.assertEqual(self.frame, before, 'raw trace must remain untouched')
        self.assertIsNotNone(compare.difference(self.frame, other), 'raw mode must still see the difference')

    def test_semantic_changes_are_not_hidden(self):
        mutations = [
            lambda f: f['contact_allocation']['occupied_order'].pop(),
            lambda f: f['contact_allocation']['slots'][0].update(generation=8),
            lambda f: f['contact_allocation'].update(high_water=5),
            lambda f: f['event_history'].update(previous_touching=[]),
            lambda f: f['event_history']['previous_touching'][1][0]['shape'].update(live=False),
            lambda f: f['contact_history'].update(owner=[2, 8]),
            lambda f: f['host_contacts']['manifolds'].reverse(),
            lambda f: f['host_contacts'].update(owners=[2, 8]),
            lambda f: f['bodies'][0].update(velocity=['3f800000']),
        ]
        for change in mutations:
            other = copy.deepcopy(self.frame)
            change(other)
            self.assertIsNotNone(compare.difference(compare.audited_storage_view(self.frame),
                                                   compare.audited_storage_view(other)))

    def test_corrupt_membership_is_rejected(self):
        mutations = [
            lambda f: f.update(schema='gpu-core-state-v18'),
            lambda f: f['contact_allocation']['occupied_order'].append([[1, 2], 0]),
            lambda f: f['contact_allocation']['occupied_order'].append([[1, 2], 1]),
            lambda f: f['event_history'].update(previous_touching=[{}]),
        ]
        for change in mutations:
            other = copy.deepcopy(self.frame)
            change(other)
            with self.assertRaises(ValueError): compare.audited_storage_view(other)

if __name__ == '__main__': unittest.main()
