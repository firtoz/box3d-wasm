#!/usr/bin/env python3
"""Run actual generated adapter regressions using an already built CPU viewer.
No GPU process or implicit build. Requires --build-dir with compile_commands.json.
"""
import argparse, json, os, runpy, shlex, subprocess, tempfile
from pathlib import Path
root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--build-dir', type=Path, required=True)
a = p.parse_args(); build = a.build_dir.resolve()
units = json.loads((build/'compile_commands.json').read_text())
u = next(u for u in units if u['file'].endswith('/debug_adapter.c'))
link = shlex.split((build/'CMakeFiles/samples_cpu.dir/link.txt').read_text())
libs = [str((build/x).resolve()) if x.endswith(('.a','.so')) and not x.startswith('/') else x for x in link[link.index('-o')+2:]]
with tempfile.TemporaryDirectory(prefix='sokol-debug-pool-') as temporary:
    out = Path(temporary)
    runpy.run_path(str(root/'scripts/inject-sokol-capacity.py'))['generate'](root.parents[1]/'box3d/samples/gfx', out/'gfx')
    cmd = shlex.split(u['command']); cmd[cmd.index('-o')+1] = str(out/'test.o'); cmd[cmd.index('-c')+1] = str(root/'native-samples/sokol_debug_pool_test.c')
    cmd = [x for x in cmd if x not in ['-DNDEBUG','-O3']]
    cmd += ['-O1','-UNDEBUG','-DEXPECT_FIXED','-ffunction-sections','-fdata-sections','-I'+str(out/'gfx'),'-I'+str(root.parents[1]/'box3d/samples/gfx')]
    subprocess.run(cmd, cwd=u['directory'], check=True)
    subprocess.run(['c++','-Wl,--gc-sections',str(out/'test.o'),'-o',str(out/'test'),*libs], cwd=build, check=True)
    for case in range(6):
        env = {k:v for k,v in os.environ.items() if k not in ['GPU_BENCH_CUBES','GPU_SAMPLES_SHAPE_CAPACITY']}
        if case: env['GPU_SAMPLES_SHAPE_CAPACITY'] = '257' if case == 5 else '8'
        subprocess.run([str(out/'test'),str(case)], env=env, check=True, timeout=30)
