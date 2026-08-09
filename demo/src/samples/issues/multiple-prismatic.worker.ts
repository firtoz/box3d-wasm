import { PhysicsWorkerBase } from "../../physics-worker-base";
import {BodyType, type BodyId, type Vec3} from "box3d-wasm";
import { buildMultiplePrismaticDynamicBodies, multiplePrismaticGroundSize } from "./multiple-prismatic-scene";

class MultiplePrismaticWorker extends PhysicsWorkerBase {
  private anchor: BodyId = 0n as BodyId;

  protected setupGround(): void {
    // Upstream uses an empty static body as the first prismatic anchor (no hull).
    this.anchor = this.world!.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  }

  protected getGroundSize(): Vec3 {
    return multiplePrismaticGroundSize();
  }

  /** Upstream sets m_mouseForceScale = 1e6 so dragging can yank the chain hard. */
  protected getMouseForceScale(): number {
    return 1_000_000;
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildMultiplePrismaticDynamicBodies(this.world!, this.runtime!, this.anchor);
  }
}

new MultiplePrismaticWorker();
