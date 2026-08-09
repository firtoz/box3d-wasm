import {B3_PI, type Quat, type Vec3} from "box3d-wasm";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "./f32";

const RAND_LIMIT = 32767;
const RAND_SEED = 12345;

export class Box3DRng {
  private seed: number;

  constructor(seed: number = RAND_SEED) {
    this.seed = seed >>> 0;
  }

  randomInt(): number {
    let x = this.seed >>> 0;
    x ^= x << 13;
    x ^= x >>> 17;
    x ^= x << 5;
    // Match upstream uint32 XorShift32: store and return unsigned modulo.
    this.seed = x >>> 0;
    return this.seed % (RAND_LIMIT + 1);
  }

  /** Match upstream `RandomIntRange(lo, hi)` (returns float-valued ints). */
  randomIntRange(lo: number, hi: number): number {
    return lo + (this.randomInt() % (hi - lo + 1));
  }

  /** Match upstream `RandomFloatRange` float32 arithmetic exactly. */
  randomFloatRange(lo: number, hi: number): number {
    // float r = (float)(RandomInt() & RAND_LIMIT); r /= RAND_LIMIT; r = (hi - lo) * r + lo;
    let r = f32(this.randomInt() & RAND_LIMIT);
    r = f32Div(r, RAND_LIMIT);
    return f32Add(f32Mul(f32Sub(hi, lo), r), lo);
  }

  randomVec3(lo: Vec3, hi: Vec3): Vec3 {
    return [
      this.randomFloatRange(lo[0], hi[0]),
      this.randomFloatRange(lo[1], hi[1]),
      this.randomFloatRange(lo[2], hi[2]),
    ];
  }

  /** Match upstream `RandomVec3Uniform(lo, hi)`. */
  randomVec3Uniform(lo: number, hi: number): Vec3 {
    return [
      this.randomFloatRange(lo, hi),
      this.randomFloatRange(lo, hi),
      this.randomFloatRange(lo, hi),
    ];
  }

  randomQuat(): Quat {
    const u1 = this.randomFloatRange(0, 1);
    const u2 = this.randomFloatRange(0, 2 * B3_PI);
    const u3 = this.randomFloatRange(0, 2 * B3_PI);

    const sqrt1MinusU1 = Math.sqrt(1 - u1);
    const sqrtU1 = Math.sqrt(u1);

    return [
      sqrt1MinusU1 * Math.sin(u2),
      sqrt1MinusU1 * Math.cos(u2),
      sqrtU1 * Math.sin(u3),
      sqrtU1 * Math.cos(u3),
    ];
  }
}
