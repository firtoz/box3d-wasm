import type { BodyId, HeightFieldHandle, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { CharacterMover } from "./character-mover";
import { buildMoverLive, moverGroundSize, moverStart } from "./mover-scene";

class MoverWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];
  private heightField: HeightFieldHandle | null = null;
  private mover: CharacterMover | null = null;
  private posBuffer: SharedArrayBuffer | null = null;
  private posValues: Float32Array | null = null;
  private throttleX = 0;
  private throttleY = 0;
  private forward: Vec3 = [0, 0, -1];
  private right: Vec3 = [1, 0, 0];
  private clipVelocity = true;

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return moverGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const built = await buildMoverLive(this.world!, this.runtime!);
    this.meshes = built.meshes;
    this.heightField = built.heightField;
    this.mover = new CharacterMover(moverStart);
    this.posBuffer = new SharedArrayBuffer(12);
    this.posValues = new Float32Array(this.posBuffer);
    this.writePos();
    return built.handles;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.posBuffer === null ? {} : { mover: this.posBuffer };
  }

  protected stepPhysics(): void {
    if (this.world === null || this.mover === null) return;
    this.mover.step(this.world, this.fixedTimeStep, this.forward, this.right, { x: this.throttleX, y: this.throttleY }, this.clipVelocity);
    this.world.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
    this.writePos();
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
    if (msg.type === "jump") {
      if (this.mover !== null) this.mover.jumpQueued = true;
      return true;
    }
    if (msg.type === "sprint" && typeof msg.value === "boolean") {
      if (this.mover !== null) this.mover.sprint = msg.value;
      return true;
    }
    if (msg.type === "clip" && typeof msg.value === "boolean") {
      this.clipVelocity = msg.value;
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

  private writePos(): void {
    if (this.posValues === null || this.mover === null) return;
    this.posValues[0] = this.mover.position[0];
    this.posValues[1] = this.mover.position[1];
    this.posValues[2] = this.mover.position[2];
  }
}

new MoverWorker();
