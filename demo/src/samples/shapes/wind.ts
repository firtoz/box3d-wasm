import * as THREE from "three";
import { BodyType, type BodyId, type Box3DRuntime } from "box3d-wasm";
import { createDebugLine, disposeDebugObject, updateDebugLine } from "../debug-overlay";
import type { DemoBody, DemoSample } from "../types";
import { addBox, capsuleMesh, disposeBodies, syncBodies } from "../shared";
import {
  applyWindForces,
  buildWindDynamicBodies,
  WIND_DEFAULT,
  WIND_DEFAULT_COUNT,
  WIND_DEFAULT_DRAG,
  WIND_DEFAULT_LIFT,
  WIND_MAX_COUNT,
  WIND_RADIUS,
  WIND_VERTICAL_OFFSET,
  windCamera,
  windGroundSize,
  type WindShapeType,
  type WindState,
} from "./wind-scene";

const COLOR = 0x60a5fa;

function meshForShape(shapeType: WindShapeType, x: number): THREE.Mesh {
  const r = WIND_RADIUS;
  let mesh: THREE.Mesh;
  if (shapeType === "sphere") mesh = new THREE.Mesh(new THREE.SphereGeometry(r, 16, 12), new THREE.MeshStandardMaterial({ color: COLOR, roughness: 0.75 }));
  else if (shapeType === "capsule") mesh = capsuleMesh(0.5 * r, 2 * r, COLOR, 0.75, "x");
  else {
    mesh = new THREE.Mesh(
      new THREE.BoxGeometry(2.5 * r, 1.5 * r, 0.25 * r),
      new THREE.MeshStandardMaterial({ color: COLOR, roughness: 0.75 }),
    );
  }
  mesh.position.set(x, WIND_VERTICAL_OFFSET, 0);
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  return mesh;
}

export const windSample: DemoSample = {
  id: "shapes/wind",
  name: "Shapes / Wind",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, windGroundSize(), [0, -1, 0], 0x222222, true);
    let shapeType: WindShapeType = "box";
    let count = WIND_DEFAULT_COUNT;
    let chain: BodyId[] = [];
    const visible: DemoBody[] = [];
    let state: WindState;
    let paused = false;
    const arrow = createDebugLine(scene, 0xff00ff);

    const spawn = (tuning?: { windX: number; drag: number; lift: number }) => {
      const built = buildWindDynamicBodies(world, runtime, { count, shapeType });
      chain = built.handles;
      state = built.state;
      if (tuning !== undefined) {
        state.wind[0] = tuning.windX;
        state.drag = tuning.drag;
        state.lift = tuning.lift;
      }
      for (let i = 1; i < chain.length; i++) {
        const handle = chain[i]!;
        const mesh = meshForShape(shapeType, (2 * (i - 1) + 1) * WIND_RADIUS);
        scene.add(mesh);
        const body: DemoBody = { handle, mesh, type: BodyType.Dynamic };
        bodies.push(body);
        visible.push(body);
      }
    };

    const rebuild = () => {
      const tuning = { windX: state.wind[0], drag: state.drag, lift: state.lift };
      for (const handle of chain) world.destroyBody(handle);
      disposeBodies(scene, visible);
      visible.length = 0;
      bodies.length = 1;
      spawn(tuning);
    };
    spawn();

    return {
      world,
      bodies,
      camera: windCamera,
      info: "wind on a spherical-joint chain",
      setPaused(value) { paused = value; },
      controls: [
        { key: "circle", label: "Circle", type: "button", onClick: () => { shapeType = "sphere"; rebuild(); } },
        { key: "capsule", label: "Capsule", type: "button", onClick: () => { shapeType = "capsule"; rebuild(); } },
        { key: "box", label: "Box", type: "button", onClick: () => { shapeType = "box"; rebuild(); } },
        { key: "wind", label: "Wind", type: "range", min: -50, max: 50, step: 0.1, value: WIND_DEFAULT[0], onChange: (v) => { if (typeof v === "number") state.wind[0] = v; } },
        { key: "drag", label: "Drag", type: "range", min: 0, max: 1, step: 0.01, value: WIND_DEFAULT_DRAG, onChange: (v) => { if (typeof v === "number") state.drag = v; } },
        { key: "lift", label: "Lift", type: "range", min: 0, max: 4, step: 0.01, value: WIND_DEFAULT_LIFT, onChange: (v) => { if (typeof v === "number") state.lift = v; } },
        { key: "count", label: "Count", type: "range", min: 1, max: WIND_MAX_COUNT, step: 1, value: count, onChange: (v) => { if (typeof v === "number") { count = Math.round(v); rebuild(); } } },
      ],
      step() {
        if (!paused) {
          world.step();
          const windVec = applyWindForces(runtime, state);
          updateDebugLine(arrow, [0, 0.5, 0], [0.2 * windVec[0], 0.5 + 0.2 * windVec[1], 0.2 * windVec[2]]);
        }
        syncBodies(world, bodies);
      },
      dispose() {
        disposeDebugObject(scene, arrow);
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
