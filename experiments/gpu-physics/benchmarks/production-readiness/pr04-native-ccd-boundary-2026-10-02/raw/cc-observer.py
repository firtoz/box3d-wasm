#!/usr/bin/env python3
"""Run the original C compiler; retain actual compile commands/dependencies."""
from pathlib import Path
import fcntl
import hashlib
import json
import os
import subprocess
import sys

A = Path(__file__).resolve().parent
args = sys.argv[1:]
compile_unit = '-c' in args and '-o' in args
object_path = Path(args[args.index('-o') + 1]).resolve() if compile_unit else None
dependency_path = Path(str(object_path) + '.observer.d') if compile_unit else None
command = ['/usr/bin/cc'] + args
if compile_unit:
    command += ['-MD', '-MF', str(dependency_path)]
result = subprocess.run(command)
row = dict(command=command, cwd=str(Path.cwd()), exit=result.returncode, compile_unit=compile_unit)
if compile_unit:
    row.update(object=str(object_path), dependency_file=str(dependency_path))
    if result.returncode == 0:
        row['object_sha256'] = hashlib.sha256(object_path.read_bytes()).hexdigest()
        names = dependency_path.read_text().replace('\\\n', ' ').split(':', 1)[1].split()
        row['dependencies'] = {str(Path(n).resolve()): hashlib.sha256(Path(n).read_bytes()).hexdigest() for n in names}
with (A / 'C-compiler-invocations.jsonl').open('a') as log:
    fcntl.flock(log.fileno(), fcntl.LOCK_EX)
    log.write(json.dumps(row) + '\n')
    log.flush()
sys.exit(result.returncode)
