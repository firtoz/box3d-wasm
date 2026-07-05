import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

const PI = Math.PI;

class CardHouseThickWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const alpha = 25 * PI / 180;
    const offsetX = 0.5 * 0.98 * Math.sin(alpha) + 0.045;
    const offsetY = 0.5 * 0.98 * Math.cos(alpha) + 0.035;
    const addPair = (startX: number, y: number, count: number) => {
      for (let j = 0; j < count; j++) {
        for (const sign of [-1, 1]) addBox(this.world!, this.runtime!, handles, [startX + sign * offsetX, y, 0], [0.04, 0.49, 0.19], [0, 0, Math.sin(sign * alpha / 2), Math.cos(sign * alpha / 2)], { friction: 0.8 });
        startX += 4 * offsetX;
      }
    };
    const addRow = (startX: number, y: number, count: number) => {
      for (let i = 0; i < count; i++) addBox(this.world!, this.runtime!, handles, [startX + i * 4 * offsetX, y, 0], [0.04, 0.49, 0.19], [0, 0, Math.sin(PI / 4), Math.cos(PI / 4)], { friction: 0.8 });
    };
    addPair(-6 * offsetX, offsetY, 4); addRow(-4 * offsetX, 2 * offsetY + 0.04, 3); addPair(-4 * offsetX, 3 * offsetY + 0.08, 3); addRow(-2 * offsetX, 4 * offsetY + 0.12, 2); addPair(-2 * offsetX, 5 * offsetY + 0.16, 2); addRow(0, 6 * offsetY + 0.20, 1); addPair(0, 7 * offsetY + 0.24, 1);
    return handles;
  }
}

new CardHouseThickWorker();
