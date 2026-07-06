import { PhysicsWorkerBase } from "../physics-worker-base";
import { B3_PI, BodyType, type Vec3 } from "box3d-wasm";

const f = Math.fround;

function rotZ(v: Vec3, angle: number): Vec3 {
  const c = f(Math.cos(angle)), s = f(Math.sin(angle));
  return [f(f(v[0] * c) - f(v[1] * s)), f(f(v[0] * s) + f(v[1] * c)), v[2]];
}

function invRotZ(v: Vec3, angle: number): Vec3 {
  const c = f(Math.cos(angle)), s = f(Math.sin(angle));
  return [f(f(v[0] * c) + f(v[1] * s)), f(f(-v[0] * s) + f(v[1] * c)), v[2]];
}

const WASHER_COUNT = 8000;

class WasherWorker extends PhysicsWorkerBase {
  protected groundSize: Vec3 = [60, 1, 60];

  protected async buildScene(): Promise<number[]> {
    const motorSpeed = f(25);
    const angleRad = f(f(B3_PI / 180) * motorSpeed);

    const drumBody = this.world!.createBody({
      type: BodyType.Kinematic,
      position: [0, 21, 0],
      rotation: [0, 0, 0, 1],
      angularVelocity: [0, 0, angleRad],
      linearVelocity: [0.001, -0.002, 0],
      isAwake: true,
    });

    const r0 = f(14), r1 = f(16), r2 = f(18);
    const nd: Vec3 = [f(0), f(0), f(-10)];
    const pd: Vec3 = [f(0), f(0), f(10)];
    const angle = f(B3_PI / 18);
    const qoAngle = f(f(0.1) * angle);

    let u1: Vec3 = [1, 0, 0];
    for (let i = 0; i < 36; i++) {
      const u2: Vec3 = i === 35 ? [1, 0, 0] : rotZ(u1, angle);

      const a1 = invRotZ(u1, qoAngle);
      const a2 = rotZ(u2, qoAngle);

      const p1 = [f(nd[0] + f(r1 * a1[0])), f(nd[1] + f(r1 * a1[1])), f(nd[2] + f(r1 * a1[2]))] as const;
      const p2 = [f(nd[0] + f(r2 * a1[0])), f(nd[1] + f(r2 * a1[1])), f(nd[2] + f(r2 * a1[2]))] as const;
      const p3 = [f(nd[0] + f(r1 * a2[0])), f(nd[1] + f(r1 * a2[1])), f(nd[2] + f(r1 * a2[2]))] as const;
      const p4 = [f(nd[0] + f(r2 * a2[0])), f(nd[1] + f(r2 * a2[1])), f(nd[2] + f(r2 * a2[2]))] as const;
      const p5 = [f(pd[0] + f(r1 * a1[0])), f(pd[1] + f(r1 * a1[1])), f(pd[2] + f(r1 * a1[2]))] as const;
      const p6 = [f(pd[0] + f(r2 * a1[0])), f(pd[1] + f(r2 * a1[1])), f(pd[2] + f(r2 * a1[2]))] as const;
      const p7 = [f(pd[0] + f(r1 * a2[0])), f(pd[1] + f(r1 * a2[1])), f(pd[2] + f(r1 * a2[2]))] as const;
      const p8 = [f(pd[0] + f(r2 * a2[0])), f(pd[1] + f(r2 * a2[1])), f(pd[2] + f(r2 * a2[2]))] as const;

      const hullHandle = this.runtime!.createHullFromPoints([...p1, ...p2, ...p3, ...p4, ...p5, ...p6, ...p7, ...p8]);
      this.runtime!.createShapeFromHull(drumBody, hullHandle);
      this.runtime!.destroyHull(hullHandle);

      if (i % 9 === 0) {
        const q1 = [f(nd[0] + f(r0 * u1[0])), f(nd[1] + f(r0 * u1[1])), f(nd[2] + f(r0 * u1[2]))] as const;
        const q2 = [f(nd[0] + f(r1 * u1[0])), f(nd[1] + f(r1 * u1[1])), f(nd[2] + f(r1 * u1[2]))] as const;
        const q3 = [f(nd[0] + f(r0 * u2[0])), f(nd[1] + f(r0 * u2[1])), f(nd[2] + f(r0 * u2[2]))] as const;
        const q4 = [f(nd[0] + f(r1 * u2[0])), f(nd[1] + f(r1 * u2[1])), f(nd[2] + f(r1 * u2[2]))] as const;
        const q5 = [f(pd[0] + f(r0 * u1[0])), f(pd[1] + f(r0 * u1[1])), f(pd[2] + f(r0 * u1[2]))] as const;
        const q6 = [f(pd[0] + f(r1 * u1[0])), f(pd[1] + f(r1 * u1[1])), f(pd[2] + f(r1 * u1[2]))] as const;
        const q7 = [f(pd[0] + f(r0 * u2[0])), f(pd[1] + f(r0 * u2[1])), f(pd[2] + f(r0 * u2[2]))] as const;
        const q8 = [f(pd[0] + f(r1 * u2[0])), f(pd[1] + f(r1 * u2[1])), f(pd[2] + f(r1 * u2[2]))] as const;

        const postHandle = this.runtime!.createHullFromPoints([...q1, ...q2, ...q3, ...q4, ...q5, ...q6, ...q7, ...q8]);
        this.runtime!.createShapeFromHull(drumBody, postHandle);
        this.runtime!.destroyHull(postHandle);
      }

      u1 = u2;
    }

    const a = f(0.2);
    const gridCount = 20;
    const cubeHandle = this.runtime!.createHullFromPoints([
      -a, -a, -a,  a, -a, -a,  a, a, -a,  -a, a, -a,
      -a, -a,  a,  a, -a,  a,  a, a,  a,  -a, a,  a,
    ]);
    const handles = Array.from({ length: WASHER_COUNT + 1 }) as number[];
    handles[0] = drumBody;
    let idx = 0;
    let x = f(f(f(-2) * a) * gridCount);
    for (let i = 0; i < gridCount; i++) {
      let y = f(f(f(-2) * a) * gridCount + f(21));
      for (let j = 0; j < gridCount; j++) {
        let z = f(f(f(-2) * a) * gridCount);
        for (let k = 0; k < gridCount; k++) {
          const bodyHandle = this.world!.createBody({ type: BodyType.Dynamic, position: [x, y, z], isAwake: true });
          this.runtime!.createShapeFromHull(bodyHandle, cubeHandle);
          handles[idx + 1] = bodyHandle;
          idx++;
          z = f(z + f(f(4) * a));
        }
        y = f(y + f(f(4) * a));
      }
      x = f(x + f(f(4) * a));
    }
    this.runtime!.destroyHull(cubeHandle);

    return handles;
  }
}

new WasherWorker();
