#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import unittest
import json
import subprocess
import sys
import tempfile


def module(name):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(name+'.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


launch = module('run-native-samples')
link = module('prepare-samples-link')
inject = module('inject-sokol-bench')


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

    def test_missing_audit_builds_are_unknown_and_fail_required_check(self):
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp) / 'audit.json'
            result = subprocess.run([sys.executable, str(Path(__file__).with_name('audit-native-api.py')),
                '--gpu-build-dir', str(Path(temp) / 'missing-gpu'),
                '--both-build-dir', str(Path(temp) / 'missing-both'),
                '--require-built', '--json', str(output)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 1, result.stderr)
            report = json.loads(output.read_text())
            self.assertFalse(report['build_artifacts_verified'])
            self.assertIsNone(report['additional_missing_stateful_symbols'])
            self.assertIsNone(report['cpu_only_comparison_passthrough'])
            self.assertEqual(len(report['missing_build_inputs']), 3)

    def test_gpu_controls_fail_closed_when_upstream_changes(self):
        source = (Path(__file__).resolve().parents[3] / 'box3d/samples/sample.cpp').read_text()
        patched = inject.inject_gpu_limitations(source)
        self.assertNotIn('ImGui::Checkbox( "Warm Starting##Solver"', patched)
        self.assertIn('CPU Workers##Solver', patched)
        self.assertIn('if ( false && context->sample->HasSolverControls()', patched)
        with self.assertRaises(SystemExit):
            inject.inject_gpu_limitations(source.replace('Warm Starting##Solver', 'Changed control'))

    def test_unbalanced_source_fails(self):
        with self.assertRaises(ValueError):
            list(link.definitions('int broken(void) { return 1;'))


if __name__ == '__main__':
    unittest.main()
