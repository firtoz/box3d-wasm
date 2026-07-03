import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";

function rotZ(v: Vec3, angle: number): Vec3 {
  const c = Math.cos(angle), s = Math.sin(angle);
  return [v[0] * c - v[1] * s, v[0] * s + v[1] * c, v[2]];
}

function invRotZ(v: Vec3, angle: number): Vec3 {
  const c = Math.cos(angle), s = Math.sin(angle);
  return [v[0] * c + v[1] * s, -v[0] * s + v[1] * c, v[2]];
}

const WASHER_COUNT = 8000;

class WasherWorker extends PhysicsWorkerBase {
  protected groundSize: Vec3 = [60, 1, 60];

  protected async buildScene(): Promise<number[]> {
    const motorSpeed = 25;
    const angleRad = motorSpeed * Math.PI / 180;

    const drumBody = this.world!.createBody({
      type: 1,
      position: [0, 21, 0],
      angularVelocity: [0, 0, angleRad],
      linearVelocity: [0.001, -0.002, 0],
      isAwake: true,
    });

    const r0 = 14, r1 = 16, r2 = 18;
    const nd: Vec3 = [0, 0, -10];
    const pd: Vec3 = [0, 0, 10];
    const angle = Math.PI / 18;
    const qoAngle = 0.1 * angle;

    let u1: Vec3 = [1, 0, 0];
    for (let i = 0; i < 36; i++) {
      const u2 = i === 35 ? [1, 0, 0] as Vec3 : rotZ(u1, angle);

      const a1 = invRotZ(u1, qoAngle);
      const a2 = rotZ(u2, qoAngle);

      const p1 = [nd[0] + r1 * a1[0], nd[1] + r1 * a1[1], nd[2] + r1 * a1[2]] as const;
      const p2 = [nd[0] + r2 * a1[0], nd[1] + r2 * a1[1], nd[2] + r2 * a1[2]] as const;
      const p3 = [nd[0] + r1 * a2[0], nd[1] + r1 * a2[1], nd[2] + r1 * a2[2]] as const;
      const p4 = [nd[0] + r2 * a2[0], nd[1] + r2 * a2[1], nd[2] + r2 * a2[2]] as const;
      const p5 = [pd[0] + r1 * a1[0], pd[1] + r1 * a1[1], pd[2] + r1 * a1[2]] as const;
      const p6 = [pd[0] + r2 * a1[0], pd[1] + r2 * a1[1], pd[2] + r2 * a1[2]] as const;
      const p7 = [pd[0] + r1 * a2[0], pd[1] + r1 * a2[1], pd[2] + r1 * a2[2]] as const;
      const p8 = [pd[0] + r2 * a2[0], pd[1] + r2 * a2[1], pd[2] + r2 * a2[2]] as const;

      const hullHandle = this.runtime!.createHullFromPoints([...p1, ...p2, ...p3, ...p4, ...p5, ...p6, ...p7, ...p8]);
      this.runtime!.createShapeFromHull(drumBody, hullHandle);
      this.runtime!.destroyHull(hullHandle);

      if (i % 9 === 0) {
        const q1 = [nd[0] + r0 * u1[0], nd[1] + r0 * u1[1], nd[2] + r0 * u1[2]] as const;
        const q2 = [nd[0] + r1 * u1[0], nd[1] + r1 * u1[1], nd[2] + r1 * u1[2]] as const;
        const q3 = [nd[0] + r0 * u2[0], nd[1] + r0 * u2[1], nd[2] + r0 * u2[2]] as const;
        const q4 = [nd[0] + r1 * u2[0], nd[1] + r1 * u2[1], nd[2] + r1 * u2[2]] as const;
        const q5 = [pd[0] + r0 * u1[0], pd[1] + r0 * u1[1], pd[2] + r0 * u1[2]] as const;
        const q6 = [pd[0] + r1 * u1[0], pd[1] + r1 * u1[1], pd[2] + r1 * u1[2]] as const;
        const q7 = [pd[0] + r0 * u2[0], pd[1] + r0 * u2[1], pd[2] + r0 * u2[2]] as const;
        const q8 = [pd[0] + r1 * u2[0], pd[1] + r1 * u2[1], pd[2] + r1 * u2[2]] as const;

        const postHandle = this.runtime!.createHullFromPoints([...q1, ...q2, ...q3, ...q4, ...q5, ...q6, ...q7, ...q8]);
        this.runtime!.createShapeFromHull(drumBody, postHandle);
        this.runtime!.destroyHull(postHandle);
      }

      u1 = u2;
    }

    const a = 0.2;
    const gridCount = 20;
    const cubeHandle = this.runtime!.createHullFromPoints([
      -a, -a, -a,  a, -a, -a,  a, a, -a,  -a, a, -a,
      -a, -a,  a,  a, -a,  a,  a, a,  a,  -a, a,  a,
    ]);
    const cubeDef = { density: 1000, friction: 0.5, restitution: 0 };
    const handles = new Array<number>(WASHER_COUNT);
    let idx = 0;
    for (let i = 0; i < gridCount; i++) {
      const x = -2 * a * gridCount + i * 4 * a;
      for (let j = 0; j < gridCount; j++) {
        const y = -2 * a * gridCount + 21 + j * 4 * a;
        for (let k = 0; k < gridCount; k++) {
          const z = -2 * a * gridCount + k * 4 * a;
          const bodyHandle = this.world!.createBody({ type: 2, position: [x, y, z], isAwake: true });
          this.runtime!.createShapeFromHull(bodyHandle, cubeHandle, cubeDef);
          handles[idx] = bodyHandle;
          idx++;
        }
      }
    }
    this.runtime!.destroyHull(cubeHandle);

    return handles;
  }
}

new WasherWorker();
