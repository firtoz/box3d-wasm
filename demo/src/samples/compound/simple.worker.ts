import { PhysicsWorkerBase } from "../../physics-worker-base";
import { B3_AXIS_Y, B3_PI, BodyType, type BodyHandle, type CompoundHandle, type Vec3 } from "box3d-wasm";

class CompoundSimpleWorker extends PhysicsWorkerBase {
  private compound: CompoundHandle | 0 = 0;

  protected getGroundSize(): Vec3 {
    return [20, 1, 20];
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    const handles: BodyHandle[] = [];

    // Clean up previous compound on restart
    if (this.compound !== 0) {
      this.runtime!.destroyCompound(this.compound);
    }

    // Create compound with one hull matching C++ SimpleCompound
    this.compound = this.runtime!.createCompoundFromHulls([
      {
        halfWidths: [4, 0.5, 4],
        transform: { position: [1, -0.5, 0], rotation: [0, 0, 0, 1] },
        friction: 0.5,
      },
    ]);

    // Static body with compound shape
    const q = this.runtime!.makeQuatFromAxisAngle(B3_AXIS_Y, 0.25 * B3_PI);
    const body = this.world!.createBody({ type: BodyType.Static, position: [2, -1, 0], rotation: q });
    this.runtime!.createCompoundShape(body, this.compound);
    handles.push(body);

    // Dynamic sphere
    const sphere = this.world!.createBody({ type: BodyType.Dynamic, position: [0, 2, 0], isAwake: true });
    this.runtime!.createSphereShape(sphere, [0, 0, 0], 0.25);
    handles.push(sphere);

    return handles;
  }
}

new CompoundSimpleWorker();
