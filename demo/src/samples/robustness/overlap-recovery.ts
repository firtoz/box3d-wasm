import * as THREE from "three";
import { BodyType, type BodyId, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import {
  buildOverlapRecoveryDynamicBodies,
  defaultOverlapRecoveryParams,
  overlapRecoveryCamera,
  overlapRecoveryGroundSize,
} from "./overlap-recovery-scene";

export const overlapRecoverySample: DemoSample = {
  id: "robustness/overlap-recovery",
  name: "Robustness / Overlap Recovery",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, overlapRecoveryGroundSize(), [0, -1, 0], 0x222222, true);
    const params = defaultOverlapRecoveryParams();
    const dynamic: DemoBody[] = [];

    const addDynamics = (handles: BodyId[]) => {
      const size = 2 * params.extent;
      for (const handle of handles) {
        const mesh = new THREE.Mesh(
          new THREE.BoxGeometry(size, size, size),
          new THREE.MeshStandardMaterial({ color: 0x60a5fa, roughness: 0.75 }),
        );
        mesh.castShadow = true;
        mesh.receiveShadow = true;
        scene.add(mesh);
        const body: DemoBody = { handle, mesh, type: BodyType.Dynamic };
        bodies.push(body);
        dynamic.push(body);
      }
    };

    const rebuild = () => {
      for (const body of dynamic) world.destroyBody(body.handle);
      disposeBodies(scene, dynamic);
      dynamic.length = 0;
      bodies.length = 1;
      addDynamics(buildOverlapRecoveryDynamicBodies(world, runtime, params));
    };
    addDynamics(buildOverlapRecoveryDynamicBodies(world, runtime, params));

    return {
      world,
      bodies,
      camera: overlapRecoveryCamera,
      info: "bodies starting overlapping; contact tuning drives recovery",
      controls: [
        { key: "extent", label: "Extent", type: "range", min: 0.1, max: 1, step: 0.1, value: params.extent, onChange: (v) => { if (typeof v === "number") { params.extent = v; rebuild(); } } },
        { key: "base-count", label: "Base Count", type: "range", min: 1, max: 10, step: 1, value: params.baseCount, onChange: (v) => { if (typeof v === "number") { params.baseCount = Math.round(v); rebuild(); } } },
        { key: "overlap", label: "Overlap", type: "range", min: 0, max: 1, step: 0.01, value: params.overlap, onChange: (v) => { if (typeof v === "number") { params.overlap = v; rebuild(); } } },
        { key: "speed", label: "Speed", type: "range", min: 0, max: 10, step: 0.1, value: params.speed, onChange: (v) => { if (typeof v === "number") { params.speed = v; rebuild(); } } },
        { key: "hertz", label: "Hertz", type: "range", min: 0, max: 240, step: 1, value: params.hertz, onChange: (v) => { if (typeof v === "number") { params.hertz = v; rebuild(); } } },
        { key: "damping", label: "Damping Ratio", type: "range", min: 0, max: 20, step: 0.1, value: params.dampingRatio, onChange: (v) => { if (typeof v === "number") { params.dampingRatio = v; rebuild(); } } },
        { key: "reset", label: "Reset Scene", type: "button", onClick: () => rebuild() },
      ],
      step() { syncBodies(world, bodies); },
      dispose() {
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
