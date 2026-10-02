#!/usr/bin/env python3
"""Offline diagnosis only: no build, device, or simulation is launched."""
from pathlib import Path
import gzip
import json
import math
import re

ROOT = Path(__file__).resolve().parent


def vector(text):
    values = [float(x) for x in text.split(",")]
    assert all(map(math.isfinite, values))
    return values


def trace_states(path):
    states = {}
    body = None
    for line in gzip.open(path, "rt"):
        a = line.split()
        if a[0] == "B":
            body = int(a[2])
        elif a[0] == "F" and body == 0:
            values = list(map(float, a[4:7] + a[8:12] + a[13:16] + a[17:20]))
            assert len(values) == 13 and all(map(math.isfinite, values))
            states[int(a[1]), int(a[2])] = dict(
                p=values[:3], q=values[3:7], v=values[7:10], w=values[10:])
    assert len(states) == 480
    return states


def analyze():
    observations = {}
    coverage = {}
    pattern = re.compile(r"gpu-ccd-trace step=(\d+) body=(\d+) start=\[([^]]+)\] end=\[([^]]+)\] fraction=([\d.e+-]+)")
    for backend in ("ordinary", "native"):
        stderr = (ROOT / "raw" / backend / "stderr.log").read_text()
        states = trace_states(ROOT / "raw" / backend / "comparison.txt.gz")
        classification = {}
        for line in stderr.splitlines():
            if not line.startswith("CPUclass "):
                continue
            fields = dict(token.split("=", 1) for token in line.split()[1:])
            row = {k: vector(v) if k in ("p", "q", "v", "w") else float(v)
                   for k, v in fields.items()}
            frame = int(row.pop("frame"))
            assert row["body"] == 1
            # CPU body1 = original trace body0; trace engine0 = CPU.
            assert row["p"] == states[frame, 0]["p"]
            assert row["q"] == states[frame, 0]["q"]
            classification[frame] = row
        assert sorted(classification) == list(range(220, 233))
        gpu = {}
        for match in pattern.finditer(stderr):
            step, body, start, end, fraction = match.groups()
            assert int(body) == 2
            # GPU step is one based, original trace frame is zero based.
            gpu[int(step) - 1] = dict(start=vector(start), end=vector(end), fraction=float(fraction))
        cpu_toi_count = stderr.count("CPUtoi ")
        coverage[backend] = dict(
            CPU_classification_frames=len(classification), CPU_TOI_records=cpu_toi_count,
            GPU_host_TOI_records=len(gpu), direct_GPU_TOI_observed=bool(gpu),
            native_internal_GPU_TOI_observed=False,
            original_full_dragging_acceptance=False)
        window = []
        for frame, cpu in classification.items():
            state = states[frame, 1]
            row = dict(frame=frame, CPU_classification=cpu, GPU_post_state=state,
                       post_position_difference_m=math.dist(cpu["p"], state["p"]),
                       velocity_difference_m_per_s=math.dist(states[frame, 0]["v"], state["v"]))
            if frame in gpu:
                boundary = gpu[frame]
                start, end, fraction = boundary["start"], boundary["end"], boundary["fraction"]
                reconstructed = [a + fraction * (b - a) for a, b in zip(start, end)]
                row["GPU_host_boundary"] = dict(
                    **boundary, translation_m=math.dist(start, end),
                    pre_CCD_position_difference_m=math.dist(cpu["p"], end),
                    reconstructed_post=reconstructed,
                    reconstruction_residual_m=math.dist(reconstructed, state["p"]),
                    correction_vector_m=[b - a for a, b in zip(end, state["p"])],
                    correction_norm_m=math.dist(end, state["p"]))
            window.append(row)
        observations[backend] = dict(window=window)
    report = dict(
        scope="Diagnostic host-float64 arithmetic on printed records, not original-limit physics acceptance. Exact printed prefix equality does not establish complete future-state equality. Native internal GPU TOI remains unobserved; no fraction is inferred from its post state.",
        observations=observations)
    audit = dict(
        scope="Receipt pass means adapter/completed240step/prefix equality only; boundary coverage is separately audited here.",
        backends=coverage, complete_both_backend_boundary_coverage=False,
        production_candidate=False, headline_performance=False,
        PR04_complete=False)
    return report, audit


if __name__ == "__main__":
    report, audit = analyze()
    for name, value in (("analysis.json", report), ("coverage-audit.json", audit)):
        (ROOT / name).write_text(json.dumps(value, indent=2) + "\n")
    row = next(x for x in report["observations"]["ordinary"]["window"] if x["frame"] == 227)
    print(json.dumps({"ordinary_frame227": row, "coverage": audit["backends"]}, indent=2))
