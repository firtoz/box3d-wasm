#!/usr/bin/env python3
"""Check renderer reservation boundaries and generated-source compatibility."""
import os
from pathlib import Path
import subprocess
import tempfile
import runpy

root = Path(__file__).resolve().parents[1]
generate = runpy.run_path(str(root/'scripts/inject-sokol-capacity.py'))['generate']
with tempfile.TemporaryDirectory(prefix='sokol-capacity-') as directory:
    out = Path(directory)
    generate(root.parents[1]/'box3d/samples/gfx', out/'gfx')
    # Split-view injection must remain composable with the reservation hooks.
    subprocess.run(['python3', str(root/'scripts/inject-sokol-split.py'), str(out/'gfx'), str(out/'split')], check=True)
    (out/'main.c').write_text('#include "sokol_capacity.h"\n#include <stdio.h>\nint main(void) { printf("%d\\n", sample_renderer_capacity()); }\n')
    for both in (False, True):
        binary = out/('both' if both else 'single')
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', *(['-DBOTH_SAMPLES'] if both else []),
            '-I', str(root/'native-samples'), str(out/'main.c'), str(root/'native-samples/sokol_capacity.c'), '-o', str(binary)], check=True)
        env = {k:v for k,v in os.environ.items() if k not in ('GPU_BENCH_CUBES','GPU_SAMPLES_SHAPE_CAPACITY')}
        cases = [({},65536), ({'GPU_BENCH_CUBES':'100000'},200002 if both else 100001),
            ({'GPU_BENCH_CUBES':'1000000'},2000002 if both else 1000001),
            ({'GPU_SAMPLES_SHAPE_CAPACITY':'250000'},250000),
            ({'GPU_BENCH_CUBES':'2147483647'},None), ({'GPU_BENCH_CUBES':'-1'},None),
            ({'GPU_SAMPLES_SHAPE_CAPACITY':'garbage'},None),
            ({'GPU_BENCH_CUBES':'100000','GPU_SAMPLES_SHAPE_CAPACITY':'65536'},None)]
        for extra, expected in cases:
            result = subprocess.run([str(binary)], env=env|extra, capture_output=True, text=True)
            if expected is None:
                assert result.returncode != 0, extra
            else:
                assert result.returncode == 0 and int(result.stdout) == expected, (extra,result)
print('Sokol reservation: high counts, overflow, invalid/undersized settings and split generation passed')
