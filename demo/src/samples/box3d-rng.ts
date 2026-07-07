import { B3_PI, type Quat, type Vec3 } from "box3d-wasm";

const RAND_LIMIT = 32767;
const RAND_SEED = 12345;

export class Box3DRng {
  private seed = RAND_SEED >>> 0;

  randomInt(): number {
    let x = this.seed >>> 0;
    x ^= x << 13;
    x ^= x >>> 17;
    x ^= x << 5;
    this.seed = x >>> 0;
    return x % (RAND_LIMIT + 1);
  }

  randomFloatRange(lo: number, hi: number): number {
    let r = this.randomInt() & RAND_LIMIT;
    r /= RAND_LIMIT;
    return Math.fround((hi - lo) * r + lo);
  }

  randomVec3(lo: Vec3, hi: Vec3): Vec3 {
    return [
      this.randomFloatRange(lo[0], hi[0]),
      this.randomFloatRange(lo[1], hi[1]),
      this.randomFloatRange(lo[2], hi[2]),
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
