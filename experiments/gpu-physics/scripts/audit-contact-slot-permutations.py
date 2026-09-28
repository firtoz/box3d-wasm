#!/usr/bin/env python3
"""Diagnostic contact-storage equivalence; does not change qualification comparators."""
import argparse
import gzip
import importlib.util
import json
from pathlib import Path

spec = importlib.util.spec_from_file_location('core', Path(__file__).with_name('compare-core-state.py'))
core = importlib.util.module_from_spec(spec)
spec.loader.exec_module(core)
key = lambda value: json.dumps(value, sort_keys=True, separators=(',', ':'))


def storage_view(frame):
    frame = core.audited_storage_view(frame)
    allocation = frame['contact_allocation']
    slots = allocation.get('slots')
    high = allocation.get('high_water')
    if type(high) is not int or high < 0 or not isinstance(slots, list) or len(slots) != high:
        raise ValueError('invalid contact allocation span')
    live, pairs = {}, {}
    for slot in slots:
        if not isinstance(slot, dict) or set(slot) != {'contact', 'generation'}:
            raise ValueError('unexpected allocation record fields')
        generation = slot['generation']
        if type(generation) is not int or not 0 <= generation <= 0xffffffff:
            raise ValueError('invalid contact generation')
        identity = slot['contact']
        if identity is None:
            continue
        if (not isinstance(identity, list) or len(identity) != 2
                or not isinstance(identity[0], list) or len(identity[0]) != 2
                or any(type(i) is not int or i < 0 for i in identity[0])
                or type(identity[1]) is not int or identity[1] < 0):
            raise ValueError('invalid semantic contact identity')
        identity_key = key(identity)
        if identity_key in live:
            raise ValueError('duplicate live contact identity')
        live[identity_key] = generation
        pairs.setdefault(key(identity[0]), []).append(identity[1])
    contact_keys = [key(c['identity']) for c in frame['contacts']]
    if len(set(contact_keys)) != len(contact_keys) or set(contact_keys) != set(live):
        raise ValueError('allocation/contact identity mismatch')
    for pair, ordinals in pairs.items():
        if sorted(ordinals) != list(range(len(ordinals))):
            raise ValueError('missing root or child ordinal')
        root_generation = live[key([json.loads(pair), 0])]
        if any(live[key([json.loads(pair), i])] != root_generation for i in ordinals):
            raise ValueError('child/root generation mismatch')
    roots = {key([json.loads(pair), 0]) for pair in pairs}
    if {key(i) for i in allocation['occupied_order']} != roots:
        raise ValueError('occupied root membership mismatch')
    # Keep every record, including retired generations and unused slots.
    # All non-allocation fields remain exactly as in the audited membership view.
    return {**frame, 'contact_allocation': {**allocation, 'slots': sorted(slots, key=key)}}


def audit(paths, frames):
    # Validate every frame of each original trace using the existing checker.
    for path in paths:
        result = core.compare([path], frames)
        if result['status'] != 'pass':
            return result
    handles = [gzip.open(p, 'rt') if p.suffix == '.gz' else p.open() for p in paths]
    try:
        for number in range(1, frames + 1):
            docs = [storage_view(json.loads(next(h))) for h in handles]
            for path, doc in zip(paths[1:], docs[1:]):
                difference = core.difference(docs[0], doc)
                if difference:
                    return {'equivalent': False, 'frame': number, 'file': str(path), 'difference': difference}
        return {'equivalent': True, 'frames': frames, 'runs': len(paths)}
    finally:
        for handle in handles:
            handle.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('traces', nargs='+', type=Path)
    parser.add_argument('--frames', required=True, type=int)
    parser.add_argument('--out', required=True, type=Path)
    args = parser.parse_args()
    if len(args.traces) < 2 or args.frames < 1:
        parser.error('at least two traces and positive frame count required')
    result = audit(args.traces, args.frames)
    result['scope'] = 'Diagnostic only: validated slot-record permutations plus previously audited memberships; raw and qualification gates unchanged'
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))
