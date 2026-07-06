import * as THREE from "three";
import { B3_AXIS_Y, B3_PI, BodyType, type Box3DRuntime, type PhysicsWorld } from "box3d-wasm";
import type { DemoBody, DemoSample, ControlSpec } from "./types";
import { addBox, capsuleMesh, disposeBodies, syncBodies } from "./shared";

const STACK_SIZE = 24;

function buildStack(runtime: Box3DRuntime, world: PhysicsWorld, scene: THREE.Scene, bodies: DemoBody[], useCapsule: boolean): void {
  addBox(world, scene, bodies, [30, 1, 30], [0, -1, 0], 0x222222, true);

  const color = 0xf59e0b;
  const hullSize: [number, number, number] = [2.5, 0.25, 0.25];
  const capsuleSegLen = 5;
  const capsuleR = 0.25;

  for (let i = 0; i < STACK_SIZE; i++) {
    const even = (i & 1) === 0;
    const alpha = even ? 0.5 * B3_PI : 0;
    const x = even ? 1.75 : 0;
    const z = even ? 0 : 1.75;

    for (let side = 0; side < 2; side++) {
      const sx = side === 0 ? x : -x;
      const sz = side === 0 ? z : -z;
      const p: [number, number, number] = [sx, 0.5 * i + 0.25, sz];
      const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, alpha);
      const bodyHandle = world.createBody({ type: BodyType.Dynamic, position: p, isAwake: true });
      world.setBodyTransform(bodyHandle, p, rotation);

      if (useCapsule) {
        runtime.createCapsuleShape(bodyHandle, [-2.5, 0, 0], [2.5, 0, 0], capsuleR, { density: 1000 });
      } else {
        runtime.createHullShape(bodyHandle, hullSize);
      }

      const mesh = useCapsule
        ? capsuleMesh(capsuleR, capsuleSegLen, color)
        : new THREE.Mesh(
            new THREE.BoxGeometry(5, 0.5, 0.5),
            new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
          );
      mesh.position.set(p[0], p[1], p[2]);
      mesh.quaternion.set(...rotation);
      mesh.castShadow = true;
      mesh.receiveShadow = true;
      scene.add(mesh);

      bodies.push({ handle: bodyHandle, mesh, type: BodyType.Dynamic });
    }
  }
}

export const jengaStackSample: DemoSample = {
  id: "jenga-stack",
  name: "Stacking / Jenga Stack",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];
    let useCapsule = false;
    buildStack(runtime, world, scene, bodies, useCapsule);

    const controls: ControlSpec[] = [
      {
        key: "shape-type",
        label: "Capsule",
        min: 0,
        max: 1,
        step: 1,
        value: 0,
        onChange(value) {
          const newUseCapsule = (value as number) >= 0.5;
          if (newUseCapsule !== useCapsule) {
            useCapsule = newUseCapsule;
            disposeBodies(scene, bodies);
            bodies.length = 0;
            buildStack(runtime, world, scene, bodies, useCapsule);
          }
          controls[0].value = newUseCapsule ? 1 : 0;
        },
      },
    ];

    return {
      world,
      bodies,
      controls,
      step(dt, subSteps) {
        world.step(dt, subSteps ?? 4);
        syncBodies(world, bodies);
      },
      dispose() {
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
