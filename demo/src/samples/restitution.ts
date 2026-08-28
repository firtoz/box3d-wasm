import * as THREE from "three";
import { BodyType, type BodyId, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";
import {
  buildRestitutionDynamicBodies,
  restitutionCamera,
  restitutionGroundSize,
  type RestitutionShape,
} from "./restitution-scene";

function addRestitutionMeshes(scene: THREE.Scene, bodies: DemoBody[], handles: readonly BodyId[], shape: RestitutionShape): void {
  let x = -(40 - 1);
  for (const handle of handles) {
    const mesh = shape === "box"
      ? new THREE.Mesh(new THREE.BoxGeometry(1, 1, 1), new THREE.MeshStandardMaterial({ color: 0x38bdf8, roughness: 0.75 }))
      : new THREE.Mesh(new THREE.SphereGeometry(0.5, 24, 16), new THREE.MeshStandardMaterial({ color: 0x38bdf8, roughness: 0.75 }));
    mesh.position.set(x, 40, 0);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    scene.add(mesh);
    bodies.push({ handle, mesh, type: BodyType.Dynamic });
    x += 2;
  }
}

export const restitutionSample: DemoSample = {
  id: "restitution",
  name: "Shapes / Restitution",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, restitutionGroundSize(), [0, -1, 0], 0x222222, true);
    let shape: RestitutionShape = "sphere";
    const spawn = () => {
      const dynamic = bodies.splice(1);
      for (const body of dynamic) {
        scene.remove(body.mesh);
        body.mesh.geometry.dispose();
        (body.mesh.material as THREE.Material).dispose();
        world.destroyBody(body.handle);
      }
      addRestitutionMeshes(scene, bodies, buildRestitutionDynamicBodies(world, runtime, shape), shape);
    };
    spawn();
    return {
      world,
      bodies,
      profile: true,
      camera: restitutionCamera,
      info: "restitution ramp — Sphere / Box",
      controls: [
        { key: "sphere", label: "Sphere", type: "button", onClick: () => { shape = "sphere"; spawn(); } },
        { key: "box", label: "Box", type: "button", onClick: () => { shape = "box"; spawn(); } },
      ],
      step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
