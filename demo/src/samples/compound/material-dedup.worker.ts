import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { addBox } from "../shared-worker";

class CompoundMaterialDedupWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [12, 0.5, 12];
  }

  protected async buildScene(): Promise<BodyId[]> {
    const handles: BodyId[] = [];
    addBox(this.world!, this.runtime!, handles, [-2, 4, 0], [1, 1, 1], [0, 0, 0, 1], { friction: 0.3, density: 3000 });
    addBox(this.world!, this.runtime!, handles, [2, 4, 0], [1, 1, 1], [0, 0, 0, 1], { restitution: 0.5 });
    return handles;
  }
}

new CompoundMaterialDedupWorker();
