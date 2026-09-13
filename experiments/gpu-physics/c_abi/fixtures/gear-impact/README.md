# Gear Lift cold-start regression inputs

These four fixed states preserve the CPU step-26 and GPU step-45 debris captures from the v17 native Gear Lift investigation. Each file contains a body count followed by body ID, position, quaternion, linear velocity, and angular velocity. They are test inputs, not benchmark reports.

`scripts/prepare-gear-impact-replay.py` copies these inputs and extracts terrain from the upstream sample. The correctness gate must work without the local `artifacts/` archive. The original reports remain in the ignored `artifacts/native-scene-20260910-v17-remap/` directory when available.
