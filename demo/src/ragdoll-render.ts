import * as THREE from "three";
import type { Vec3 } from "box3d-wasm";

export type RagdollRenderBone = {
  a: Vec3;
  b: Vec3;
  radius: number;
  color: number;
  position: [number, number, number];
  rotation: [number, number, number, number];
};

export const RAGDOLL_RENDER_BONES: RagdollRenderBone[] = [
  { a: [0.07, 0, -0.08], b: [-0.07, 0, -0.08], radius: 0.13, color: 0x1e90ff, position: [0, 0.932087, -0.051708], rotation: [0.739169, 0, 0, 0.67352] },
  { a: [0.06, 0, -0.052264], b: [-0.06, 0, -0.052264], radius: 0.12, color: 0x48d1cc, position: [0, 1.113505, -0.03481], rotation: [0.739973, 0, 0, 0.672637] },
  { a: [0.08, -0.015133, -0.091801], b: [-0.08, -0.015133, -0.091801], radius: 0.1, color: 0x48d1cc, position: [0, 1.194336, -0.027087], rotation: [0.703611, 0, 0, 0.710586] },
  { a: [0.11, -0.039753, -0.13], b: [-0.11, -0.039753, -0.13], radius: 0.145, color: 0x48d1cc, position: [0, 1.31043, -0.028232], rotation: [0.669856, 0.000001, -0.000001, 0.742491] },
  { a: [-0.000001, 0, -0.02], b: [0, -0.005, -0.08], radius: 0.07, color: 0xffdead, position: [0, 1.575582, -0.055837], rotation: [0.879922, 0, 0, 0.475118] },
  { a: [-0.000001, 0.016892, -0.05869], b: [0, -0.003629, -0.115072], radius: 0.0975, color: 0xffdead, position: [0, 1.653348, -0.003241], rotation: [0.750288, 0, 0, 0.661111] },
  { a: [0.023719, 0.006008, -0.039068], b: [-0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff, position: [0.090416, 0.986104, -0.03509], rotation: [-0.703287, -0.070715, 0.053866, 0.705327] },
  { a: [0.001778, 0, 0.009841], b: [-0.078577, 0.014707, -0.41816], radius: 0.075, color: 0x1e90ff, position: [0.101198, 0.527027, -0.037374], rotation: [-0.653328, -0.06686, 0.058582, 0.751838] },
  { a: [-0.023719, 0.006008, -0.039068], b: [0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff, position: [-0.090416, 0.986104, -0.03509], rotation: [-0.703287, 0.070715, -0.053865, 0.705326] },
  { a: [-0.00182, 0, 0.010071], b: [0.077883, 0.014825, -0.418047], radius: 0.075, color: 0x1e90ff, position: [-0.101198, 0.527027, -0.037373], rotation: [-0.653327, 0.06686, -0.058582, 0.751839] },
  { a: [0, 0, 0], b: [-0.091118, 0.037775, 0.229719], radius: 0.075, color: 0x48d1cc, position: [0.20378, 1.484275, -0.115897], rotation: [0.143082, 0.69598, -0.69013, 0.13733] },
  { a: [0, 0, 0], b: [-0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead, position: [0.305614, 1.242908, -0.117599], rotation: [0.165048, 0.563437, -0.802002, 0.109959] },
  { a: [0, 0, 0], b: [0.091118, 0.037775, 0.229718], radius: 0.075, color: 0x48d1cc, position: [-0.20378, 1.484276, -0.115899], rotation: [0.143083, -0.695978, 0.690132, 0.137329] },
  { a: [0, 0, 0], b: [0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead, position: [-0.305614, 1.242907, -0.117599], rotation: [0.165048, -0.563437, 0.802002, 0.109959] },
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
