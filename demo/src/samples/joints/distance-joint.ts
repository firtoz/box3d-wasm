import * as THREE from "three";
import { type BodyId, type Box3DRuntime } from "box3d-wasm";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointSphere } from "./shared";
import {
  DISTANCE_JOINT_MAX_COUNT,
  DISTANCE_JOINT_RADIUS,
  DISTANCE_JOINT_Y_OFFSET,
  applyDistanceJointTuning,
  createDistanceJointChain,
  defaultDistanceJointTuning,
  distanceJointCamera,
  distanceJointGroundSize,
} from "./distance-joint-scene";

export const distanceJointSample: DemoSample = {
  id: "joints/distance-joint",
  name: "Joints / Distance Joint",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, distanceJointGroundSize(), [0, -1, 0], 0x222222, true);
    const tuning = defaultDistanceJointTuning();
    let chain = createDistanceJointChain(world, runtime, tuning);
    const chainBodies: DemoBody[] = [];
    const lines: THREE.Line[] = [];
    const pointsA: THREE.Points[] = [];
    const pointsB: THREE.Points[] = [];
    const pA = [0, 0, 0] as [number, number, number];
    const pB = [0, 0, 0] as [number, number, number];

    const clearOverlay = () => {
      for (const line of lines) disposeDebugObject(scene, line);
      for (const point of pointsA) disposeDebugObject(scene, point);
      for (const point of pointsB) disposeDebugObject(scene, point);
      lines.length = 0;
      pointsA.length = 0;
      pointsB.length = 0;
    };

    const addSphere = (handle: BodyId, x: number) => {
      chainBodies.push(addVisibleJointSphere(scene, bodies, handle, DISTANCE_JOINT_RADIUS, [x, DISTANCE_JOINT_Y_OFFSET, 0], 0x38bdf8));
      lines.push(createDebugLine(scene, 0xffffff));
      pointsA.push(createDebugPoint(scene, 0xffffff, 4));
      pointsB.push(createDebugPoint(scene, 0xffffff, 4));
    };

    for (let i = 0; i < chain.bodies.length; i++) {
      addSphere(chain.bodies[i]!, (i + 1) * tuning.length);
    }

    const rebuild = (count: number) => {
      for (const joint of chain.joints) world.destroyJoint(joint);
      for (const handle of chain.bodies) world.destroyBody(handle);
      for (const body of chainBodies) {
        const index = bodies.indexOf(body);
        if (index >= 0) bodies.splice(index, 1);
        scene.remove(body.mesh);
        body.mesh.geometry.dispose();
        (body.mesh.material as THREE.Material).dispose();
      }
      chainBodies.length = 0;
      clearOverlay();
      tuning.count = count;
      chain = createDistanceJointChain(world, runtime, tuning, chain.anchor);
      for (let i = 0; i < chain.bodies.length; i++) {
        addSphere(chain.bodies[i]!, (i + 1) * tuning.length);
      }
    };

    const apply = () => applyDistanceJointTuning(world, chain.joints, tuning);

    return {
      world,
      bodies,
      profile: true,
      camera: distanceJointCamera,
      info: "distance joint — length / spring / limit / count",
      controls: [
        { key: "length", label: "Length", type: "range", min: 0.1, max: 4, step: 0.1, value: tuning.length, onChange: (v) => { if (typeof v === "number") { tuning.length = v; apply(); } } },
        { key: "spring", label: "Spring", type: "toggle", value: tuning.enableSpring, onChange: (v) => { if (typeof v === "boolean") { tuning.enableSpring = v; apply(); } } },
        { key: "tension", label: "Tension", type: "range", min: 0, max: 4000, step: 10, value: tuning.tensionForce, onChange: (v) => { if (typeof v === "number") { tuning.tensionForce = v; apply(); } } },
        { key: "compression", label: "Compression", type: "range", min: 0, max: 200, step: 1, value: tuning.compressionForce, onChange: (v) => { if (typeof v === "number") { tuning.compressionForce = v; apply(); } } },
        { key: "hertz", label: "Hertz", type: "range", min: 0, max: 15, step: 0.1, value: tuning.hertz, onChange: (v) => { if (typeof v === "number") { tuning.hertz = v; apply(); } } },
        { key: "damping", label: "Damping", type: "range", min: 0, max: 4, step: 0.1, value: tuning.dampingRatio, onChange: (v) => { if (typeof v === "number") { tuning.dampingRatio = v; apply(); } } },
        { key: "limit", label: "Limit", type: "toggle", value: tuning.enableLimit, onChange: (v) => { if (typeof v === "boolean") { tuning.enableLimit = v; apply(); } } },
        { key: "min", label: "Min", type: "range", min: 0.1, max: 4, step: 0.1, value: tuning.minLength, onChange: (v) => { if (typeof v === "number") { tuning.minLength = v; apply(); } } },
        { key: "max", label: "Max", type: "range", min: 0.1, max: 4, step: 0.1, value: tuning.maxLength, onChange: (v) => { if (typeof v === "number") { tuning.maxLength = v; apply(); } } },
        { key: "count", label: "Count", type: "range", min: 1, max: DISTANCE_JOINT_MAX_COUNT, step: 1, value: tuning.count, onChange: (v) => { if (typeof v === "number") rebuild(Math.round(v)); } },
      ],
      step(dt, subSteps) {
        world.step(dt ?? 1 / 60, subSteps ?? 4);
        syncBodies(world, bodies);
        for (let i = 0; i < chainBodies.length; i++) {
          const mesh = chainBodies[i]?.mesh;
          if (mesh === undefined) continue;
          const prev = i === 0 ? undefined : chainBodies[i - 1]?.mesh;
          if (i === 0) {
            pA[0] = 0;
            pA[1] = DISTANCE_JOINT_Y_OFFSET;
            pA[2] = 0;
          } else if (prev !== undefined) {
            pA[0] = prev.position.x;
            pA[1] = prev.position.y;
            pA[2] = prev.position.z;
          }
          pB[0] = mesh.position.x;
          pB[1] = mesh.position.y;
          pB[2] = mesh.position.z;
          const line = lines[i];
          const pointA = pointsA[i];
          const pointB = pointsB[i];
          if (line !== undefined) updateDebugLine(line, pA, pB);
          if (pointA !== undefined) updateDebugPoint(pointA, pA);
          if (pointB !== undefined) updateDebugPoint(pointB, pB);
        }
      },
      dispose() {
        clearOverlay();
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
