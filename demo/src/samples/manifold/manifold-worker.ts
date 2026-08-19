import {
  type Box3DRuntime,
  type LocalManifold,
  type Vec3,
  type WorldTransform,
  type BodyId,
} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  IDENTITY_XF,
  MANIFOLD_CONTACT_HEADER,
  MANIFOLD_CONTACT_STRIDE,
  MANIFOLD_COUNT_INDEX,
  MANIFOLD_FRAME_POS,
  MANIFOLD_FRAME_ROT,
  MANIFOLD_MAX_CONTACTS,
  MANIFOLD_NORMAL_INDEX,
  MANIFOLD_POINT_BASE,
  type ManifoldScene,
} from "./manifold-dump";

function writeContactBuffer(runtime: Box3DRuntime, manifold: LocalManifold, frame: WorldTransform, out: Float32Array): void {
  out[MANIFOLD_FRAME_POS] = frame.position[0];
  out[MANIFOLD_FRAME_POS + 1] = frame.position[1];
  out[MANIFOLD_FRAME_POS + 2] = frame.position[2];
  out[MANIFOLD_FRAME_ROT] = frame.rotation[0];
  out[MANIFOLD_FRAME_ROT + 1] = frame.rotation[1];
  out[MANIFOLD_FRAME_ROT + 2] = frame.rotation[2];
  out[MANIFOLD_FRAME_ROT + 3] = frame.rotation[3];
  out[MANIFOLD_COUNT_INDEX] = manifold.pointCount;
  const worldNormal = runtime.rotateVector(frame.rotation, manifold.normal);
  out[MANIFOLD_NORMAL_INDEX] = worldNormal[0];
  out[MANIFOLD_NORMAL_INDEX + 1] = worldNormal[1];
  out[MANIFOLD_NORMAL_INDEX + 2] = worldNormal[2];
  for (let i = 0; i < manifold.pointCount && i < MANIFOLD_MAX_CONTACTS; i++) {
    const local = manifold.points[i]!;
    const rotated = runtime.rotateVector(frame.rotation, local.point);
    const base = MANIFOLD_POINT_BASE + i * MANIFOLD_CONTACT_STRIDE;
    out[base] = frame.position[0] + rotated[0];
    out[base + 1] = frame.position[1] + rotated[1];
    out[base + 2] = frame.position[2] + rotated[2];
    out[base + 3] = local.separation;
  }
}

export class ManifoldWorker extends PhysicsWorkerBase {
  private collide: ((runtime: Box3DRuntime) => LocalManifold) | null = null;
  private disposeResources: (() => void) | null = null;
  private contactFrame: WorldTransform = IDENTITY_XF;
  private buffer: SharedArrayBuffer | null = null;
  private values: Float32Array | null = null;

  constructor(private readonly scene: ManifoldScene) {
    super();
  }

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<BodyId[]> {
    const resources = this.scene.create(this.runtime!);
    this.collide = resources.collide;
    this.disposeResources = resources.dispose;
    this.contactFrame = resources.contactFrame;
    this.buffer = new SharedArrayBuffer((MANIFOLD_CONTACT_HEADER + MANIFOLD_MAX_CONTACTS * MANIFOLD_CONTACT_STRIDE) * 4);
    this.values = new Float32Array(this.buffer);
    this.refresh();
    return [];
  }

  protected onBeforeDisposeWorld(): void {
    this.disposeResources?.();
    this.disposeResources = null;
    this.collide = null;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.buffer === null ? {} : { manifold: this.buffer };
  }

  protected stepPhysics(): void {
    this.totalSteps += 1;
    this.refresh();
  }

  private refresh(): void {
    if (this.runtime === null || this.collide === null || this.values === null) return;
    writeContactBuffer(this.runtime, this.collide(this.runtime), this.contactFrame, this.values);
  }
}
