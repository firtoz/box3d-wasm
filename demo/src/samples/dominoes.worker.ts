import { PhysicsWorkerBase } from "../physics-worker-base";

class DominoesWorker extends PhysicsWorkerBase<{ multiplier?: number }> {
  protected async buildScene(initData: { multiplier?: number }): Promise<number[]> {
    const rings = 30 * (initData.multiplier ?? 1);
    const count = rings * 180;
    const handles = Array.from({ length: count }) as number[];
    let idx = 0;
    for (let ring = 0; ring < rings; ring++) {
      const scale = 0.5 + ring * 0.0585;
      const radius = 7.0 + (1.5 + ring * 0.015) * ring;
      const n = 1.515 + ring * 0.03;
      for (let deg = 0; deg < 360; deg += 2) {
        const rad = deg * Math.PI / 180;
        const cs = Math.cos(rad);
        const sn = Math.sin(rad);
        const px = radius * cs + (deg * n / 716) * cs;
        const pz = radius * sn + (deg * n / 716) * sn;
        const p: [number, number, number] = [px, 0.8 * scale, pz];
        const bodyHandle = this.world!.createBody({ type: 2, position: p, rotation: [0, -Math.sin(rad / 2), 0, Math.cos(rad / 2)], isAwake: true });
        this.runtime!.createHullShape(bodyHandle, [0.2 * scale, 0.8 * scale, 0.05 * scale]);
        handles[idx] = bodyHandle;
        if (ring % 2 === 0 ? Math.abs(deg - 358) < 0.1 : Math.abs(deg) < 0.1) {
          const dir = ring % 2 === 0 ? -1 : 1;
          this.world!.applyLinearImpulse(bodyHandle, [0, 0, dir * 25 * scale * scale * scale], [p[0], p[1] + 0.8 * scale, p[2]]);
        }
        idx++;
      }
    }
    return handles;
  }
}

new DominoesWorker();
