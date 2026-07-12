import type { Vec3 } from "box3d-wasm";

export const f32 = Math.fround;

/** Float32 multiply matching C `a * b` for float operands (exact product fits in double). */
export function f32Mul(a: number, b: number): number {
  return f32(f32(a) * f32(b));
}

/** Float32 add matching C `a + b` for float operands. */
export function f32Add(a: number, b: number): number {
  return f32(f32(a) + f32(b));
}

/** Float32 subtract matching C `a - b` for float operands. */
export function f32Sub(a: number, b: number): number {
  return f32(f32(a) - f32(b));
}

/** Float32 divide matching C `a / b` for float operands. */
export function f32Div(a: number, b: number): number {
  return f32(f32(a) / f32(b));
}

export function f32Lerp(a: number, b: number, alpha: number): number {
  return f32(f32(1 - alpha) * a + f32(alpha * b));
}

export function f32LerpVec3(a: Vec3, b: Vec3, alpha: number): Vec3 {
  return [f32Lerp(a[0], b[0], alpha), f32Lerp(a[1], b[1], alpha), f32Lerp(a[2], b[2], alpha)];
}
