import * as THREE from "three";
import { BodyType, type Box3DRuntime, type Vec3 } from "box3d-wasm";
import { ObjectRuntime, type ObjectWorld } from "box3d-wasm/objects";
import type { DemoBody, DemoSample } from "./types";
import { disposeBodies, syncBodies } from "./shared";

function addObjectBox(
  world: ObjectWorld,
  scene: THREE.Scene,
  bodies: DemoBody[],
  size: Vec3,
  position: Vec3,
  color: number,
  isStatic = false,
): DemoBody {
  const { body, shape } = world.createBoxWithShape({
    size,
    position,
    static: isStatic,
    density: isStatic ? 0 : 1000,
  });

  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = !isStatic;
  mesh.receiveShadow = true;
  scene.add(mesh);

  const renderBody = {
    handle: body.handle,
    mesh,
    shapeIds: [shape.handle.shapeHandle],
    type: isStatic ? BodyType.Static : BodyType.Dynamic,
  } satisfies DemoBody;
  bodies.push(renderBody);
  return renderBody;
}

export const singleBoxSample: DemoSample = {
  id: "single-box",
  name: "Stacking / Single Box",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const objectRuntime = ObjectRuntime.fromRuntime(runtime);
    const world = objectRuntime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addObjectBox(world, scene, bodies, [20, 1, 20], [0, -1, 0], 0x222222, true);

    const cube = addObjectBox(world, scene, bodies, [0.5, 0.5, 0.5], [0, 0.5, 0], 0xf59e0b);
    world.body(cube.handle).setAngularVelocity([0, 10, 0]);
    return {
      world: world.raw,
      bodies,
      camera: { position: [0, 4.226, 9.063], target: [0, 0, 0] },
      controls: [],
      step(dt, subSteps) {
        world.step(dt, subSteps ?? 4);
        syncBodies(world.raw, bodies);
      },
      dispose() {
        disposeBodies(scene, bodies);
        world.dispose();
      },
    };
  },
};
