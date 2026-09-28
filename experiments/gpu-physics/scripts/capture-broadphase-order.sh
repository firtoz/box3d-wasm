#!/usr/bin/env bash
# Generate independent native input/events and expected traversal sequences.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/broadphase-order-reference}"
mkdir -p "$OUT"
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release
cmake --build oracle/build --target box3d -j4
for name in human determinism utils; do
    cc -O2 -DNDEBUG -ffunction-sections -fdata-sections -I ../../box3d/include -I ../../box3d/shared \
        -c "../../box3d/shared/$name.c" -o "$OUT/$name.o"
done
cc -O2 -DNDEBUG -I ../../box3d/include -I ../../box3d/src \
    -c c_abi/broadphase_tree_reference.c -o "$OUT/tree.o"
g++ -O2 -std=c++17 -Wl,--gc-sections c_abi/ragdoll_reference.cpp "$OUT/tree.o" \
    "$OUT/human.o" "$OUT/determinism.o" "$OUT/utils.o" -I ../../box3d/include -I ../../box3d/shared \
    -Wl,--wrap=b3World_Step -Wl,--wrap=b3UpdateBroadPhasePairs -Wl,--wrap=b3CreateContact \
    -Wl,--wrap=b3DynamicTree_EnlargeProxy -Wl,--wrap=b3DynamicTree_MoveProxy -Wl,--wrap=b3DynamicTree_Rebuild \
    oracle/build/box3d-build/src/libbox3d.a -lpthread -lm -o "$OUT/cpu"
"$OUT/cpu" ragdolls 167 > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import struct, sys
root = Path(sys.argv[1])
output = bytearray(b'B3TO' + struct.pack('<I', 1))
leaves = []
frame = 0
queries = 0
def flush():
    global queries
    if leaves:
        output.extend(struct.pack('<III', 3, frame, len(leaves)))
        output.extend(struct.pack('<' + 'I' * len(leaves), *leaves))
        leaves.clear()
        queries += 1
for line in (root / 'cpu.log').read_text().splitlines():
    fields = line.split()
    if not fields:
        continue
    kind = fields[0]
    if kind == 'leaf':
        leaves.append(int(fields[3]))
        continue
    if kind == 'tree-move':
        raise AssertionError('fixture format must be extended for reinsertion events')
    if kind not in ('tree-initial', 'tree-enlarge', 'tree-rebuild', 'query-begin'):
        continue
    flush()
    if kind == 'query-begin':
        frame = int(fields[1])
    elif kind == 'tree-rebuild':
        output.extend(struct.pack('<II', 2, int(fields[2])))
    else:
        output.extend(struct.pack('<II6f', 0 if kind == 'tree-initial' else 1,
                                  int(fields[3]), *map(float, fields[4:10])))
flush()
assert queries == 167
(root / 'broadphase_tree_updates.bin').write_bytes(output)
print(f'Captured {queries} native query orders in {len(output)} bytes')

# A separate fixture validates the creation comparator using complete native
# moved lists and tree traversal metadata, including non-dynamic proxies.
from collections import defaultdict
frames = defaultdict(lambda: {'trees': defaultdict(list), 'moved': [], 'pairs': []})
for line in (root / 'cpu.log').read_text().splitlines():
    fields = line.split()
    if not fields or fields[0] not in ('pair-leaf', 'moved', 'pair-created'):
        continue
    record = frames[int(fields[1])]
    if fields[0] == 'pair-leaf':
        key, shape = map(int, fields[2:])
        record['trees'][key & 3].append((key, shape))
    elif fields[0] == 'moved':
        record['moved'].append(int(fields[2]))
    else:
        record['pairs'].append(tuple(map(int, fields[2:])))
output = bytearray(b'B3PO' + struct.pack('<I', 1))
pair_count = 0
for frame, record in sorted(frames.items()):
    moves = {key: rank for rank, key in enumerate(record['moved'])}
    assert len(moves) == len(record['moved'])
    metadata = {}
    for leaves in record['trees'].values():
        for rank, (key, shape) in enumerate(reversed(leaves)):
            metadata[shape] = (key, moves.get(key, 0xffffffff), rank)
    pairs = record['pairs']
    output.extend(struct.pack('<II', frame, len(pairs)))
    for a, b in pairs:
        output.extend(struct.pack('<8I', a, b, *metadata[a], *metadata[b]))
    pair_count += len(pairs)
assert len(frames) == 167
(root / 'broadphase_pair_order.bin').write_bytes(output)
print(f'Captured {pair_count} native contact creations across {len(frames)} frames')
PY
