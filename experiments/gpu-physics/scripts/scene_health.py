"""Independently check recorder limits against the shared audited native setup."""
from pathlib import Path
import math
import re
import struct

POLICY = 'native-scene-outlier-screen-v1'
SOURCE = Path(__file__).resolve().parent.parent / 'native-samples/scene_health_contracts.inc'
PATTERN = re.compile(r'^SCENE_HEALTH_CONTRACT\("([^"]+)", ([0-9.]+)f, ([0-9.]+)f, ([0-9.]+)f, (GENERIC|BALLISTIC|WORLD_CAP)\)$')
CONTRACTS = {}
for line in SOURCE.read_text().splitlines():
    if not line or line.startswith('//'):
        continue
    match = PATTERN.fullmatch(line)
    if not match:
        raise ValueError('Invalid native health contract: ' + line)
    name, reference, launch, drop, rule = match.groups()
    if name in CONTRACTS:
        raise ValueError('Duplicate native health contract: ' + name)
    CONTRACTS[name] = (float(reference),float(launch),float(drop),rule)


def f32(value):
    return struct.unpack('f',struct.pack('f',value))[0]


def limits(sample, world_speed):
    reference,launch,drop,rule = CONTRACTS.get(sample,(0.0,180.0 if sample=='Continuous/Bounce House' else 120.0,0.0,'GENERIC'))
    ceiling,rise = launch,200.0
    if rule=='BALLISTIC':
        ceiling=math.sqrt(launch*launch+2*10*drop)+0.05
        rise=max(rise,launch*launch/(2*10))
    elif rule=='WORLD_CAP':
        if type(world_speed) not in (int,float) or not math.isfinite(world_speed) or world_speed<=0:
            raise ValueError('Stall requires actual positive finite world speed limit')
        ceiling=world_speed+0.05
        rise=max(rise,world_speed*world_speed/(2*10))
    return {'health_reference_y':f32(reference),'health_speed_limit':f32(ceiling),'health_max_y_limit':f32(reference+rise)}


def checked_limits(frame, sample):
    if frame.get('health_policy')!=POLICY:
        raise ValueError('Unknown or missing health policy')
    if type(frame.get('raw_exploded')) is not bool:
        raise ValueError('Missing retained raw explosion heuristic')
    world_speed=frame.get('world_maximum_linear_speed')
    if type(world_speed) not in (int,float) or not math.isfinite(world_speed) or world_speed<=0:
        raise ValueError('Missing or invalid actual world speed limit')
    expected=limits(sample,frame.get('world_maximum_linear_speed'))
    for key,value in expected.items():
        observed=frame.get(key)
        if type(observed) not in (int,float) or not math.isfinite(observed) or abs(observed-value)>1e-5:
            raise ValueError('Unverified scene health bound: '+key)
    return expected
