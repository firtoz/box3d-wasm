#!/usr/bin/env python3
"""Regression controls for the narrowly audited membership comparison view."""
import copy
import importlib.util
from pathlib import Path
import unittest
import json
import tempfile

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

class ContactIdentityTests(unittest.TestCase):
    def setUp(self):
        self.frame = dict(schema='gpu-core-state-v20', frame=1, bodies=[], joints=[],
            contacts=[dict(count=1, points=[dict(feature_id=1, triangle=0)],
                sat_cache_words=[0,0], feature_id_words=[1,0,0,0], point_triangle_words=[0,0,0,0])],
            contact_history={}, contact_color_order=[], fat_bounds=[], pending_transforms=[], host_flags={},
            body_storage_order=[], shapes=[], geometries=[], shape_storage_order=[], body_allocation=dict(generations=[]),
            gpu_colliders=[], gpu_geometry={key:[] for key in ['hull_points','hull_planes','hull_edges','hull_topology','mesh_vertices','mesh_triangles','mesh_nodes','mix_table']},
            idle_state={}, graph_cache={}, contact_allocation={}, contact_hash={}, event_history={},
            contact_end_state={key:[] for key in ['published','deferred','ended_keys']},
            host_contacts={key:[] for key in ['registry','live','by_body','by_shape','by_id','begins','ends','hits','deferred_ends']},
            host_events={key:[] for key in ['sensor_overlaps','sensor_begins','sensor_ends','deferred_sensor_ends','continuous_sensor_hits','body_moves','joint_events']},
            host_state=dict(definition={},step_start_bodies=[],body_mirrors=[],callbacks_present={},cached_topology=None,scene_capabilities=None,query_index=None,gpu_ccd_cache_key=None),
            gpu_policy=dict(parameters={},capacities={},body_sleep_thresholds=[],fat_geometry=[],native=None),
            adapter={}, convex_ccd=None)

    def compare_frames(self, other):
        with tempfile.TemporaryDirectory() as directory:
            paths=[Path(directory)/name for name in ['a.jsonl','b.jsonl']]
            for path,frame in zip(paths,[self.frame,other]): path.write_text(json.dumps(frame)+'\n')
            return compare.compare(paths,1)

    def test_inactive_identity_changes_detected(self):
        self.assertEqual(self.compare_frames(self.frame)['status'],'pass')
        for key in ['feature_id_words','point_triangle_words']:
            other=copy.deepcopy(self.frame);other['contacts'][0][key][3]=53
            self.assertEqual(self.compare_frames(other)['status'],'fail')

    def test_missing_or_invalid_words_rejected(self):
        for key in ['feature_id_words','point_triangle_words']:
            for bad in [None, [], [0]*3, [0,0,0,-1], [0,0,0,2**32], [0,0,0,True]]:
                other=copy.deepcopy(self.frame);other['contacts'][0][key]=bad
                with self.assertRaises(ValueError):self.compare_frames(other)
            other=copy.deepcopy(self.frame);del other['contacts'][0][key]
            with self.assertRaises(ValueError):self.compare_frames(other)

    def test_active_lane_consistency_required(self):
        other=copy.deepcopy(self.frame);other['contacts'][0]['feature_id_words'][0]=2
        with self.assertRaises(ValueError):self.compare_frames(other)

    def test_legacy_remains_distinct(self):
        legacy=copy.deepcopy(self.frame);legacy['schema']='gpu-core-state-v19'
        for key in ['feature_id_words','point_triangle_words']:del legacy['contacts'][0][key]
        self.assertEqual(self.compare_frames(legacy)['status'],'fail')
        self.frame=legacy
        self.assertEqual(self.compare_frames(legacy)['status'],'pass')

class ContactHistoryTests(unittest.TestCase):
    compare_frames = ContactIdentityTests.compare_frames

    def setUp(self):
        ContactIdentityTests.setUp(self)
        self.frame['schema']='gpu-core-state-v21'
        self.frame['gpu_policy']['parameters']['contact_capacity']=2
        self.frame['contact_history']={'slot_to_rank':[1,0], 'records':[
            {'physical_slot':0, 'saved_generation':7, 'generation_matches':False}]}

    def test_lookup_and_distinct_stale_generations_are_exact(self):
        self.assertEqual(self.compare_frames(self.frame)['status'],'pass')
        for change in [lambda f:f['contact_history']['slot_to_rank'].__setitem__(0,0),
                       lambda f:f['contact_history']['records'][0].update(saved_generation=8),
                       lambda f:f['contact_history']['records'][0].update(physical_slot=1)]:
            other=copy.deepcopy(self.frame);change(other)
            self.assertEqual(self.compare_frames(other)['status'],'fail')

    def test_history_omissions_and_invalid_words_are_rejected(self):
        for bad in [None,[],[1],[-1,0],[True,0],[2**32,0]]:
            other=copy.deepcopy(self.frame);other['contact_history']['slot_to_rank']=bad
            with self.assertRaises(ValueError):self.compare_frames(other)
        for key in ['physical_slot','saved_generation']:
            other=copy.deepcopy(self.frame);del other['contact_history']['records'][0][key]
            with self.assertRaises(ValueError):self.compare_frames(other)

    def test_v20_cannot_stand_in_for_new_capture(self):
        old=copy.deepcopy(self.frame);old['schema']='gpu-core-state-v20'
        old['contact_history']={}
        self.assertEqual(self.compare_frames(old)['status'],'fail')
        self.frame=old
        self.assertEqual(self.compare_frames(old)['status'],'pass')

class RetirementCandidateTests(unittest.TestCase):
    compare_frames = ContactIdentityTests.compare_frames

    def setUp(self):
        ContactHistoryTests.setUp(self)
        self.frame['schema']='gpu-core-state-v22'
        self.frame['gpu_policy']['parameters']['pair_capacity']=4
        self.frame['contact_allocation'].update(candidate_unique_count=1,
            candidate_contact_count=1,candidate_slots=[0])

    def test_candidate_changes_remain_exact(self):
        self.assertEqual(self.compare_frames(self.frame)['status'],'pass')
        for key,value in [('candidate_slots',[0xffffffff]),('candidate_unique_count',0),('candidate_contact_count',0)]:
            other=copy.deepcopy(self.frame);other['contact_allocation'][key]=value
            self.assertEqual(self.compare_frames(other)['status'],'fail')

    def test_missing_or_malformed_candidates_rejected(self):
        for key in ['candidate_slots','candidate_unique_count','candidate_contact_count']:
            other=copy.deepcopy(self.frame);del other['contact_allocation'][key]
            with self.assertRaises(ValueError):self.compare_frames(other)
        for bad in [[],[True],[2**32],[-1],[0,1]]:
            other=copy.deepcopy(self.frame);other['contact_allocation']['candidate_slots']=bad
            with self.assertRaises(ValueError):self.compare_frames(other)

    def test_legacy_capture_is_distinct(self):
        other=copy.deepcopy(self.frame);other['schema']='gpu-core-state-v21'
        for key in ['candidate_slots','candidate_unique_count','candidate_contact_count']:
            del other['contact_allocation'][key]
        self.assertEqual(self.compare_frames(other)['status'],'fail')

class SpeculativePolicyTests(unittest.TestCase):
    compare_frames = ContactIdentityTests.compare_frames

    def setUp(self):
        RetirementCandidateTests.setUp(self)
        self.frame['schema']='gpu-core-state-v23'
        self.frame['host_state'].update(enable_speculative=True,speculative_changed=False,enable_warm_starting=True)

    def test_policy_flags_are_required_and_exact(self):
        self.assertEqual(self.compare_frames(self.frame)['status'],'pass')
        for key in ['enable_speculative','speculative_changed','enable_warm_starting']:
            other=copy.deepcopy(self.frame);other['host_state'][key]=not other['host_state'][key]
            self.assertEqual(self.compare_frames(other)['status'],'fail')
            other=copy.deepcopy(self.frame);del other['host_state'][key]
            with self.assertRaises(ValueError):self.compare_frames(other)
            other=copy.deepcopy(self.frame);other['host_state'][key]=1
            with self.assertRaises(ValueError):self.compare_frames(other)

if __name__ == '__main__': unittest.main()
