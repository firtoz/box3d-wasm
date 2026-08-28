import type { BodyId, HeightFieldHandle, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildRigidBodyLive, rigidBodyGroundSize, rigidWalkSpeed } from "./rigid-body-scene";

class RigidBodyWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];
  private heightField: HeightFieldHandle | null = null;
  private character: BodyId | null = null;
  private throttleX = 0;
  private throttleY = 0;
  private forward: Vec3 = [0, 0, -1];
  private right: Vec3 = [1, 0, 0];

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return rigidBodyGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const built = await buildRigidBodyLive(this.world!, this.runtime!);
    this.meshes = built.meshes;
    this.heightField = built.heightField;
    this.character = built.handles[0] ?? null;
    return built.handles;
  }

  protected stepPhysics(): void {
    if (this.world === null || this.character === null) return;
    const v = this.world.getBodyLinearVelocity(this.character);
    const wish: Vec3 = [
      this.throttleX * this.forward[0] + this.throttleY * this.right[0],
      0,
      this.throttleX * this.forward[2] + this.throttleY * this.right[2],
    ];
    const len = Math.hypot(wish[0], wish[2]);
    if (len > 1e-4) {
      wish[0] = (wish[0] / len) * rigidWalkSpeed;
      wish[2] = (wish[2] / len) * rigidWalkSpeed;
    }
    this.world.setBodyLinearVelocity(this.character, [wish[0], v[1], wish[2]]);
    this.world.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "drive-throttle") {
      if (typeof msg.x === "number") this.throttleX = msg.x;
      if (typeof msg.y === "number") this.throttleY = -msg.y;
      return true;
    }
    if (msg.type === "drive-view") {
      if (typeof msg.fx === "number") this.forward = [msg.fx, 0, msg.fz as number];
      if (typeof msg.rx === "number") this.right = [msg.rx, 0, msg.rz as number];
      const fl = Math.hypot(this.forward[0], this.forward[2]);
      if (fl > 0.001) {
        this.forward[0] /= fl;
        this.forward[2] /= fl;
      }
      return true;
    }
    return false;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world === null) return;
    for (let i = this.meshes.length - 1; i >= 0; i--) this.world.destroyMesh(this.meshes[i]!);
    if (this.heightField !== null) this.world.destroyHeightField(this.heightField);
    this.meshes = [];
    this.heightField = null;
  }
}

new RigidBodyWorker();
