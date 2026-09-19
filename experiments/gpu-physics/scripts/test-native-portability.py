#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import unittest


def module(name):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(name+'.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


launch = module('run-native-samples')
link = module('prepare-samples-link')


class PortabilityTests(unittest.TestCase):
    def test_automatic_backend_policy(self):
        self.assertTrue(launch.choose_cache('Linux','auto','auto',True))
        for os in ('Darwin','Windows'):
            self.assertFalse(launch.choose_cache(os,'auto','auto',True))
        self.assertFalse(launch.choose_cache('Linux','auto','auto',False))
        self.assertFalse(launch.choose_cache('Linux','auto','dx12',True))
        self.assertFalse(launch.choose_cache('Linux','0','auto',True))

    def test_forced_unsupported_cache_is_an_error(self):
        for args in [('Darwin','1','auto',True),('Windows','1','dx12',True),
                     ('Linux','1','vulkan',False),('Linux','typo','auto',True)]:
            with self.assertRaises(ValueError):
                launch.choose_cache(*args)

    def test_filter_keeps_prototypes_statics_and_literal_braces(self):
        source = '''#include "keep.h"
extern int chosen(int a);
static int helper(int a) { return a; }
B3_API int chosen(int a)
{
    const char* braces = "} {";
    /* } */ if (a) { return helper(a); }
    return 0;
}
B3_API const char* retained(void) { return "ok"; }
'''
        filtered, added = link.filter_source(source, {'chosen','helper'})
        self.assertIn('extern int chosen(int a);',filtered)
        self.assertIn('static int helper',filtered)
        self.assertNotIn('braces',filtered)
        self.assertEqual(added,{'retained'})
        self.assertEqual(source.count('\n'),filtered.count('\n'))

    def test_unbalanced_source_fails(self):
        with self.assertRaises(ValueError):
            list(link.definitions('int broken(void) { return 1;'))


if __name__ == '__main__':
    unittest.main()
