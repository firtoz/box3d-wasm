"""Completed-step performance validation for the two fixed, unmodified scenes.

This validates measurement integrity, not physical correctness. Contact counts
are scheduling-phase data: CCD can change poses afterward. The current flag must
remain truthful; no contact counts are relabelled as post-CCD results. A state
revision allowance is valid only for these immutable, input-free benchmarks on
an isolated display and the audited Step/Wait/note_world call sequence.
"""
import math

SAMPLES = {'Determinism/Falling Ragdolls', 'Benchmark/Large Pyramid'}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def integer(record, name):
    value = record.get(name)
    require(type(value) is int and value >= 0, f'invalid {name}')
    return value


def scheduling_metrics(row, previous=None):
    m = row['gpu_contact_metrics']
    require(m.get('phase') == 'contact_scheduling', 'wrong metric phase')
    require(m.get('known') is True, 'unknown metrics')
    require(m.get('capacity_loss') is False, 'capacity loss')
    step = integer(row, 'submitted_step')
    require(integer(m, 'snapshot_step') == integer(m, 'submitted_step') == step,
            'old or mismatched metric step')
    topology = integer(m, 'current_topology')
    require(integer(m, 'snapshot_topology') == topology, 'stale topology')
    scheduled = integer(m, 'snapshot_state')
    completed = integer(m, 'current_state')
    delta = completed - scheduled
    # Host CCD increments once iff it corrects poses. Device CCD conservatively
    # reserves one extra revision on submission. Neither changes scheduling data.
    require(delta in (0, 1), 'unexplained state revision gap')
    require(type(m.get('current')) is bool and m['current'] == (delta == 0),
            'incorrect current label or dirty state')
    if previous is not None:
        require(step == previous['submitted_step'] + 1, 'nonconsecutive step')
        require(topology == previous['current_topology'], 'topology changed')
        require(scheduled == previous['current_state'] + 1,
                'unexpected between-step state mutation')
    for key in ('candidate_pairs', 'allocated_roots', 'allocated_manifold_slots',
                'touching_roots', 'non_sensor_roots'):
        integer(m, key)
    return m


def validate(report, *, sample, bodies, joints, sleep, warmup=60, timed=180,
             workers=8, width=320, height=240):
    require(sample in SAMPLES, 'scene has not been audited for input-free stepping')
    expected = dict(status='ok', mode='gpu', sample=sample, warmup=warmup,
                    timed=timed, measured=timed, frames_observed=warmup+timed,
                    completed_step_mode=True, unpaced=True, health_scan=False,
                    pause_script=False, enable_sleep=sleep, worker_count=workers,
                    scene_seed=0)
    for key, value in expected.items():
        require(type(report.get(key)) is type(value) and report[key] == value,
                f'incorrect report {key}')
    require(not report.get('gpu_fail'), 'GPU failure')
    require(report.get('switch_count') == 0, 'scene switched')
    rows = report['frames']
    require(len(rows) == timed, 'truncated timing rows')
    previous = None
    stale_steps = []
    for i, row in enumerate(rows):
        step = warmup + i + 1
        require(row.get('sample') == sample and row.get('i') == i, 'wrong row identity')
        require(row.get('submitted_step') == row.get('completed_step') == step,
                'incomplete or wrong step')
        require(row.get('in_flight') == 0 and row.get('pause') is False
                and row.get('single_step') is False, 'paused or pending work')
        require((row.get('body_count'), row.get('joint_count')) == (bodies, joints),
                'scene counts changed')
        require((row.get('framebuffer_width'), row.get('framebuffer_height')) == (width, height),
                'render dimensions changed')
        require(row.get('health_ms') == 0 and row.get('bodies') == []
                and row.get('joints') == [], 'diagnostic capture present')
        previous = scheduling_metrics(row, previous)
        if not previous['current']:
            stale_steps.append(step)
        ms = row.get('physics_ms')
        require(type(ms) in (int, float) and math.isfinite(ms) and ms > 0,
                'invalid completed-step latency')
    return {'validated_steps': timed, 'post_scheduling_revision_steps': stale_steps,
            'metric_phase': 'contact_scheduling', 'physical_qualification': False}
