import * as THREE from "three";
import { BodyType, type BodyHandle, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, addSphere, disposeBodies, syncBodies } from "../shared";
import type { RenderBody } from "../generic-host";
import {
  buildBulletVsStackDynamicBodies,
  bulletVsStackCamera,
  bulletVsStackGroundSize,
  createBulletVsStackBodies,
  launchBullet,
} from "./bullet-vs-stack-scene";

function isBoxBody(spec: RenderBody): spec is RenderBody & { kind: "box"; size: [number, number, number]; color: number } {
  return spec.kind === "box";
}

export const bulletVsStackSample: DemoSample = {
  id: "continuous/bullet-vs-stack",
  name: "Continuous / Bullet vs Stack",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, bulletVsStackGroundSize(), [0, -1, 0], 0x222222, true);
    const handles = buildBulletVsStackDynamicBodies(world, runtime);
    const specs = createBulletVsStackBodies();
    for (let i = 0; i < handles.length; i++) {
      const spec = specs[i];
      const handle = handles[i]! as BodyHandle;
      if (spec === undefined || !isBoxBody(spec)) continue;
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(spec.size[0], spec.size[1], spec.size[2]),
        new THREE.MeshStandardMaterial({ color: spec.color, roughness: 0.75 }),
      );
      mesh.position.set(spec.position[0], spec.position[1], spec.position[2]);
      mesh.castShadow = spec.type !== BodyType.Static;
      mesh.receiveShadow = true;
      scene.add(mesh);
      bodies.push({ handle, mesh, shapeIds: world.getBodyShapes(handle), type: spec.type ?? BodyType.Dynamic });
    }
    let bulletHandle = 0 as BodyHandle;
    return {
      world,
      bodies,
      profile: true,
      camera: bulletVsStackCamera,
      controls: [{
        key: "launch",
        label: "Launch",
        type: "button",
        onClick: () => {
          if (bulletHandle !== 0) {
            const idx = bodies.findIndex((b) => b.handle === bulletHandle);
            if (idx >= 0) {
              const old = bodies[idx]!;
              scene.remove(old.mesh);
              old.mesh.geometry.dispose();
              (old.mesh.material as THREE.Material).dispose();
              bodies.splice(idx, 1);
            }
            world.destroyBody(bulletHandle);
          }
          const launchHandles: number[] = [];
          bulletHandle = launchBullet(world, runtime, launchHandles) as BodyHandle;
          addSphere(world, scene, bodies, 0.25, [20.5, 5.5, 0], 0xef4444);
          bodies[bodies.length - 1]!.handle = bulletHandle;
        },
      }],
      step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
