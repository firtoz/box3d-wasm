import * as THREE from "three";
import type { DemoBody } from "./types";
import type { PhysicsWorld, Vec3 } from "box3d-wasm";

function yToX(): THREE.Quaternion {
  return new THREE.Quaternion().setFromUnitVectors(new THREE.Vector3(0, 1, 0), new THREE.Vector3(1, 0, 0));
}

export function addBox(
  world: PhysicsWorld,
  scene: THREE.Scene,
  bodies: DemoBody[],
  size: Vec3,
  position: Vec3,
  color: number,
  isStatic = false,
): DemoBody {
  const handle = world.createBox({ size, position, static: isStatic, density: isStatic ? 0 : 1000 });
  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = !isStatic;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, type: isStatic ? 0 : 2 };
  bodies.push(body);
  return body;
}

export function addSphere(
  world: PhysicsWorld,
  scene: THREE.Scene,
  bodies: DemoBody[],
  radius: number,
  position: Vec3,
  color: number,
): DemoBody {
  const handle = world.createSphere({ radius, position, density: 1000 });
  const mesh = new THREE.Mesh(
    new THREE.SphereGeometry(radius, 24, 16),
    new THREE.MeshStandardMaterial({ color, roughness: 0.6 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, type: 2 };
  bodies.push(body);
  return body;
}

export function addHull(
  world: PhysicsWorld,
  scene: THREE.Scene,
  bodies: DemoBody[],
  size: Vec3,
  position: Vec3,
  color: number,
  friction = 0.5,
  rollingResistance = 0,
  createHullShape?: (world: PhysicsWorld, size: Vec3, position: Vec3, friction: number, rollingResistance: number) => number,
): DemoBody {
  const handle = createHullShape === undefined ? world.createBox({ size, position, static: false, density: 1000 }) : createHullShape(world, size, position, friction, rollingResistance);
  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, type: 2 };
  bodies.push(body);
  return body;
}

export function capsuleMesh(radius: number, length: number, color: number, roughness = 0.75): THREE.Mesh {
  const geom = new (THREE as any).CapsuleGeometry(radius, length, 6, 12) as THREE.BufferGeometry;
  geom.applyQuaternion(yToX());
  const mat = new THREE.MeshStandardMaterial({ color, roughness });
  const mesh = new THREE.Mesh(geom, mat);
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  return mesh;
}

const AWAKE_COLOR = 0xd2b48c;
const SLEEPING_COLOR = 0x778899;
const STATIC_COLOR = 0xa9a9a9;

export function syncBodies(world: PhysicsWorld, bodies: DemoBody[]): void {
  for (const body of bodies) {
    const transform = world.getBodyTransform(body.handle);
    body.mesh.position.set(transform.position[0], transform.position[1], transform.position[2]);
    body.mesh.quaternion.set(
      transform.rotation[0],
      transform.rotation[1],
      transform.rotation[2],
      transform.rotation[3],
    );
    if (!body.preserveColor) {
      const awake = body.type !== 0 && world.bodyIsAwake(body.handle);
      const colorHex = body.type === 0 ? STATIC_COLOR : awake ? AWAKE_COLOR : SLEEPING_COLOR;
      const mat = body.mesh.material as THREE.MeshStandardMaterial;
      mat.color.setHex(colorHex);
      if (body.extraMeshes !== undefined) {
        for (const em of body.extraMeshes) {
          const emMat = em.material as THREE.MeshStandardMaterial;
          emMat.color.setHex(colorHex);
        }
      }
    }
  }
}

export function disposeBodies(scene: THREE.Scene, bodies: DemoBody[]): void {
  for (const { mesh, extraMeshes } of bodies) {
    scene.remove(mesh);
    mesh.geometry.dispose();
    const material = mesh.material;
    if (Array.isArray(material)) {
      material.forEach((entry) => entry.dispose());
    } else {
      material.dispose();
    }
    if (extraMeshes !== undefined) {
      for (const em of extraMeshes) {
        scene.remove(em);
        em.geometry.dispose();
        const emMat = em.material;
        if (Array.isArray(emMat)) {
          emMat.forEach((entry) => entry.dispose());
        } else {
          emMat.dispose();
        }
      }
    }
  }
}
