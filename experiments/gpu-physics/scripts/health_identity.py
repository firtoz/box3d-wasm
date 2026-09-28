"""Match diagnostic joints using an explicitly established body-lifetime map.

Slot numbers alone are not identities. This module deliberately does not infer
CPU/GPU correspondence from poses or silently pair ambiguous parallel joints.
"""


def body_key(record, prefix=''):
    index = record['id' if not prefix else prefix]
    generation = record['generation' if not prefix else prefix + '_generation']
    if type(index) is not int or index <= 0 or type(generation) is not int or not 0 <= generation <= 65535:
        raise ValueError('invalid body lifetime')
    return index, generation


def match_joints(cpu_joints, gpu_joints, gpu_to_cpu_bodies):
    """Return (CPU, GPU) pairs, refusing missing, stale or ambiguous identities.

    Map keys and values are (body slot, generation), established independently
    from creation/lifetime evidence. A generation is not a universal identifier:
    callers must partition worlds and handle generation wraparound separately.
    Multiple joints of one type between the same oriented endpoints require a
    richer identity record and are rejected, never paired by allocation ID.
    """
    if len(set(gpu_to_cpu_bodies.values())) != len(gpu_to_cpu_bodies):
        raise ValueError('body lifetime map is not one-to-one')

    def indexed(joints, mapping):
        result = {}
        for joint in joints:
            kind = joint['type']
            if type(kind) is not int or not 0 <= kind <= 8:
                raise ValueError('invalid joint type')
            a, b = body_key(joint, 'body_a'), body_key(joint, 'body_b')
            if a == b:
                raise ValueError('joint endpoints are identical')
            if mapping is not None:
                if a not in mapping or b not in mapping:
                    raise ValueError('unmapped body lifetime')
                a, b = mapping[a], mapping[b]
            key = kind, a, b
            if key in result:
                raise ValueError('ambiguous parallel joint identity')
            result[key] = joint
        return result

    cpu, gpu = indexed(cpu_joints, None), indexed(gpu_joints, gpu_to_cpu_bodies)
    if cpu.keys() != gpu.keys():
        raise ValueError('joint topology differs after lifetime mapping')
    return [(cpu[key], gpu[key]) for key in sorted(cpu)]


def creation_body_map(cpu_bodies, gpu_bodies):
    """Map complete live body sets by diagnostic creation ordinal, never slot.

    Inputs must come from matching fresh processes, the same scene creation
    schedule, and the opt-in Human lifetime hook. Zero means uninstrumented and
    is rejected. Each frame is matched independently, so retired bodies cannot
    lend identities to reused slots. This hook currently covers Human bodies.
    """
    def indexed(bodies):
        result, lifetimes = {}, set()
        for body in bodies:
            creation = body.get('creation')
            if type(creation) is not int or creation <= 0:
                raise ValueError('missing positive creation identity')
            lifetime = body_key(body)
            if creation in result or lifetime in lifetimes:
                raise ValueError('duplicate creation or body lifetime')
            result[creation] = lifetime
            lifetimes.add(lifetime)
        return result
    cpu, gpu = indexed(cpu_bodies), indexed(gpu_bodies)
    if cpu.keys() != gpu.keys():
        raise ValueError('live creation sets differ')
    return {gpu[k]: cpu[k] for k in cpu}


def health_to_core_creations(health_frame, state):
    """Resolve Human creation IDs to core IDs in the same captured GPU step.

    Human counters exclude terrain; core counters include all bodies. Reconcile
    them through live public slots AND generations, never a constant offset.
    Health may cover only the instrumented subset of the full world.
    """
    step = state['frame']
    if (health_frame['i'] + 1 != step or
            health_frame['completed_step'] != step):
        raise ValueError('health/core step mismatch')
    allocation = state['body_allocation']
    slots, generations = allocation['slots'], allocation['generations']
    if len(slots) != len(generations):
        raise ValueError('body allocation length mismatch')
    live = [value for value in slots if value is not None]
    identities = [body['identity'] for body in state['bodies']]
    if (any(type(value) is not int or value <= 0 for value in live) or
            len(set(live)) != len(live) or len(set(identities)) != len(identities) or
            set(live) != set(identities)):
        raise ValueError('core body identities do not match live allocation')
    result, seen = {}, set()
    for body in health_frame['bodies']:
        creation = body.get('creation')
        if type(creation) is not int or creation <= 0:
            raise ValueError('missing positive creation identity')
        slot, generation = body_key(body)
        if creation in result or (slot, generation) in seen:
            raise ValueError('duplicate creation or body lifetime')
        if (slot > len(slots) or slots[slot - 1] is None or
                generations[slot - 1] != generation):
            raise ValueError('health references missing or stale core body lifetime')
        result[creation] = slots[slot - 1]
        seen.add((slot, generation))
    return result
