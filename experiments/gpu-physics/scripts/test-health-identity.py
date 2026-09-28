#!/usr/bin/env python3
"""Controls against silently matching recycled or ambiguous diagnostic joints."""
import unittest
from health_identity import match_joints, creation_body_map, health_to_core_creations
from copy import deepcopy


def joint(slot, a=(1, 1), b=(2, 1), kind=5):
    return dict(id=slot, generation=1, type=kind, body_a=a[0], body_a_generation=a[1],
                body_b=b[0], body_b_generation=b[1])


class IdentityTests(unittest.TestCase):
    def test_different_allocations_match_explicit_lifetimes(self):
        cpu, gpu = joint(3), joint(800, (20, 4), (10, 9))
        self.assertEqual(match_joints([cpu], [gpu], {(20,4):(1,1), (10,9):(2,1)}), [(cpu,gpu)])

    def test_recycled_slot_cannot_use_previous_lifetime_mapping(self):
        with self.assertRaisesRegex(ValueError, 'unmapped'):
            match_joints([joint(3)], [joint(3, b=(2,2))], {(1,1):(1,1), (2,1):(2,1)})

    def test_explicit_new_lifetime_mapping_allows_different_reuse_policies(self):
        c, g = joint(3, b=(2,2)), joint(900, b=(9,1))
        self.assertEqual(match_joints([c],[g],{(1,1):(1,1),(9,1):(2,2)}),[(c,g)])

    def test_duplicate_parallel_joints_are_not_arbitrarily_paired(self):
        with self.assertRaisesRegex(ValueError, 'ambiguous'):
            match_joints([joint(3),joint(4)],[joint(3),joint(4)],{(1,1):(1,1),(2,1):(2,1)})

    def test_wrong_joint_type_is_not_paired(self):
        with self.assertRaisesRegex(ValueError, 'topology'):
            match_joints([joint(3)],[joint(3,kind=6)],{(1,1):(1,1),(2,1):(2,1)})

    def test_non_bijective_map_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'one-to-one'):
            match_joints([],[],{(1,1):(5,1),(2,1):(5,1)})

    def test_legacy_records_cannot_silently_match(self):
        with self.assertRaises(KeyError):
            match_joints([{'id':1}],[{'id':1}],{})


class CreationTests(unittest.TestCase):
    def test_slot_reuse_maps_new_creation(self):
        c = [dict(id=1,generation=2,creation=15)]
        g = [dict(id=9,generation=7,creation=15)]
        self.assertEqual(creation_body_map(c,g),{(9,7):(1,2)})

    def test_different_creation_with_same_slot_rejected(self):
        with self.assertRaisesRegex(ValueError,'sets differ'):
            creation_body_map([dict(id=1,generation=1,creation=1)],
                              [dict(id=1,generation=1,creation=2)])

    def test_uninstrumented_and_duplicate_records_rejected(self):
        for bodies in [[dict(id=1,generation=1,creation=0)],
                       [dict(id=1,generation=1,creation=1)]*2]:
            with self.assertRaises(ValueError):
                creation_body_map(bodies,bodies)


class CoreCreationTests(unittest.TestCase):
    def setUp(self):
        self.health = dict(i=375, completed_step=376,
                           bodies=[dict(id=2, generation=7, creation=2379)])
        self.state = dict(frame=376, bodies=[dict(identity=1), dict(identity=2479)],
                          body_allocation=dict(slots=[1, 2479, None], generations=[1, 7, 2]))

    def test_recycled_slot_maps_current_generation(self):
        self.assertEqual(health_to_core_creations(self.health, self.state), {2379:2479})
        self.state['body_allocation']['slots'][1] = 8000
        self.state['bodies'][1]['identity'] = 8000
        self.assertEqual(health_to_core_creations(self.health, self.state), {2379:8000})

    def test_stale_deleted_and_out_of_range_slots_rejected(self):
        for slot, generation in [(2,6), (3,2), (4,1)]:
            h = deepcopy(self.health)
            h['bodies'][0].update(id=slot, generation=generation)
            with self.assertRaisesRegex(ValueError, 'missing or stale'):
                health_to_core_creations(h, self.state)

    def test_wrong_step_rejected(self):
        for field in ['i', 'completed_step']:
            h = deepcopy(self.health)
            h[field] += 1
            with self.assertRaisesRegex(ValueError, 'step mismatch'):
                health_to_core_creations(h, self.state)

    def test_duplicate_and_missing_core_identity_rejected(self):
        for identities in [[1,1], [1], [1,9000]]:
            s = deepcopy(self.state)
            s['bodies'] = [dict(identity=i) for i in identities]
            with self.assertRaisesRegex(ValueError, 'core body identities'):
                health_to_core_creations(self.health, s)

    def test_duplicate_health_lifetimes_rejected(self):
        self.health['bodies'].append(dict(id=2, generation=7, creation=2380))
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            health_to_core_creations(self.health, self.state)


if __name__ == '__main__':
    unittest.main()
