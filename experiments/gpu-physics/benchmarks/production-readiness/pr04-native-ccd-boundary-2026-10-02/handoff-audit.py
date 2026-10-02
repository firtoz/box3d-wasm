#!/usr/bin/env python3
"""Static handoff/acceptance audit, using archived bytes only."""
from pathlib import Path
import hashlib
import json
import tarfile

ROOT=Path(__file__).resolve().parent
PARENT=ROOT.parent/'pr04-drag-ccd-boundary-2026-10-02'


def audit():
    names=['shaders/physics/integrate.wgsl','shaders/physics/broadphase.wgsl',
           'shaders/physics/collide.wgsl','src/api/world.rs','src/types.rs']
    with tarfile.open(ROOT/'raw/observer-source-inputs.tar.gz') as tar:
        data={n:tar.extractfile(n).read() for n in names}
    source={n:v.decode() for n,v in data.items()}
    cpu=(PARENT/'raw/reference/original-solver.c').read_bytes()
    integrate=source[names[0]];broadphase=source[names[1]];collide=source[names[2]];world=source[names[3]]
    assert 'let max_motion = max(max_delta, max_velocity * params.step_dt);' in integrate
    assert 'max_motion > min(0.5 * min_extent, SPECULATIVE)' in integrate
    assert 'b.flags = b.flags | FLAG_FAST | FLAG_CCD_NO_HIT;' in integrate
    assert 'select(SPECULATIVE, 0.0, child || (b.flags & FLAG_CCD_NO_HIT) != 0u)' in broadphase
    assert 'let handoff_mask = FLAG_FAST | FLAG_CCD_NO_HIT;' in collide
    assert '((body_a.flags | body_b.flags) & FLAG_FAST) != 0u' in collide
    test=world.split('fn ccd_landing_refreshes_empty_contact_inside_speculative_shell()',1)[1].split('\n    #[test]',1)[0]
    for token in ['bd.position = [0.0, 0.522, 0.0]','[0.0, -2.66667, 0.0]',
                  '(landing - 0.505).abs() < 0.001','next >= 0.495']:
        assert token in test
    assert 'maxMotion > safetyFactor * sim->minExtent' in cpu.decode()
    return dict(
        budget=dict(builds=0,engine_processes=0,candidates=0,timing=0),
        inputs={**{n:hashlib.sha256(v).hexdigest() for n,v in data.items()},
                'parent/raw/reference/original-solver.c':hashlib.sha256(cpu).hexdigest()},
        observed_native_frame227_flags=98312,decoded_flags=['SLEEP_ENABLED=8','FAST=32768','CCD_NO_HIT=65536'],
        source_facts=dict(
            existing_motion_metric='apply_deltas already calculates CPU-style local vector-extent angular motion, maximum velocity and maximum position correction, but applies the speculative-shell cap to its fast classification.',
            flags_are_consumed='FAST/CCD_NO_HIT affect next-step speculative proxy padding, mesh refresh and handoff. The old host-only cutoff rollback leaves the shader classification/flags unchanged.',
            existing_acceptance='ccd_landing_refreshes_empty_contact_inside_speculative_shell uses a unit cube atY.522 with velocityY-2.66667, dt1/60/four substeps, requires firstY.505±.001 and secondY>=.495. Those original requirements remain required.',
            CPU_gate='CPU uses half-minimum-extent threshold with maximum position/velocity motion and only classifies an awake/nonquiet dynamic body. This selected trace body remains below that gate.'),
        limits='Static flags/consumers audit identifies an incomplete rollback and a concrete acceptance constraint. It does not prove why the historical paired control topples late, or prove a proposed repair.',
        next_action='Resolve the low-speed CCD landing versus loaded-joint/contact acceptance interaction with the smallest independent CPU/GPU reproducer before selecting a new candidate. Any candidate must keep classification, proxy flags and both CCD paths consistent, preserve empty-manifold recycling and all original landing/drag/settling/restitution limits. Do not repeat the old host-only cutoff control or assume a consistent cutoff change alone passes.',
        production_candidate_selected=False,PR04_complete=False)


if __name__=='__main__':
    (ROOT/'handoff-audit.json').write_text(json.dumps(audit(),indent=2)+'\n')
    print('Static fast-flag handoff and original landing acceptance audit saved.')
