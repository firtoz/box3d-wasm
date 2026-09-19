#!/usr/bin/env python3
"""Validate native-scene JSON records. Missing/stale/incomplete data is not a pass."""
from __future__ import annotations

import json
import math
import re
import sys
from pathlib import Path


GENERIC_SPEED = 120.0
GENERIC_MAX_Y = 200.0


def load_json(path: Path):
    try:
        return json.loads(path.read_text())
    except Exception as exc:
        return {"_error": str(exc)}


def record_complete(doc: dict, expected_sample: str, expected_timed: int) -> tuple[str, str]:
    if not isinstance(doc, dict):
        return "fail", "record must be a JSON object"
    if doc.get("_error"):
        return "fail", f"invalid json: {doc['_error']}"
    if doc.get("status") not in ("ok", "incomplete"):
        return "fail", f"missing status (got {doc.get('status')!r})"
    if not doc.get("sample"):
        return "fail", "missing sample identity"
    if doc.get("sample") != expected_sample:
        return "fail", f"sample identity {doc.get('sample')!r} != {expected_sample!r}"
    timed = doc.get("timed")
    measured = doc.get("measured")
    if timed is None or measured is None:
        return "fail", "timed/measured are required"
    if type(timed) is not int or type(measured) is not int or measured < 0:
        return "fail", "timed/measured must be nonnegative integers"
    if timed != expected_timed:
        return "fail", f"timed {timed} != requested {expected_timed}"
    if int(measured) != int(timed):
        return "incomplete", f"measured {measured} != timed {timed}"
    frames = doc.get("frames")
    if not isinstance(frames, list):
        return "fail", "frames must be an array"
    if len(frames) != int(measured):
        return "fail", f"truncated frames {len(frames)} != measured {measured}"
    if doc.get("status") != "ok":
        return "incomplete", "record status is incomplete"
    if doc.get("gear_diagnostic", 0) != 0:
        return "fail", "diagnostic scene omissions are not native parity evidence"
    if doc.get("health_scan") is not True:
        return "fail", "correctness record requires health_scan"
    for key in ("warmup", "worker_count", "enable_sleep", "unpaced", "completed_step_mode"):
        if key not in doc:
            return "fail", f"missing setting {key}"
    for key in ("warmup", "worker_count"):
        if type(doc[key]) is not int or doc[key] < 0:
            return "fail", f"invalid setting {key}"
    for key in ("enable_sleep", "unpaced", "completed_step_mode"):
        if type(doc[key]) is not bool:
            return "fail", f"invalid setting {key}"
    if expected_sample == "Continuous/Mesh Drop" and type(doc.get("scene_seed")) is not int:
        return "fail", "Mesh Drop requires recorded scene seed"
    if expected_sample == "Compound/Village":
        if doc.get("village_drop") is not True:
            return "fail", "Village requires recorded sphere-drop interaction"
        if timed < 600 or doc["warmup"] != 0:
            return "incomplete", "Village requires 600 measured steps from the initial drop"
    if expected_sample == "Joints/Gear Lift" and (doc.get("warmup") != 2 or expected_timed < 1200):
        return "incomplete", "Gear Lift requires 1200 measured steps after two warmup steps"
    for i, frame in enumerate(frames):
        if not isinstance(frame, dict):
            return "fail", f"invalid frame {i}"
        for key in ("min_y", "max_y", "max_speed", "nan_count", "body_count", "joint_count", "submitted_step"):
            value = frame.get(key)
            if type(value) not in (int, float) or not math.isfinite(value):
                return "fail", f"frame {i}: missing/non-finite {key}"
        if frame.get("sample", expected_sample) != expected_sample:
            return "fail", f"frame {i}: wrong scene identity"
        if frame.get("i") != i or frame["submitted_step"] != doc["warmup"] + i + 1:
            return "fail", f"frame {i}: unexpected physics step {frame['submitted_step']}"
        if frame["min_y"] > frame["max_y"] or frame["max_speed"] < 0:
            return "fail", f"frame {i}: invalid health bounds"
        if type(frame.get("exploded")) is not bool:
            return "fail", f"frame {i}: missing exploded flag"
        bodies = frame.get("bodies")
        if not isinstance(bodies, list) or not bodies:
            return "fail", f"frame {i}: missing per-body trace"
        ids = set()
        for body in bodies:
            if not isinstance(body, dict):
                return "fail", f"frame {i}: invalid body record"
            if type(body.get("id")) is not int or body["id"] in ids:
                return "fail", f"frame {i}: invalid/duplicate body identity"
            ids.add(body["id"])
            for key, size in (("p", 3), ("q", 4), ("v", 3), ("w", 3)):
                vec = body.get(key)
                if not isinstance(vec, list) or len(vec) != size or any(
                    type(x) not in (int, float) or not math.isfinite(x) for x in vec
                ):
                    return "fail", f"frame {i} body {body['id']}: invalid {key}"
            if abs(sum(x*x for x in body["q"]) - 1.0) > 0.01:
                return "fail", f"frame {i} body {body['id']}: non-unit orientation"
        joints = frame.get("joints")
        if not isinstance(joints, list) or len(joints) != frame["joint_count"]:
            return "fail", f"frame {i}: incomplete joint trace"
        joint_ids = set()
        for joint in joints:
            if not isinstance(joint, dict) or type(joint.get("id")) is not int or type(joint.get("anchor_constrained")) is not bool:
                return "fail", f"frame {i}: invalid joint record"
            value = joint.get("anchor_error")
            if joint.get("id") in joint_ids or type(value) not in (int, float) or not math.isfinite(value):
                return "fail", f"frame {i}: invalid joint measurement"
            joint_ids.add(joint.get("id"))
            angular = joint.get("angular_error")
            if type(joint.get("angular_constrained")) is not bool or type(angular) not in (int, float) or not math.isfinite(angular) or angular < 0:
                return "fail", f"frame {i}: invalid angular joint measurement"
    return "ok", "complete"


def generic_health(doc: dict) -> tuple[str, str]:
    frames = doc.get("frames") or []
    for frame in frames:
        if frame.get("exploded"):
            return (
                "fail",
                f"exploded at frame {frame.get('i')} min_y={frame.get('min_y')} "
                f"speed={frame.get('max_speed')} body={frame.get('worst_body')}",
            )
        sample = frame.get("sample", doc.get("sample"))
        speed_limit = 180.0 if sample == "Continuous/Bounce House" else GENERIC_SPEED
        speed = frame.get("max_speed")
        max_y = frame.get("max_y")
        nan_count = frame.get("nan_count") or 0
        if nan_count:
            return "fail", f"non-finite pose nan_count={nan_count}"
        if speed is not None and speed > speed_limit:
            return "fail", f"runaway speed {speed}"
        if max_y is not None and max_y > GENERIC_MAX_Y:
            return "fail", f"runaway max_y {max_y}"
        if sample == "Continuous/Bounce House":
            bodies = frame.get("bodies", [])
            if len(bodies) != 1 or frame.get("body_count") != 3 or frame.get("joint_count") != 0:
                return "fail", "Bounce House requires one dynamic sphere"
            body = bodies[0]
            if abs(body["p"][0]) > 9.51 or abs(body["p"][2]) > 9.51 or abs(body["p"][1] - 4.0) > 0.002:
                return "fail", "Bounce House sphere escaped its native enclosure"
            # Restitution=1, no gravity/friction: speed must remain sqrt(2)*120.
            body_speed = math.sqrt(sum(x*x for x in body["v"]))
            if abs(body_speed - math.sqrt(2)*120) > 0.05:
                return "fail", "Bounce House gained/lost kinetic energy"
    return "ok", "scene-aware finite/runaway checks"


def envelope(doc: dict, key: str) -> list[float]:
    values = []
    for frame in doc.get("frames") or []:
        value = frame.get(key)
        if isinstance(value, (int, float)) and math.isfinite(value):
            values.append(float(value))
    return values


def compare_village_drop(gpu: dict, cpu: dict) -> tuple[str, str]:
    for label, doc in (("GPU", gpu), ("CPU", cpu)):
        frames = doc["frames"]
        for frame in frames:
            if frame["body_count"] != 2 or frame["joint_count"] != 0 or len(frame["bodies"]) != 1:
                return "fail", f"{label} Village requires static compound plus one dropped sphere"
        if frames[0]["bodies"][0]["p"][1] < 29.0:
            return "fail", f"{label} Village drop did not start above the scene"
        if max(abs(f["bodies"][0]["v"][1]) for f in frames) < 10.0:
            return "fail", f"{label} Village drop never exercised impact"
        for frame in frames[-120:]:
            body = frame["bodies"][0]
            if not 1.5 < body["p"][1] < 3.0 or math.sqrt(sum(x*x for x in body["v"])) > 0.05:
                return "fail", f"{label} Village lost stationary roof support"
    for g, c in zip(gpu["frames"], cpu["frames"]):
        a, b = g["bodies"][0], c["bodies"][0]
        if a["id"] != b["id"]:
            return "fail", "Village sphere identities differ"
        for key in ("p", "q", "v", "w"):
            if any(abs(x-y) > 1e-5 for x, y in zip(a[key], b[key])):
                return "fail", f"Village step {g['submitted_step']}: {key} differs from CPU by more than 1e-5"
    return "ok", "600-step Village sphere-drop trajectory and stationary support match CPU"


def compare_cpu(gpu: dict, cpu: dict | None, scene_id: str) -> tuple[str, str]:
    if cpu is None:
        return "incomplete", "missing matched CPU trace"
    cpu_status, cpu_detail = record_complete(cpu, scene_id, gpu.get("timed") or 0)
    if cpu_status != "ok":
        return "incomplete", f"cpu trace {cpu_status}: {cpu_detail}"
    for key in ("warmup", "worker_count", "enable_sleep", "unpaced", "completed_step_mode"):
        if gpu[key] != cpu[key]:
            return "fail", f"unmatched setting {key}: gpu={gpu[key]} cpu={cpu[key]}"
    if gpu.get("scene_seed") != cpu.get("scene_seed"):
        return "fail", "unmatched scene seeds"
    health_status, health_detail = generic_health(cpu)
    if health_status != "ok":
        return "fail", f"invalid CPU reference: {health_detail}"
    if scene_id == "Compound/Village":
        return compare_village_drop(gpu, cpu)
    gear_rock_ids = frozenset()
    gear_support_checked = scene_id == "Joints/Gear Lift"
    if gear_support_checked:
        try:
            from gear_support import compare as compare_gear_support, ROCK_IDS
            gear_rock_ids = ROCK_IDS
        except ImportError as exc:
            return "fail", f"Gear Lift support checker unavailable: {exc}"
        support_status, support_detail = compare_gear_support(gpu, cpu)
        if support_status != "ok":
            return support_status, support_detail
    # Check every corresponding frame. These remain broad screening envelopes,
    # not a substitute for geometric support and joint residual measurements.
    for g, c in zip(gpu["frames"], cpu["frames"]):
        if (g["body_count"], g["joint_count"]) != (c["body_count"], c["joint_count"]):
            return "fail", f"step {g['submitted_step']}: body/joint count mismatch"
        gb = {b["id"]: b for b in g["bodies"]}
        cb = {b["id"]: b for b in c["bodies"]}
        if gb.keys() != cb.keys():
            return "fail", f"step {g['submitted_step']}: dynamic body identities differ"
        for identity, body in gb.items():
            if scene_id == "Issues/s&box Ghost Collisions":
                # Native fixture: locked upright, half-height 36 inches, walkable y=0.
                if body["p"][1] < 36 * 0.0254 - 0.02:
                    return "fail", f"step {g['submitted_step']} body {identity}: floor penetration exceeds 0.02m"
                if body["v"][1] > 0.5:
                    return "fail", f"step {g['submitted_step']} body {identity}: ghost launch vy={body['v'][1]}"
            if identity not in gear_rock_ids and body["p"][1] < cb[identity]["p"][1] - 2.0:
                return "fail", f"step {g['submitted_step']} body {identity}: downward drift >2m"
        if gear_support_checked:
            # Body 3 is the driven input gear in the source-pinned fixture.
            # Preserve real mechanism progress, not just stationary support.
            if 3 not in gb or 3 not in cb:
                return "fail", "Gear Lift input gear missing"
            qa, qb = gb[3]["q"], cb[3]["q"]
            chord = math.sqrt(min(sum((a-b)**2 for a,b in zip(qa,qb)),
                                  sum((a+b)**2 for a,b in zip(qa,qb))))
            angle = 4*math.asin(min(1.0, chord/2))
            if angle > 0.05:
                return "fail", f"step {g['submitted_step']}: Gear Lift input gear phase differs by {angle:.6g} radians"
        cj = {j["id"]: j for j in c["joints"]}
        if {j["id"] for j in g["joints"]} != cj.keys():
            return "fail", f"step {g['submitted_step']}: joint identities differ"
        for joint in g["joints"]:
            reference = cj[joint["id"]]
            if joint["anchor_constrained"] != reference["anchor_constrained"]:
                return "fail", f"step {g['submitted_step']}: joint type mismatch"
            # Extra anchor error budget = 10 * the engine's 0.005m linear slop.
            # Reference error preserves intentional soft-joint deflection.
            if joint["anchor_constrained"] and joint["anchor_error"] > reference["anchor_error"] + 0.05:
                return "fail", f"step {g['submitted_step']} joint {joint['id']}: anchor error {joint['anchor_error']:.6g} vs CPU {reference['anchor_error']:.6g}"
            if joint["angular_constrained"] != reference["angular_constrained"]:
                return "fail", f"step {g['submitted_step']}: angular constraint type mismatch"
            # Explicit screening budget: 0.05 radians beyond matched CPU error.
            if joint["angular_constrained"] and joint["angular_error"] > reference["angular_error"] + 0.05:
                return "fail", f"step {g['submitted_step']} joint {joint['id']}: angular error {joint['angular_error']:.6g} vs CPU {reference['angular_error']:.6g} radians"
        if not gear_support_checked and g["min_y"] < c["min_y"] - 2.0:
            return "fail", f"step {g['submitted_step']}: downward drift gpu={g['min_y']} cpu={c['min_y']}"
        status, detail = compare_frame(g, c, scene_id)
        if status != "ok":
            return status, f"step {g['submitted_step']}: {detail}"
    if gear_support_checked:
        return "ok", "all-frame identities, joint errors, speed/launch bounds and complete hull/solid support vs CPU"
    return "ok", "all-frame CPU screening envelope (not constraint/support certification)"


def compare_frame(gpu: dict, cpu: dict, scene_id: str) -> tuple[str, str]:
    gpu = {"frames": [gpu]}
    cpu = {"frames": [cpu]}

    gpu_min = envelope(gpu, "min_y")
    cpu_min = envelope(cpu, "min_y")
    gpu_speed = envelope(gpu, "max_speed")
    cpu_speed = envelope(cpu, "max_speed")
    gpu_max = envelope(gpu, "max_y")
    cpu_max = envelope(cpu, "max_y")
    if not gpu_min or not cpu_min:
        return "fail", "missing min_y envelopes"

    last_gpu_min = gpu_min[-1]
    last_cpu_min = cpu_min[-1]
    last_gpu_speed = gpu_speed[-1] if gpu_speed else 0.0
    last_cpu_speed = cpu_speed[-1] if cpu_speed else 0.0
    last_gpu_max = gpu_max[-1] if gpu_max else 0.0
    last_cpu_max = cpu_max[-1] if cpu_max else 0.0

    if scene_id == "Joints/Ball and Chain":
        if abs(last_gpu_min - last_cpu_min) > 3.0:
            return "fail", f"chain hang min_y gpu={last_gpu_min:.3f} cpu={last_cpu_min:.3f}"
        if last_gpu_speed > max(8.0, last_cpu_speed * 4.0 + 4.0):
            return "fail", f"chain speed gpu={last_gpu_speed:.3f} cpu={last_cpu_speed:.3f}"
        return "ok", "chain position/speed screening vs CPU"
    if scene_id in ("Issues/s&box mover", "Issues/s&box Ghost Collisions"):
        if last_gpu_min < last_cpu_min - 0.75:
            return "fail", f"support lost min_y gpu={last_gpu_min:.3f} cpu={last_cpu_min:.3f}"
        return "ok", "support/penetration vs CPU"
    if scene_id in ("Events/Hit", "Joints/Bridge", "Joints/Gear Lift", "Joints/Motion Locks", "Joints/Driving"):
        if last_gpu_speed > max(12.0, last_cpu_speed * 6.0 + 8.0):
            return "fail", f"impact/joint speed gpu={last_gpu_speed:.3f} cpu={last_cpu_speed:.3f}"
        if last_gpu_max > last_cpu_max + 12.0:
            return "fail", f"launch max_y gpu={last_gpu_max:.3f} cpu={last_cpu_max:.3f}"
        return "ok", "velocity/pose envelope vs CPU"
    if scene_id in ("Determinism/Wave Pile", "Continuous/Mesh Drop"):
        if last_gpu_min < last_cpu_min - 1.0:
            return "fail", f"mesh support min_y gpu={last_gpu_min:.3f} cpu={last_cpu_min:.3f}"
        if last_gpu_speed > max(16.0, last_cpu_speed * 5.0 + 8.0):
            return "fail", f"mesh speed gpu={last_gpu_speed:.3f} cpu={last_cpu_speed:.3f}"
        return "ok", "mesh rest envelope vs CPU"
    if last_gpu_min < last_cpu_min - 2.0:
        return "fail", f"slow sink min_y gpu={last_gpu_min:.3f} cpu={last_cpu_min:.3f}"
    return "ok", "CPU envelope"


def classify(
    gpu: dict | None,
    cpu: dict | None,
    scene_id: str,
    expected_timed: int,
    process_rc: int,
    log_text: str,
    gpu_fail: bool,
    cpu_process_rc: int = 0,
) -> dict:
    if "memory allocation of" in log_text:
        return {"status": "fail", "detail": "process tried an oversized allocation", "criterion": "containment"}
    if gpu_fail or "refusing compound" in log_text or "GPU PHYSICS FAILED" in log_text or "GPU FAILED" in log_text:
        if scene_id == "Compound/Village":
            return {
                "status": "unsupported",
                "detail": "compound/scene capacity rejected before a successful Village simulate",
                "criterion": "unsupported capacity",
            }
        return {"status": "unsupported", "detail": "GPU failed/rejected the scene", "criterion": "unsupported"}
    if process_rc == 124:
        return {"status": "fail", "detail": "timeout", "criterion": "process"}
    if process_rc != 0:
        return {"status": "fail", "detail": f"exit {process_rc}", "criterion": "process"}
    if gpu is None:
        return {"status": "fail", "detail": "missing gpu json", "criterion": "record"}
    if cpu_process_rc != 0:
        return {"status": "fail", "detail": f"CPU exit {cpu_process_rc}", "criterion": "reference process"}
    status, detail = record_complete(gpu, scene_id, expected_timed)
    if status != "ok":
        return {"status": status if status != "ok" else "fail", "detail": detail, "criterion": "record"}
    if scene_id == "Compound/Village":
        # Capacity can double when the post-loading test sphere is added. Require
        # the actual packed geometry count, not an exact allocation bucket size.
        inputs = re.findall(r"gpu-scene-input shapes=(\d+) mesh_vertices=(\d+) mesh_triangles=(\d+) scene_heap_bytes=(\d+)", log_text)
        if not any(int(shapes) == 52502 and int(vertices) > 0 and int(triangles) > 0
                   and 0 < int(size) <= 512 * 1024 * 1024
                   for shapes, vertices, triangles, size in inputs):
            return {"status": "fail", "detail": "missing full Village compound allocation evidence", "criterion": "geometry"}
    health_status, health_detail = generic_health(gpu)
    if health_status != "ok":
        return {"status": health_status, "detail": health_detail, "criterion": "generic health"}
    envelope_status, envelope_detail = compare_cpu(gpu, cpu, scene_id)
    if envelope_status == "ok":
        envelope_status = "pass"
    return {"status": envelope_status, "detail": envelope_detail, "criterion": "cpu envelope"}


def self_check() -> None:
    missing = classify(None, None, "Joints/Bridge", 10, 0, "", False)
    assert missing["status"] == "fail"
    truncated = {
        "status": "ok",
        "sample": "Joints/Bridge",
        "timed": 10,
        "measured": 3,
        "frames": [{"i": 0, "exploded": False, "min_y": 1, "max_y": 2, "max_speed": 1, "nan_count": 0}],
    }
    bad = classify(truncated, None, "Joints/Bridge", 10, 0, "", False)
    assert bad["status"] in ("fail", "incomplete")
    exploded = {
        "status": "ok",
        "sample": "Events/Hit",
        "timed": 1,
        "measured": 1,
        "frames": [{"i": 0, "exploded": True, "min_y": 0, "max_y": 1, "max_speed": 9, "nan_count": 0}],
    }
    boom = classify(exploded, exploded, "Events/Hit", 1, 0, "", False)
    assert boom["status"] == "fail"
    village = classify({"status": "ok", "sample": "Compound/Village", "timed": 1, "measured": 1, "frames": []}, None, "Compound/Village", 1, 0, "refusing compound with 52500 children", True)
    assert village["status"] == "unsupported"
    stale_identity = {
        "status": "ok",
        "sample": "Other",
        "timed": 1,
        "measured": 1,
        "frames": [{"exploded": False, "min_y": 1, "max_y": 1, "max_speed": 0, "nan_count": 0}],
    }
    ident = classify(stale_identity, None, "Joints/Bridge", 1, 0, "", False)
    assert ident["status"] == "fail"
    import copy
    valid = dict(status="ok", sample="Joints/Bridge", timed=2, measured=2,
                 health_scan=True, warmup=0, worker_count=1, enable_sleep=True,
                 unpaced=True, completed_step_mode=False,
                 frames=[dict(i=i, submitted_step=i+1, min_y=0, max_y=1,
                              max_speed=0, nan_count=0, body_count=2,
                              joint_count=0, joints=[], exploded=False,
                              bodies=[dict(id=1,p=[0,0,0],q=[0,0,0,1],v=[0,0,0],w=[0,0,0])]) for i in range(2)])
    assert classify(valid, valid, "Joints/Bridge", 2, 0, "", False)["status"] == "pass"
    for key, value in (("min_y", -100), ("max_speed", float("nan")),
                       ("submitted_step", 9), ("max_speed", None)):
        bad = copy.deepcopy(valid)
        bad["frames"][0][key] = value  # valid final frame must not hide the fault
        assert classify(bad, valid, "Joints/Bridge", 2, 0, "", False)["status"] == "fail"
    assert classify(valid, valid, "Joints/Bridge", 2, 0, "", False, 1)["status"] == "fail"
    diagnostic = copy.deepcopy(valid)
    diagnostic["gear_diagnostic"] = 1
    assert classify(diagnostic, valid, "Joints/Bridge", 2, 0, "", False)["status"] == "fail"
    assert classify(valid, diagnostic, "Joints/Bridge", 2, 0, "", False)["status"] == "incomplete"
    village = copy.deepcopy(valid)
    village.update(sample="Compound/Village", village_drop=True, timed=600, measured=600)
    village["frames"] = []
    for i in range(600):
        frame = copy.deepcopy(valid["frames"][0])
        y = 30.0 - (30.0 - 2.19756746) * min(i / 300.0, 1.0)
        frame.update(i=i, submitted_step=i+1, min_y=y, max_y=y)
        frame["bodies"][0].update(p=[0,y,0], v=[0,-15 if i < 300 else 0,0])
        village["frames"].append(frame)
    village_log = "gpu-scene-input shapes=52502 mesh_vertices=2678 mesh_triangles=4370 scene_heap_bytes=12257696"
    assert classify(village, village, "Compound/Village", 600, 0, village_log, False)["status"] == "pass"
    assert classify(village, village, "Compound/Village", 600, 0, "", False)["status"] == "fail"
    assert classify(village, village, "Compound/Village", 600, 0, village_log.replace("52502", "2"), False)["status"] == "fail"
    grown_log = "gpu-alloc scene generation=0 shapes=105002 scene_heap_bytes=24257696\n" + village_log.replace("12257696", "24257696")
    assert classify(village, village, "Compound/Village", 600, 0, grown_log, False)["status"] == "pass"
    assert classify(village, village, "Compound/Village", 600, 0, grown_log.splitlines()[0], False)["status"] == "fail"
    assert classify(village, village, "Compound/Village", 600, 0, village_log.replace("mesh_triangles=4370", "mesh_triangles=0"), False)["status"] == "fail"
    for mutation in ("missing_fixture", "empty", "sink", "drift", "never_falls"):
        bad = copy.deepcopy(village)
        if mutation == "missing_fixture": bad.pop("village_drop")
        if mutation == "empty": bad["frames"][0]["bodies"] = []
        if mutation == "sink": bad["frames"][-1]["bodies"][0]["p"][1] = -20
        if mutation == "drift": bad["frames"][100]["bodies"][0]["p"][0] = 0.01
        if mutation == "never_falls":
            for frame in bad["frames"]: frame["bodies"][0]["v"] = [0,0,0]
        assert classify(bad, village, "Compound/Village", 600, 0, village_log, False)["status"] == "fail", mutation
    print("native_scene_validate self-check ok")


def main() -> int:
    if len(sys.argv) > 1 and sys.argv[1] == "--self-check":
        self_check()
        return 0
    payload = json.loads(sys.stdin.read())
    result = classify(
        payload.get("gpu"),
        payload.get("cpu"),
        payload["scene_id"],
        int(payload["expected_timed"]),
        int(payload.get("process_rc", 0)),
        payload.get("log_text", ""),
        bool(payload.get("gpu_fail", False)),
        int(payload.get("cpu_process_rc", 0)),
    )
    json.dump(result, sys.stdout)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
