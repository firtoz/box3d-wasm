#!/usr/bin/env python3
"""Verify portable raw evidence and reproduce the diagnosis without a GPU."""
from pathlib import Path
import difflib
import gzip
import hashlib
import json
import tarfile
from analyze import analyze
from importlib.util import module_from_spec, spec_from_file_location

ROOT = Path(__file__).resolve().parent


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read_json(name):
    return json.loads((ROOT / name).read_text())


def archive(name):
    with tarfile.open(ROOT / name) as tar:
        return {m.name: sha(tar.extractfile(m).read()) for m in tar.getmembers() if m.isfile()}


def prefix(path):
    lines = []
    for line in gzip.open(path, "rt"):
        if line.startswith("B ") and int(line.split()[1]) >= 240:
            break
        if line.startswith(("B ", "F ", "M ", "P ")):
            lines.append(line.rstrip("\n"))
    assert sum(x.startswith("B ") for x in lines) == 1200
    assert sum(x.startswith("F ") for x in lines) == 2400
    assert len(lines) == 14020
    return lines


index = read_json("raw-index.json")
assert len(index) == 36
assert {str(x.relative_to(ROOT)) for x in (ROOT / "raw").rglob("*") if x.is_file()} == set(index)
for name, expected in index.items():
    data = (ROOT / name).read_bytes()
    assert sha(data) == expected["sha256"] and len(data) == expected["bytes"], name
for original, row in read_json("reference-map.json").items():
    assert sha((ROOT / row["portable"]).read_bytes()) == row["sha256"], original

p = read_json("raw/protocol.json")
r = read_json("raw/receipt.json")
assert r["status"] == "completed-boundary-diagnostic" and "running" not in r
assert r["protocol_sha256"] == sha((ROOT / "raw/protocol.json").read_bytes())
assert r["driver_sha256"] == sha((ROOT / "raw/run.py").read_bytes())
assert p["budget"] == dict(CPU_C_translation_unit_builds=1, CPP_fixture_compilations_and_links=2,
                         archive_copy_replace_prefix_operations=3, fresh_diagnostic_processes=2,
                         completed_steps_per_process=240, Rust_builds=0, solver_candidates=0,
                         retries=0, headline_timing=0)
assert len(r["commands"]) == 5 and all(x["exit"] == 0 for x in r["commands"])
assert r["commands"][0]["command"] == p["compile_command"]
for command in r["commands"]:
    for stream in ("stdout", "stderr"):
        assert sha((ROOT / "raw" / command["name"] / (stream + ".log")).read_bytes()) == command[stream + "_sha256"]
for backend in ("ordinary", "native"):
    frozen = next(x for x in p["links"] if x["name"] == backend)
    actual = next(x for x in r["commands"] if x["name"] == backend + "-link")
    assert actual["command"] == frozen["command"]
    assert r["binaries"][backend]["Rust_library_sha256"] == frozen["Rust_library_sha256"]

# Receipt records actual ar-member identities; executables/objects are deliberately
# not shipped. This validates the recorded replacement chain, not a recompilation.
assert r["CPU_archive_copy_sha256"] == p["inputs"][p["CPU_archive"]]
assert len(p["CPU_archive_members"]) == len(r["CPU_observer_archive_members"]) == 50
assert set(p["CPU_archive_members"]) == set(r["CPU_observer_archive_members"])
assert [n for n, h in r["CPU_observer_archive_members"].items() if h != p["CPU_archive_members"][n]] == ["solver.c.o"]
assert r["CPU_observer_archive_members"]["solver.c.o"] == r["CPU_observer_object_sha256"]
deps = archive("raw/CPU-compile-dependencies.tar.gz")
assert len(deps) == len(r["CPU_compile_dependencies"]) == 91
assert deps == {n.lstrip("/"): h for n, h in r["CPU_compile_dependencies"].items()}
assert any(deps[n] == sha((ROOT / "raw/solver.c").read_bytes()) for n in deps if n.endswith("/solver.c"))

consumer = read_json("raw/reference/C-consumer.json")
assert consumer["inputs_before"] == consumer["inputs_after"] == p["CPP_consumer_inputs"]
assert consumer["compiled_units"] == p["compiled_units"] and len(p["compiled_units"]) == 178
c_files = archive("raw/reference/C-consumer-source-inputs.tar.gz")
generated = archive("raw/reference/C-consumer-generated-inputs.tar.gz")
assert len(c_files) == len(p["CPP_consumer_inputs"]) == 614
for name, h in p["CPP_consumer_inputs"].items():
    portable = name[6:] if name.startswith("../../") else name
    assert c_files[portable] == h, name
for unit in p["compiled_units"]:
    key = unit["file"].split("/box3d-wasm/", 1)[1]
    key = key.removeprefix("experiments/gpu-physics/")
    if key.startswith("artifacts/"):
        assert generated[key.split("/cmake/", 1)[1]] == unit["source_sha256"]
    else:
        assert c_files[key] == unit["source_sha256"]

producer = read_json("raw/reference/Rust-producer.json")
rust_files = archive("raw/reference/Rust-source-inputs.tar.gz")
assert rust_files == producer["compiled_inputs_before"] and len(rust_files) == 107
assert [n for n, h in p["compiled_Rust_inputs_current"].items() if h != rust_files[n]] == ["src/gpu_invariants.rs"]
assert "cfg(test)" in p["current_exception"]
for backend in ("ordinary", "native"):
    assert producer["libraries"][backend]["sha256"] == r["binaries"][backend]["Rust_library_sha256"]

for original, changed, patch in (("original-solver.c", "solver.c", "CPU-read-only-observer.patch"),
                                  ("original-drag.cpp", "drag-prefix.cpp", "fixture-prefix.patch")):
    before = (ROOT / "raw/reference" / original).read_text()
    after = (ROOT / "raw" / changed).read_text()
    expected = "".join(difflib.unified_diff(before.splitlines(True), after.splitlines(True), fromfile=original, tofile=changed))
    assert (ROOT / patch).read_text() == expected
    if changed == "solver.c":
        assert all(line.startswith("---") for line in expected.splitlines() if line.startswith("-"))
        assert 'maxMotion > safetyFactor * sim->minExtent' in after

assert [x["name"] for x in r["results"]] == ["ordinary", "native"]
for row, frozen in zip(r["results"], p["cases"]):
    backend = row["name"]
    assert row["exit"] == 0 and row["pass"] and row["observer_neutral"] and row["completed_prefix"]
    assert row["command"] == frozen["command"] and row["environment_overrides"] == frozen["environment_overrides"]
    stderr = (ROOT / "raw" / backend / "stderr.log").read_text()
    assert "NVIDIA GeForce RTX 4070 SUPER" in stderr and "backend=Vulkan" in stderr
    assert "DIAGNOSTIC completed 240-step original drag prefix" in (ROOT / "raw" / backend / "stdout.log").read_text()
    for stream in ("stdout", "stderr"):
        assert sha((ROOT / "raw" / backend / (stream + ".log")).read_bytes()) == row[stream + "_sha256"]
    actual = prefix(ROOT / "raw" / backend / "comparison.txt.gz")
    baseline = prefix(ROOT / "raw/reference" / (backend + "-baseline.txt.gz"))
    assert actual == baseline
    assert sha(("\n".join(actual) + "\n").encode()) == row["observer_prefix_sha256"] == row["baseline_prefix_sha256"]
    assert sha(gzip.decompress((ROOT / "raw" / backend / "comparison.txt.gz").read_bytes())) == row["comparison_sha256"]

analysis, coverage = analyze()
assert read_json("analysis.json") == analysis and read_json("coverage-audit.json") == coverage
assert not coverage["complete_both_backend_boundary_coverage"] and not coverage["PR04_complete"]
assert coverage["backends"]["native"]["GPU_host_TOI_records"] == 0
for backend in ("ordinary", "native"):
    assert coverage["backends"][backend]["CPU_TOI_records"] == 0
    assert all(x["CPU_classification"]["fast"] == 0 for x in analysis["observations"][backend]["window"])
impact = next(x for x in analysis["observations"]["ordinary"]["window"] if x["frame"] == 227)
assert impact["GPU_host_boundary"]["fraction"] == 0.6233841
assert impact["GPU_host_boundary"]["translation_m"] > .02
assert impact["GPU_host_boundary"]["pre_CCD_position_difference_m"] < .002
assert impact["GPU_host_boundary"]["correction_norm_m"] > .016
assert impact["GPU_host_boundary"]["reconstruction_residual_m"] < 1e-6
assert impact["post_position_difference_m"] > .017
assert impact["velocity_difference_m_per_s"] < 2e-6
spec = spec_from_file_location("ccd_source_audit", ROOT / "source-audit.py")
module = module_from_spec(spec)
spec.loader.exec_module(module)
assert read_json("source-audit.json") == module.audit()
print("PASS:36 raw files; frozen build/source/receipt chain; 480 completed diagnostic steps; 28,040 exact archived prefix lines; ordinary CCD correction attributed. Native internal TOI coverage and full dragging/PR04 acceptance remain OPEN. No candidate or timing run.")
