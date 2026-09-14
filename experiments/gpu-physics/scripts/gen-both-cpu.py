#!/usr/bin/env python3
"""Prefix libbox3d.a symbols with cpu_ and emit both_passthrough.c."""
from __future__ import annotations

import argparse
import re
import subprocess
from pathlib import Path

DUAL = {
    "b3Shape_ComputeMassData",
    "b3Joint_GetLocalFrameA",
    "b3Joint_GetLocalFrameB",
    "b3Joint_SetLocalFrameA",
    "b3Joint_SetLocalFrameB",
    "b3Joint_WakeBodies",

    "b3World_SetContactTuning",
    "b3World_SetUserData",
    "b3World_GetUserData",
    "b3World_GetAwakeBodyCount",
    "b3Body_EnableSleep",
    "b3Body_IsSleepEnabled",
    "b3Body_SetSleepThreshold",
    "b3Body_GetSleepThreshold",
    "b3Body_EnableHitEvents",
    "b3Body_SetName",
    "b3Body_GetName",

    "b3Shape_GetBody",
    "b3Contact_IsValid",
    "b3Contact_GetData",
    "b3Body_GetContactCapacity",
    "b3Body_GetContactData",
    "b3Shape_GetContactCapacity",
    "b3Shape_GetContactData",
    "b3World_SetContactRecycleDistance",
    "b3World_GetContactRecycleDistance",
    "b3CreateWorld",
    "b3DestroyWorld",
    "b3World_IsValid",
    "b3World_Step",
    "b3World_SetCustomFilterCallback",
    "b3World_SetPreSolveCallback",
    "b3World_Explode",
    "b3World_GetContactEvents",
    "b3World_GetJointEvents",
    "b3World_Draw",
    "b3World_GetBounds",
    "b3World_EnableSleeping",
    "b3World_IsSleepingEnabled",
    "b3World_SetGravity",
    "b3World_GetGravity",
    "b3World_EnableContinuous",
    "b3World_IsContinuousEnabled",
    "b3CreateBody",
    "b3DestroyBody",
    "b3Body_EnableContactRecycling",
    "b3Body_IsContactRecyclingEnabled",
    "b3Body_SetBullet",
    "b3Body_IsBullet",
    "b3Body_AllowFastRotation",
    "b3Body_IsFastRotationAllowed",
    "b3CreateSphereShape",
    "b3CreateHullShape",
    "b3CreateTransformedHullShape",
    "b3CreateCapsuleShape",
    "b3CreateMeshShape",
    "b3CreateHeightFieldShape",
    "b3CreateBakedCompoundShape",
    "b3DestroyShape",
    "b3Shape_GetFilter",
    "b3Shape_SetFilter",
    "b3Shape_SetUserData",
    "b3Shape_GetUserData",
    "b3Shape_GetDensity",
    "b3Shape_GetFriction",
    "b3Shape_GetRestitution",
    "b3Shape_SetFriction",
    "b3Shape_SetRestitution",
    "b3Shape_ApplyWind",
    "b3Shape_GetSphere",
    "b3Shape_GetCapsule",
    "b3Shape_GetMesh",
    "b3Shape_GetHeightField",
    "b3Shape_GetMeshMaterialCount",
    "b3Shape_SetMeshMaterial",
    "b3Shape_GetMeshSurfaceMaterial",
    "b3Shape_SetMesh",
    "b3Shape_GetAABB",
    "b3Shape_GetClosestPoint",
    "b3Shape_RayCast",
    "b3Body_ComputeAABB",
    "b3Body_GetClosestPoint",
    "b3Body_CastRay",
    "b3Body_CastShape",
    "b3Body_OverlapShape",
    "b3Body_CollideMover",
    "b3World_OverlapAABB",
    "b3World_OverlapShape",
    "b3World_CastRay",
    "b3World_CastRayClosest",
    "b3World_CastShape",
    "b3World_CastMover",
    "b3World_CollideMover",
    "b3Shape_EnableContactEvents",
    "b3Shape_AreContactEventsEnabled",
    "b3Shape_EnablePreSolveEvents",
    "b3Shape_ArePreSolveEventsEnabled",
    "b3CreateRevoluteJoint",
    "b3Joint_SetConstraintTuning",
    "b3Joint_GetConstraintTuning",
    "b3Joint_SetForceThreshold",
    "b3Joint_GetForceThreshold",
    "b3Joint_SetTorqueThreshold",
    "b3Joint_GetTorqueThreshold",
    "b3Joint_SetUserData",
    "b3Joint_GetUserData",
    "b3RevoluteJoint_EnableSpring",
    "b3RevoluteJoint_IsSpringEnabled",
    "b3RevoluteJoint_SetSpringHertz",
    "b3RevoluteJoint_GetSpringHertz",
    "b3RevoluteJoint_SetSpringDampingRatio",
    "b3RevoluteJoint_GetSpringDampingRatio",
    "b3RevoluteJoint_SetTargetAngle",
    "b3RevoluteJoint_GetTargetAngle",
    "b3RevoluteJoint_GetAngle",
    "b3RevoluteJoint_EnableLimit",
    "b3RevoluteJoint_IsLimitEnabled",
    "b3RevoluteJoint_GetLowerLimit",
    "b3RevoluteJoint_GetUpperLimit",
    "b3RevoluteJoint_SetLimits",
    "b3RevoluteJoint_EnableMotor",
    "b3RevoluteJoint_IsMotorEnabled",
    "b3RevoluteJoint_SetMotorSpeed",
    "b3RevoluteJoint_GetMotorSpeed",
    "b3RevoluteJoint_GetMotorTorque",
    "b3RevoluteJoint_SetMaxMotorTorque",
    "b3RevoluteJoint_GetMaxMotorTorque",
    "b3CreateWheelJoint",
    "b3WheelJoint_EnableSuspension",
    "b3WheelJoint_IsSuspensionEnabled",
    "b3WheelJoint_SetSuspensionHertz",
    "b3WheelJoint_GetSuspensionHertz",
    "b3WheelJoint_SetSuspensionDampingRatio",
    "b3WheelJoint_GetSuspensionDampingRatio",
    "b3WheelJoint_EnableSuspensionLimit",
    "b3WheelJoint_IsSuspensionLimitEnabled",
    "b3WheelJoint_GetLowerSuspensionLimit",
    "b3WheelJoint_GetUpperSuspensionLimit",
    "b3WheelJoint_SetSuspensionLimits",
    "b3WheelJoint_EnableSpinMotor",
    "b3WheelJoint_IsSpinMotorEnabled",
    "b3WheelJoint_SetSpinMotorSpeed",
    "b3WheelJoint_GetSpinMotorSpeed",
    "b3WheelJoint_SetMaxSpinTorque",
    "b3WheelJoint_GetMaxSpinTorque",
    "b3WheelJoint_GetSpinSpeed",
    "b3WheelJoint_GetSpinTorque",
    "b3WheelJoint_EnableSteering",
    "b3WheelJoint_IsSteeringEnabled",
    "b3WheelJoint_SetSteeringHertz",
    "b3WheelJoint_GetSteeringHertz",
    "b3WheelJoint_SetSteeringDampingRatio",
    "b3WheelJoint_GetSteeringDampingRatio",
    "b3WheelJoint_SetMaxSteeringTorque",
    "b3WheelJoint_GetMaxSteeringTorque",
    "b3WheelJoint_EnableSteeringLimit",
    "b3WheelJoint_IsSteeringLimitEnabled",
    "b3WheelJoint_GetLowerSteeringLimit",
    "b3WheelJoint_GetUpperSteeringLimit",
    "b3WheelJoint_SetSteeringLimits",
    "b3WheelJoint_SetTargetSteeringAngle",
    "b3WheelJoint_GetTargetSteeringAngle",
    "b3WheelJoint_GetSteeringAngle",
    "b3WheelJoint_GetSteeringTorque",
    "b3CreateSphericalJoint",
    "b3SphericalJoint_EnableConeLimit",
    "b3SphericalJoint_IsConeLimitEnabled",
    "b3SphericalJoint_GetConeLimit",
    "b3SphericalJoint_SetConeLimit",
    "b3SphericalJoint_GetConeAngle",
    "b3SphericalJoint_EnableTwistLimit",
    "b3SphericalJoint_IsTwistLimitEnabled",
    "b3SphericalJoint_GetLowerTwistLimit",
    "b3SphericalJoint_GetUpperTwistLimit",
    "b3SphericalJoint_SetTwistLimits",
    "b3SphericalJoint_GetTwistAngle",
    "b3SphericalJoint_EnableSpring",
    "b3SphericalJoint_IsSpringEnabled",
    "b3SphericalJoint_SetSpringHertz",
    "b3SphericalJoint_GetSpringHertz",
    "b3SphericalJoint_SetSpringDampingRatio",
    "b3SphericalJoint_GetSpringDampingRatio",
    "b3SphericalJoint_SetTargetRotation",
    "b3SphericalJoint_GetTargetRotation",
    "b3SphericalJoint_EnableMotor",
    "b3SphericalJoint_IsMotorEnabled",
    "b3SphericalJoint_SetMotorVelocity",
    "b3SphericalJoint_GetMotorVelocity",
    "b3SphericalJoint_GetMotorTorque",
    "b3SphericalJoint_SetMaxMotorTorque",
    "b3SphericalJoint_GetMaxMotorTorque",
    "b3CreatePrismaticJoint",
    "b3CreateFilterJoint",
    "b3CreateDistanceJoint",
    "b3DistanceJoint_SetLength",
    "b3DistanceJoint_EnableSpring",
    "b3DistanceJoint_SetSpringForceRange",
    "b3DistanceJoint_SetSpringHertz",
    "b3DistanceJoint_SetSpringDampingRatio",
    "b3DistanceJoint_EnableLimit",
    "b3DistanceJoint_SetLengthRange",
    "b3DistanceJoint_EnableMotor",
    "b3DistanceJoint_SetMotorSpeed",
    "b3DistanceJoint_SetMaxMotorForce",
    "b3CreateParallelJoint",
    "b3ParallelJoint_SetSpringHertz",
    "b3ParallelJoint_SetSpringDampingRatio",
    "b3ParallelJoint_SetMaxTorque",
    "b3PrismaticJoint_EnableSpring",
    "b3PrismaticJoint_SetSpringHertz",
    "b3PrismaticJoint_SetSpringDampingRatio",
    "b3PrismaticJoint_SetTargetTranslation",
    "b3PrismaticJoint_EnableLimit",
    "b3PrismaticJoint_SetLimits",
    "b3PrismaticJoint_EnableMotor",
    "b3PrismaticJoint_SetMotorSpeed",
    "b3PrismaticJoint_SetMaxMotorForce",
    "b3CreateWeldJoint",
    "b3WeldJoint_SetLinearHertz",
    "b3WeldJoint_GetLinearHertz",
    "b3WeldJoint_SetLinearDampingRatio",
    "b3WeldJoint_GetLinearDampingRatio",
    "b3WeldJoint_SetAngularHertz",
    "b3WeldJoint_GetAngularHertz",
    "b3WeldJoint_SetAngularDampingRatio",
    "b3WeldJoint_GetAngularDampingRatio",
    "b3CreateMotorJoint",
    "b3MotorJoint_SetLinearVelocity",
    "b3MotorJoint_GetLinearVelocity",
    "b3MotorJoint_SetAngularVelocity",
    "b3MotorJoint_GetAngularVelocity",
    "b3MotorJoint_SetMaxVelocityForce",
    "b3MotorJoint_GetMaxVelocityForce",
    "b3MotorJoint_SetMaxVelocityTorque",
    "b3MotorJoint_GetMaxVelocityTorque",
    "b3MotorJoint_SetLinearHertz",
    "b3MotorJoint_GetLinearHertz",
    "b3MotorJoint_SetLinearDampingRatio",
    "b3MotorJoint_GetLinearDampingRatio",
    "b3MotorJoint_SetAngularHertz",
    "b3MotorJoint_GetAngularHertz",
    "b3MotorJoint_SetAngularDampingRatio",
    "b3MotorJoint_GetAngularDampingRatio",
    "b3MotorJoint_SetMaxSpringForce",
    "b3MotorJoint_GetMaxSpringForce",
    "b3MotorJoint_SetMaxSpringTorque",
    "b3MotorJoint_GetMaxSpringTorque",
    "b3DestroyJoint",
    "b3Body_GetPosition",
    "b3Body_GetRotation",
    "b3Body_GetTransform",
    "b3Body_GetType",
    "b3Body_GetWorldCenter",
    "b3Body_ApplyMassFromShapes",
    "b3Body_ApplyLinearImpulse",
    "b3Body_ApplyLinearImpulseToCenter",
    "b3Body_ApplyAngularImpulse",
    "b3Body_ApplyForce",
    "b3Body_ApplyForceToCenter",
    "b3Body_ApplyTorque",
    "b3Body_GetMass",
    "b3Body_GetInverseMass",
    "b3Body_GetMassData",
    "b3Body_SetMassData",
    "b3Body_GetLocalPoint",
    "b3Body_GetWorldPoint",
    "b3Body_GetLocalVector",
    "b3Body_GetWorldVector",
    "b3Body_GetWorldPointVelocity",
    "b3Body_GetLocalPointVelocity",
    "b3Body_IsAwake",
    "b3Body_IsEnabled",
    "b3Body_Disable",
    "b3Body_Enable",
    "b3Body_SetMotionLocks",
    "b3Body_GetMotionLocks",
    "b3Body_GetLocalCenter",
    "b3Body_GetLinearDamping",
    "b3Body_GetAngularDamping",
    "b3Body_GetGravityScale",
    "b3Body_GetJointCount",
    "b3Body_GetJoints",
    "b3Body_SetAngularVelocity",
    "b3Body_SetLinearVelocity",
    "b3Body_GetAngularVelocity",
    "b3Body_GetLinearVelocity",
    "b3Body_SetTargetTransform",
    "b3Body_SetType",
    "b3Body_SetTransform",
    "b3Body_SetAwake",
    "b3Body_SetLinearDamping",
    "b3Body_SetAngularDamping",
    "b3Body_SetGravityScale",
    "b3Shape_SetName",
    "b3Shape_GetName",
}

API_RE = re.compile(r"B3_API\s+((?:const\s+)?[\w\s\*]+?)\s+(b3[A-Za-z0-9_]+)\s*\((.*?)\)\s*;", re.S)


def defined_symbols(archive: Path) -> set[str]:
    out = subprocess.check_output(["nm", "-g", "--defined-only", str(archive)], text=True, errors="replace")
    names: set[str] = set()
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1] in "ABCDGRSTVWu":
            names.add(parts[2])
    return names


def parse_headers(include_dir: Path) -> dict[str, tuple[str, str]]:
    headers = [
        include_dir / "box3d/box3d.h",
        include_dir / "box3d/collision.h",
        include_dir / "box3d/math_functions.h",
        include_dir / "box3d/types.h",
        include_dir / "box3d/constants.h",
    ]
    found: dict[str, tuple[str, str]] = {}
    for h in headers:
        text = h.read_text()
        text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
        text = re.sub(r"//.*?$", "", text, flags=re.M)
        for ret, name, args in API_RE.findall(text):
            if "#" in ret or "#" in args or '"' in ret:
                continue
            ret = " ".join(ret.split())
            args = " ".join(args.split())
            if name not in found:
                found[name] = (ret, args)
    return found


def arg_names(args: str) -> list[tuple[str, str]]:
    args = args.strip()
    if args in ("void", ""):
        return []
    out: list[tuple[str, str]] = []
    for part in args.split(","):
        part = part.strip()
        toks = part.replace("*", " * ").split()
        name = toks[-1].lstrip("*")
        typ = part[: part.rfind(name)].strip() if name else part
        out.append((typ, name))
    return out


def remap_call(typ: str, name: str) -> str:
    if "*" in typ:
        return name
    if "b3WorldId" in typ:
        return f"both_cpu_world({name})"
    if "b3BodyId" in typ:
        return f"both_cpu_body({name})"
    if "b3ShapeId" in typ:
        return f"both_cpu_shape({name})"
    if "b3JointId" in typ:
        return f"both_cpu_joint({name})"
    return name


def write_passthrough(path: Path, sigs: dict[str, tuple[str, str]], defined: set[str]) -> None:
    # Keep explicit wrappers authoritative even when the hand-maintained set lags.
    dual_source = (Path(__file__).resolve().parents[1] / "c_abi/both_dual.c").read_text()
    explicit = set(re.findall(r"B3_API\s+[^;{]*?\b(b3\w+)\([^;]*?\)\s*\{", dual_source, re.S))
    names = sorted(n for n in defined if n.startswith("b3") and n in sigs)
    lines = [
        "// Generated by scripts/gen-both-cpu.py — do not edit.",
        '#include "box3d/box3d.h"',
        '#include "box3d/collision.h"',
        '#include "both_ids.h"',
        "",
    ]
    for name in names:
        ret, args = sigs[name]
        lines.append(f"extern {ret} cpu_{name}({args});")
    lines.append("")
    for name in names:
        if name in DUAL or name in explicit:
            continue
        ret, args = sigs[name]
        an = arg_names(args)
        call = ", ".join(remap_call(t, n) for t, n in an)
        ret_kw = "" if ret == "void" else "return "
        arglist = args if args.strip() else "void"
        lines.append(f"B3_API {ret} {name}({arglist})")
        lines.append("{")
        lines.append(f"\t{ret_kw}cpu_{name}({call});")
        lines.append("}")
        lines.append("")
    path.write_text("\n".join(lines))


def prefix_archive(src: Path, dst: Path, objcopy: str, defined: set[str]) -> None:
    map_path = dst.with_suffix(".syms")
    rows = [f"{n} cpu_{n}" for n in sorted(defined) if n.startswith("b3")]
    map_path.write_text("\n".join(rows) + "\n")
    subprocess.check_call([objcopy, f"--redefine-syms={map_path}", str(src), str(dst)])


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("--archive", required=True)
    p.add_argument("--prefixed", required=True)
    p.add_argument("--passthrough", required=True)
    p.add_argument("--include-dir", required=True)
    p.add_argument("--objcopy", default="objcopy")
    args = p.parse_args()
    archive = Path(args.archive)
    defined = defined_symbols(archive)
    sigs = parse_headers(Path(args.include_dir))
    prefix_archive(archive, Path(args.prefixed), args.objcopy, defined)
    write_passthrough(Path(args.passthrough), sigs, defined)


if __name__ == "__main__":
    main()
