#!/usr/bin/env python3
"""Negative controls for full-scene spherical-limit health capture."""
import copy
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('rain', Path(__file__).with_name('compare-rain-lifetimes.py'))
rain = importlib.util.module_from_spec(spec)
spec.loader.exec_module(rain)

class SphericalHealthTests(unittest.TestCase):
    def record(self):
        return dict(type=6, spherical_limits=dict(cone_enabled=True, twist_enabled=True,
            cone_limit=0.2, lower_twist=-0.1, upper_twist=0.1, swing=0.3, twist=-0.2,
            cone_excess=0.1, lower_twist_excess=0.1, upper_twist_excess=0.0))

    def test_capture_and_disabled_limits(self):
        joint=self.record()
        self.assertIsNotNone(rain.spherical_limits(joint, True))
        joint['spherical_limits'].update(cone_enabled=False, twist_enabled=False,
            cone_excess=0, lower_twist_excess=0, upper_twist_excess=0)
        self.assertIsNotNone(rain.spherical_limits(joint, True))
        self.assertIsNone(rain.spherical_limits(dict(type=5, spherical_limits=None), True))

    def test_bad_measurements(self):
        for field,value in [('swing',float('nan')),('cone_excess',-1),
                ('cone_excess',0),('lower_twist_excess',0),('upper_twist_excess',0.1),
                ('cone_enabled',1),('cone_limit',-0.1),('lower_twist',0.2)]:
            with self.subTest(field=field,value=value):
                joint=copy.deepcopy(self.record());joint['spherical_limits'][field]=value
                with self.assertRaises(ValueError):rain.spherical_limits(joint, True)

    def test_matching_semantics_and_required_capture(self):
        cpu=self.record();cpu.update(anchor_constrained=False,angular_constrained=False)
        self.assertEqual(len(list(rain.joint_measurements(cpu,copy.deepcopy(cpu),True))),3)
        for mutation in ['missing_gpu','missing_both','cone_flag','twist_flag','cone_limit','twist_limit']:
            with self.subTest(mutation=mutation):
                a,b=copy.deepcopy(cpu),copy.deepcopy(cpu)
                if mutation=='missing_gpu':b.pop('spherical_limits')
                elif mutation=='missing_both':a.pop('spherical_limits');b.pop('spherical_limits')
                elif mutation=='cone_flag':
                    b['spherical_limits'].update(cone_enabled=False,cone_excess=0)
                elif mutation=='twist_flag':
                    b['spherical_limits'].update(twist_enabled=False,lower_twist_excess=0)
                elif mutation=='cone_limit':
                    b['spherical_limits'].update(cone_limit=0.25,cone_excess=0.05)
                else:b['spherical_limits']['upper_twist']=0.2
                with self.assertRaises(ValueError):list(rain.joint_measurements(a,b,True))

    def test_coverage(self):
        for joint in [dict(type=6),dict(type=6,spherical_limits=None),dict(type=5,spherical_limits={})]:
            with self.assertRaises(ValueError):rain.spherical_limits(joint, True)
        self.assertIsNone(rain.spherical_limits(dict(type=6), False))

class SphericalEventTests(unittest.TestCase):
    def test_episode_endings(self):
        spec = importlib.util.spec_from_file_location('events', Path(__file__).with_name('summarize-rain-residual-events.py'))
        events = importlib.util.module_from_spec(spec);spec.loader.exec_module(events)
        bodies=[dict(id=i,generation=1,creation=i,awake=False) for i in [1,2]]
        joint=SphericalHealthTests().record()
        joint.update(body_a=1,body_a_generation=1,body_b=2,body_b_generation=1,
                     anchor_constrained=False,angular_constrained=False)
        for ending in ['screen_cleared','joint_removed','capture_ended']:
            with self.subTest(ending=ending):
                cpu=[dict(i=i,bodies=copy.deepcopy(bodies),joints=[copy.deepcopy(joint)]) for i in range(3)]
                gpu=copy.deepcopy(cpu)
                for frame in gpu[:2 if ending!='capture_ended' else 3]:
                    frame['joints'][0]['spherical_limits'].update(swing=0.4,cone_excess=0.2)
                if ending=='joint_removed':
                    cpu[2]['joints']=[];gpu[2]['joints']=[]
                with patch.object(events.module,'records',side_effect=lambda path,n:iter(cpu if path=='cpu' else gpu)):
                    result=events.summarize('cpu','gpu',3,True)
                self.assertEqual(result['event_count'],1)
                event=result['events'][0]
                self.assertEqual(event['field'],'cone_excess')
                self.assertEqual(event['end_reason'],ending)
                self.assertEqual(event['first_asleep_frame'],0)
                self.assertEqual(event['observations'],3 if ending=='capture_ended' else 2)
                self.assertEqual(event['endpoints'],[1,2])

if __name__ == '__main__':unittest.main()
