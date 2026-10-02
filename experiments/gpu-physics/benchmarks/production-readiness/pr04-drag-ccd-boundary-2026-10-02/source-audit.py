#!/usr/bin/env python3
"""Audit frozen source bytes; do not infer which path an individual run used."""
from pathlib import Path
import hashlib
import json
import tarfile

ROOT = Path(__file__).resolve().parent
NAMES = ("src/api/world.rs", "src/api/world/state_trace.rs",
         "src/api/world/ccd_state_trace.rs", "src/sim.rs", "src/ccd.rs",
         "shaders/physics/ccd_motion.wgsl", "shaders/physics/ccd_world.wgsl",
         "shaders/physics/ccd_convex.wgsl")


def audit():
    with tarfile.open(ROOT / "raw/reference/Rust-source-inputs.tar.gz") as archive:
        files = {name: archive.extractfile(name).read() for name in NAMES}
    source = {name: data.decode() for name, data in files.items()}
    world = source["src/api/world.rs"]
    ccd = source["src/ccd.rs"]
    sim = source["src/sim.rs"]
    shader = source["shaders/physics/ccd_world.wgsl"]
    assert 'fn configure_convex_ccd(' in world and 'if !w.gpu_ccd_requested { return; }' in world
    assert 'w.post_ccd_pending = !sim.uses_convex_ccd();' in world
    assert 'w.post_ccd_pending && run_continuous_collision(' in world
    assert 'eprintln!("gpu-ccd-trace step=' in world
    assert 'include_str!("../shaders/physics/ccd_convex.wgsl")' in ccd
    assert 'include_str!("../shaders/physics/ccd_world.wgsl")' in ccd
    assert 'entry_point:Some("ccd_correct")' in ccd
    assert 'motion<=min(0.5*body_info.min_extent,0.02)' in shader
    assert 'var fraction=1.0;' in shader and 'fraction=hit.fraction;' in shader
    assert 'if let Some(ccd)=&self.convex_ccd { ccd.capture(&mut enc,&self.bodies); }' in sim
    assert 'if let Some(ccd)=&self.convex_ccd { ccd.correct(&mut enc); }' in sim
    assert 'self.params.joint_count==0' in sim
    assert 'start:self.read_diagnostic_records(&start,0,counts[2])' in sim
    capture = source["src/api/world/ccd_state_trace.rs"]
    assert '"start":start' in capture and 'fraction' not in capture
    return dict(
        budget=dict(engine_processes=0, builds=0, candidates=0, timing=0),
        inputs={name: hashlib.sha256(files[name]).hexdigest() for name in NAMES},
        scope="Static audit of exact compiled source bytes. Eligibility/request/route facts are not runtime proof of a particular run's native CCD execution or fraction.",
        observations=dict(
            host_observer="GPU_PHYSICS_TRACE_BODY prints only in run_continuous_collision; native convex correction has no equivalent print.",
            convex_setup="Requested CCD additionally requires continuous enabled, no custom/pre-solve callbacks, no bullet and supported CCD shapes. The captured environment alone cannot prove actual eligibility.",
            pending_route="uses_convex_ccd makes post_ccd_pending false; host continuous solving is gated by post_ccd_pending. A configured convex pass therefore bypasses the host print.",
            GPU_program="ConvexCcd combines ccd_convex.wgsl + ccd_world.wgsl and dispatches ccd_correct. The production correction classifies motion inline; standalone ccd_motion.wgsl is not the classifier called by this ConvexCcd path.",
            GPU_classification="ccd_world uses endpoint translation + quaternion angle * max_extent versus min(half min_extent,0.02). Fraction is a local shader variable; it is not retained in an output buffer.",
            dispatch="The non-idle encoded sequence copies start before solver work, then dispatches convex correction after solver/sleep work. Full native replay additionally requires zero joints; original loaded dragging has a live motor during the selected impact.",
            existing_capture="read_diagnostic_ccd_scene captures separate geometry/config/start buffers. It does not preserve the pre-correction endpoint or local TOI fraction. Completed body records contain the post-correction endpoint.",
            phase_capture="Changing the selected AB policy to phase-capture changes diagnostic flags and replay eligibility. It must not be treated as the original native run without a separate applicability/neutrality check."),
        next_experiment="A separate finite artifact-only native observer could retain motion/cutoff/start/pre-end/fraction in a dedicated non-physics GPU buffer inside ccd_correct, then read it after normal completion. Also record actual convex presence/eligibility. Preserve original mode/cache requests and require exact original B/F/M/P prefix equality before attribution. Freeze source/build/binary inputs and budgets before build/launch; do not extend this closed campaign.",
        runtime_internal_native_TOI_observed=False, production_candidate_selected=False)


if __name__ == "__main__":
    (ROOT / "source-audit.json").write_text(json.dumps(audit(), indent=2) + "\n")
    print("Static native CCD route/capture audit saved; no runtime fraction claimed.")
