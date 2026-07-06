import * as THREE from "three";
import { BodyType, type BodyHandle } from "box3d-wasm";
import type { DemoBody } from "../types";
import { capsuleMesh } from "../shared";

export function addVisibleJointBox(
  scene: THREE.Scene,
  bodies: DemoBody[],
  handle: BodyHandle,
  size: readonly [number, number, number],
  position: readonly [number, number, number],
  color: number,
  type: BodyType = BodyType.Dynamic,
): DemoBody {
  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = type !== BodyType.Static;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, type } satisfies DemoBody;
  bodies.push(body);
  return body;
}

export function addVisibleJointBodies(
  scene: THREE.Scene,
  bodies: DemoBody[],
  handles: readonly BodyHandle[],
  specs: readonly { index: number; size: readonly [number, number, number]; position: readonly [number, number, number]; color: number; type?: BodyType }[],
): void {
  for (const spec of specs) {
    const handle = handles[spec.index];
    if (handle === undefined) throw new Error(`Missing joint sample body handle at index ${spec.index}`);
    addVisibleJointBox(scene, bodies, handle, spec.size, spec.position, spec.color, spec.type ?? BodyType.Dynamic);
  }
}

export function addVisibleJointSphere(
  scene: THREE.Scene,
  bodies: DemoBody[],
  handle: BodyHandle,
  radius: number,
  position: readonly [number, number, number],
  color: number,
  type: BodyType = BodyType.Dynamic,
): DemoBody {
  const mesh = new THREE.Mesh(
    new THREE.SphereGeometry(radius, 24, 16),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = type !== BodyType.Static;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, type } satisfies DemoBody;
  bodies.push(body);
  return body;
}

export function addVisibleJointCapsule(
  scene: THREE.Scene,
  bodies: DemoBody[],
  handle: BodyHandle,
  radius: number,
  length: number,
  position: readonly [number, number, number],
  color: number,
  axis: "x" | "y" | "z" = "x",
  type: BodyType = BodyType.Dynamic,
): DemoBody {
  const mesh = capsuleMesh(radius, length, color, 0.75, axis);
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = type !== BodyType.Static;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, type } satisfies DemoBody;
  bodies.push(body);
  return body;
}
