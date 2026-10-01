# Action-file publication race

This first repaired-viewer campaign stops on a harness failure. **Zero complete viewer control passes.** The other three configurations remain unlaunched. The exact binaries come from [fresh build proof](../pr02-world-lifetime-viewer-builds-2026-10-02/README.md); no production source, physical tolerance, default or scheduling change occurs here. No timing runs.

Restart independently changes the GPU world from[1,1]to[1,2] and completes exactly one new single-step. The original reused-root failure remains preserved separately. These observations do not establish complete UI acceptance.

Current screenshot-reviewed coordinates restore all flags111 after restart through19 completed actions/six screenshots. When action20 is published with `Path.write_text`, the reader can see its empty/truncated intermediate file. The driver stops with JSONDecodeError and kills the app; this is not a clean-quit pass. The raw action20 file is valid after the writer finishes. An atomic same-directory temp-file/rename correction must cover both action publication and receipts before further apps. The already passed original held-input behavior/evaluation assertions are unchanged.

This failed campaign stays closed. The next distinct harness qualification retains both this failure and the earlier coordinate miss; no unfavorable record is replaced.

[Raw index](raw-index.json), [protocol](raw/protocol-before-runs.json), [terminal](raw/terminal.json) and actual [app receipt](raw/ordinary-gpu/receipt.json) include commands/environment/input plans, all actions, observer data, screenshot hashes and captured failure. Full diagnostic MP4 and an offline capture addendum are now portable alongside original screenshots; all original raw receipts remain unchanged. These diagnostic Xvfb/Mesa captures cannot establish desktop rendering FPS.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-adaptive-apps-2026-10-02/validate.py
```
