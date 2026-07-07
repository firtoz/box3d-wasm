import type { Vec3 } from "box3d-wasm";

export const f32 = Math.fround;

export function f32Lerp(a: number, b: number, alpha: number): number {
  return f32(f32(1 - alpha) * a + f32(alpha * b));
}

export function f32LerpVec3(a: Vec3, b: Vec3, alpha: number): Vec3 {
  return [f32Lerp(a[0], b[0], alpha), f32Lerp(a[1], b[1], alpha), f32Lerp(a[2], b[2], alpha)];
}
