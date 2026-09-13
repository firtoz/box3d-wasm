#!/usr/bin/env python3
"""Exercise sleep and no-sleep benchmarks and verify identical requested windows."""
import json
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="gpu-bench-window-") as temporary:
    for sleep in (True, False):
        output = pathlib.Path(temporary) / f"sleep-{sleep}.json"
        subprocess.run([
            str(root / "target/release/gpu-physics"), "--scene", "dominoes",
            "--bench", str(output), "--warmup", "3", "--frames", "2",
            "--metric-runs", "1", "--sleep" if sleep else "--no-sleep",
        ], cwd=root, check=True, stdout=subprocess.DEVNULL)
        report = json.loads(output.read_text())
        assert report["warmup_steps"] == 3 and report["timed_steps"] == 2
        for run in report["raw_runs"]:
            assert run["physics_step"] == 5, run
            assert run["settle_wait_steps"] == 0, run
            assert len(run["completed_step_ms"]) == 2
        assert report["cpu_trials"], "CPU oracle evidence is required"
        for cpu in report["cpu_trials"]:
            assert cpu.get("engine") == "box3d-cpu", cpu
            assert cpu["warmup_steps"] == 3 and cpu["timed_steps"] == 2, cpu
        print(f"sleep={sleep}: GPU and CPU both warmup=3, timed=2; GPU ends at step 5")
