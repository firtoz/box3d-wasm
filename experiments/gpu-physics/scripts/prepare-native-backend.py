#!/usr/bin/env python3
"""Prepare fingerprint-checked private crates; never mutate Cargo's registry."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('output', nargs='?', type=Path, default=root / 'target/native-backend')
args = parser.parse_args()
out = args.output.resolve()
patches = root / 'compiler/native-backend'
registry = Path(os.environ.get('CARGO_HOME', Path.home() / '.cargo')) / 'registry/src'
manifest = json.loads((patches / 'manifest.json').read_text())
hook = json.loads((patches / 'backend-hook.json').read_text())
queue_hooks = json.loads((patches / 'queue-hooks.json').read_text())
physics_replay = json.loads((patches / 'physics-replay.json').read_text())

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

out.mkdir(parents=True, exist_ok=True)
for item in manifest:
    dest = out / item['name']
    if not dest.exists():
        sources = registry.glob('*/' + item['name'] + '-' + item['version'])
        source = next((s for s in sources if all(digest(s / f) == h['before'] for f, h in item['files'].items())), None)
        if source is None:
            raise SystemExit('Missing pinned dependency or fingerprint mismatch: ' + item['name'] + '; run cargo fetch first')
        shutil.copytree(source, dest)
        subprocess.run(['patch', '--batch', '-p1', '-i', str(patches / item['patch'])], cwd=dest, check=True)
    for f, hashes in item['files'].items():
        allowed = {hashes['after']}
        if str(Path(item['name']) / f) == hook['file']:
            allowed.add(hook['after_sha256'])
        queue_hashes = queue_hooks.get(str(Path(item['name']) / f))
        if queue_hashes:
            allowed.add(queue_hashes['after'])
        replay_hashes = physics_replay.get(str(Path(item['name']) / f))
        if replay_hashes:
            allowed.add(replay_hashes['after'])
        if digest(dest / f) not in allowed:
            raise SystemExit('Unexpected backend source: ' + str(dest / f) + '; use a fresh output directory')
f = out / hook['file']
if digest(f) == hook['before_sha256']:
    subprocess.run(['patch', '--batch', '-p1', '-i', str(patches / 'backend-hook.patch')], cwd=out / 'wgpu-hal', check=True)
hook_allowed = {hook['after_sha256']}
if hook['file'] in physics_replay:
    hook_allowed.add(physics_replay[hook['file']]['after'])
if digest(f) not in hook_allowed:
    raise SystemExit('Compiler hook fingerprint mismatch')
queue_states = {name: digest(out / name) for name in queue_hooks}
if all(queue_states[name] == hashes['before'] for name, hashes in queue_hooks.items()):
    subprocess.run(['patch', '--batch', '-p1', '-i', str(patches / 'queue-hooks.patch')], cwd=out / 'wgpu-hal', check=True)
for name, hashes in queue_hooks.items():
    allowed = {hashes['after']}
    if name in physics_replay:
        allowed.add(physics_replay[name]['after'])
    if digest(out / name) not in allowed:
        raise SystemExit('Queue hook fingerprint mismatch: ' + name)
replay_states = {name: digest(out / name) for name in physics_replay}
if all(replay_states[name] == hashes['before'] for name, hashes in physics_replay.items()):
    subprocess.run(['patch', '--batch', '-p1', '-i', str(patches / 'physics-replay.patch')], cwd=out, check=True)
for name, hashes in physics_replay.items():
    if digest(out / name) != hashes['after']:
        raise SystemExit('Physics replay fingerprint mismatch: ' + name)
cargo = out / 'wgpu-hal/Cargo.toml'
contents = cargo.read_text()
section = '\n[dependencies.gpu-spirv-layout]\n'
contents = contents.split(section)[0]
cargo.write_text(contents + section + 'path = ' + json.dumps(str(root / 'compiler/spirv-layout')) + '\n')
config = out / 'cargo-config.toml'
config.write_text('[patch.crates-io]\n' + ''.join(item['name'] + ' = { path = ' + json.dumps(str(out / item['name'])) + ' }\n' for item in manifest))
print(config)
