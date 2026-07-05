import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";

class DisableWorker extends PhysicsWorkerBase {
  private linkIds: number[] = [];
  private ballId = 0;

  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const W = this.world!;
    const R = this.runtime!;
    const handles: number[] = [];

    const capsuleHalf = 0.5;
    const capsuleRadius = 0.25;
    const spacing = 2.5;
    const numLinks = 4;
    const startY = 8;

    for (let i = 0; i < numLinks; i++) {
      const body = W.createBody({
        type: i === 0 ? BodyType.Kinematic : BodyType.Dynamic,
        position: [0, startY - i * spacing, 0],
      });
      R.createCapsuleShape(body, [0, -capsuleHalf, 0], [0, capsuleHalf, 0], capsuleRadius, { density: 1 });
      this.linkIds.push(body);
      handles.push(body);
    }

    for (let i = 0; i < numLinks - 1; i++) {
      const midY = startY - i * spacing - spacing / 2;
      W.createWeldJoint(this.linkIds[i], this.linkIds[i + 1], {
        localFrameA: { position: R.getBodyLocalPoint(this.linkIds[i], [0, midY, 0]) },
        localFrameB: { position: R.getBodyLocalPoint(this.linkIds[i + 1], [0, midY, 0]) },
      });
    }

    this.ballId = W.createBody({
      type: BodyType.Dynamic,
      position: [-5, 4, 0],
      gravityScale: 0,
    });
    R.createSphereShape(this.ballId, [0, 0, 0], 0.3, { density: 1 });
    handles.push(this.ballId);

    return handles;
  }

  protected stepPhysics(): number {
    const start = performance.now();
    this.world!.step(this.fixedTimeStep, this.subSteps);
    this.runtime!.applyLinearImpulseToCenter(this.linkIds[2], [0.5, 0, 0], true);
    return performance.now() - start;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const R = this.runtime!;
    switch (msg.type) {
      case "enableLink2": {
        if (msg.value) { R.bodyEnable(this.linkIds[2]); } else { R.bodyDisable(this.linkIds[2]); }
        return true;
      }
      case "enableBall": {
        if (msg.value) { R.bodyEnable(this.ballId); } else { R.bodyDisable(this.ballId); }
        return true;
      }
    }
    return false;
  }
}

new DisableWorker();
