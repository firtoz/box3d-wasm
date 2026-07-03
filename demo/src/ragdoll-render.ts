import * as THREE from "three";
import type { Vec3 } from "box3d-wasm";

export type RagdollRenderBone = {
  a: Vec3;
  b: Vec3;
  radius: number;
  color: number;
};

export const RAGDOLL_RENDER_BONES: RagdollRenderBone[] = [
  { a: [0.07, 0, -0.08], b: [-0.07, 0, -0.08], radius: 0.13, color: 0x1e90ff },
  { a: [0.06, 0, -0.052264], b: [-0.06, 0, -0.052264], radius: 0.12, color: 0x7dd3fc },
  { a: [0.08, -0.015133, -0.091801], b: [-0.08, -0.015133, -0.091801], radius: 0.1, color: 0x7dd3fc },
  { a: [0.11, -0.039753, -0.13], b: [-0.11, -0.039753, -0.13], radius: 0.145, color: 0x7dd3fc },
  { a: [-0.000001, 0, -0.02], b: [0, -0.005, -0.08], radius: 0.07, color: 0xffdead },
  { a: [-0.000001, 0.016892, -0.05869], b: [0, -0.003629, -0.115072], radius: 0.0975, color: 0xffdead },
  { a: [0.023719, 0.006008, -0.039068], b: [-0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff },
  { a: [0.001778, 0, 0.009841], b: [-0.078577, 0.014707, -0.41816], radius: 0.075, color: 0x1e90ff },
  { a: [-0.023719, 0.006008, -0.039068], b: [0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff },
  { a: [-0.00182, 0, 0.010071], b: [0.077883, 0.014825, -0.418047], radius: 0.075, color: 0x1e90ff },
  { a: [0, 0, 0], b: [-0.091118, 0.037775, 0.229719], radius: 0.075, color: 0x7dd3fc },
  { a: [0, 0, 0], b: [-0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead },
  { a: [0, 0, 0], b: [0.091118, 0.037775, 0.229718], radius: 0.075, color: 0x7dd3fc },
  { a: [0, 0, 0], b: [0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead },
];

export function ragdollCapsuleMesh(a: Vec3, b: Vec3, radius: number, color: number): THREE.Mesh {
  const va = new THREE.Vector3(...a);
  const vb = new THREE.Vector3(...b);
  const delta = vb.clone().sub(va);
  const length = delta.length();
  const geom = new (THREE as any).CapsuleGeometry(radius, length, 6, 12) as THREE.BufferGeometry;
  geom.applyQuaternion(new THREE.Quaternion().setFromUnitVectors(new THREE.Vector3(0, 1, 0), delta.normalize()));
  geom.translate((a[0] + b[0]) * 0.5, (a[1] + b[1]) * 0.5, (a[2] + b[2]) * 0.5);
  const mesh = new THREE.Mesh(geom, new THREE.MeshStandardMaterial({ color, roughness: 0.75 }));
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  return mesh;
}
