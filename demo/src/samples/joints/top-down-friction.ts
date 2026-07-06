import * as THREE from "three";
import { BodyType, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBox, addVisibleJointCapsule, addVisibleJointSphere } from "./shared";
import { createTopDownFriction, createTopDownFrictionVisible, topDownFrictionCamera, topDownFrictionGroundParts } from "./top-down-friction-scene";

export const topDownFrictionSample: DemoSample = {
  id: "joints/top-down-friction",
  name: "Joints / Top Down Friction",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const { world, handles } = createTopDownFriction(runtime);
    const bodies: DemoBody[] = [];
    const staticMeshes: THREE.Mesh[] = [];
    for (const part of topDownFrictionGroundParts) {
      const mesh = new THREE.Mesh(new THREE.BoxGeometry(part.size[0], part.size[1], part.size[2]), new THREE.MeshStandardMaterial({ color: 0x222222, roughness: 0.75 }));
      mesh.position.set(part.position[0], part.position[1], part.position[2]);
      mesh.receiveShadow = true;
      scene.add(mesh);
      staticMeshes.push(mesh);
    }
    const visible = createTopDownFrictionVisible();
    for (let i = 0; i < visible.length; i++) {
      const handle = handles[i + 1];
      const spec = visible[i];
      if (handle === undefined) throw new Error(`Missing top-down-friction body at visible index ${i}`);
      if (spec.kind === "capsule") addVisibleJointCapsule(scene, bodies, handle, spec.radius, spec.length, spec.position, 0x38bdf8);
      else if (spec.kind === "sphere") addVisibleJointSphere(scene, bodies, handle, spec.radius, spec.position, 0xf97316);
      else addVisibleJointBox(scene, bodies, handle, [spec.size[0] * 0.5, spec.size[1] * 0.5, spec.size[2] * 0.5], spec.position, 0x22c55e, BodyType.Dynamic);
    }
    return {
      world,
      bodies,
      profile: true,
      camera: topDownFrictionCamera,
      controls: [{ key: "explode", label: "Explode", type: "button", onClick: () => world.explode([0, 10, 0], 10, 5, 10000) }],
      step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { for (const mesh of staticMeshes) { scene.remove(mesh); mesh.geometry.dispose(); (mesh.material as THREE.Material).dispose(); } disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
