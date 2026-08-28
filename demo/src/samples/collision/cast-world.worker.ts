import { BodyId, Vec3, WorldCastMode, WorldCastType, type AABB, type WorldCastModeId, type WorldCastTypeId } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { Box3DRng } from "../box3d-rng";
import {
  CAST_WORLD_BODY_STRIDE,
  CAST_WORLD_HEADER_FLOATS,
  CAST_WORLD_HIT_CAPACITY,
  CAST_WORLD_HIT_STRIDE,
  CAST_WORLD_MAX,
  castWorldDefaultMode,
  castWorldDefaultOrigin,
  castWorldDefaultRadius,
  castWorldDefaultTranslation,
  castWorldDefaultType,
  castWorldGroundSize,
  createCastWorldResources,
  createCastWorldShape,
  runCastWorld,
  writeCastWorldBuffer,
  type CastWorldResources,
} from "./cast-world-scene";

class CastWorldWorker extends PhysicsWorkerBase {
  private slots: (BodyId | 0n)[] = Array.from({ length: CAST_WORLD_MAX }, () => 0n);
  private kinds: number[] = Array.from({ length: CAST_WORLD_MAX }, () => -1);
  private bodyIndex = 0;
  private rng = new Box3DRng();
  private resources: CastWorldResources | null = null;
  private origin: Vec3 = [...castWorldDefaultOrigin];
  private translation: Vec3 = [...castWorldDefaultTranslation];
  private mode: WorldCastModeId = castWorldDefaultMode;
  private castType: WorldCastTypeId = castWorldDefaultType;
  private radius = castWorldDefaultRadius;
  private initialOverlap = false;
  private extraBuffer: SharedArrayBuffer | null = null;
  private extraValues: Float32Array | null = null;

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return castWorldGroundSize();
  }

  protected getTrackedBodyCapacity(): number {
    return CAST_WORLD_MAX;
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.slots = Array.from({ length: CAST_WORLD_MAX }, () => 0n);
    this.kinds = Array.from({ length: CAST_WORLD_MAX }, () => -1);
    this.bodyIndex = 0;
    this.rng = new Box3DRng();
    this.origin = [...castWorldDefaultOrigin];
    this.translation = [...castWorldDefaultTranslation];
    this.mode = castWorldDefaultMode;
    this.castType = castWorldDefaultType;
    this.radius = castWorldDefaultRadius;
    this.initialOverlap = false;
    this.resources = createCastWorldResources(this.world!);
    const floats = CAST_WORLD_HEADER_FLOATS + CAST_WORLD_HIT_CAPACITY * CAST_WORLD_HIT_STRIDE + CAST_WORLD_MAX * CAST_WORLD_BODY_STRIDE;
    this.extraBuffer = new SharedArrayBuffer(floats * 4);
    this.extraValues = new Float32Array(this.extraBuffer);
    this.capture();
    return [];
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.extraBuffer === null ? {} : { cast: this.extraBuffer };
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    this.capture();
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "cast-pick" && Array.isArray(msg.origin) && Array.isArray(msg.translation)) {
      this.origin = [Number(msg.origin[0]), Number(msg.origin[1]), Number(msg.origin[2])];
      this.translation = [Number(msg.translation[0]), Number(msg.translation[1]), Number(msg.translation[2])];
      this.capture();
      return true;
    }
    if (msg.type === "set-mode") {
      this.mode = clampEnum(Number(msg.value), WorldCastMode.Any, WorldCastMode.Sorted) as WorldCastModeId;
      this.capture();
      return true;
    }
    if (msg.type === "set-type") {
      this.castType = clampEnum(Number(msg.value), WorldCastType.Ray, WorldCastType.Box) as WorldCastTypeId;
      this.capture();
      return true;
    }
    if (msg.type === "set-radius") {
      this.radius = Number(msg.value);
      this.capture();
      return true;
    }
    if (msg.type === "set-overlap") {
      this.initialOverlap = msg.value === true;
      this.capture();
      return true;
    }
    if (msg.type === "spawn") {
      this.spawn(Number(msg.kind), Number(msg.count));
      return true;
    }
    if (msg.type === "destroy") {
      this.destroyFirst();
      return true;
    }
    return false;
  }

  private liveHandles(): BodyId[] {
    const handles: BodyId[] = [];
    for (const slot of this.slots) {
      if (slot !== 0n) handles.push(slot);
    }
    return handles;
  }

  private spawn(kind: number, count: number): void {
    if (this.world === null || this.runtime === null || this.resources === null) return;
    for (let i = 0; i < count; i++) {
      const index = this.bodyIndex;
      const existing = this.slots[index];
      if (existing !== 0n && existing !== undefined) this.world.destroyBody(existing);
      const body = createCastWorldShape(this.world, this.runtime, this.resources, this.rng, index, kind);
      this.slots[index] = body;
      this.kinds[index] = kind;
      this.bodyIndex = (this.bodyIndex + 1) % CAST_WORLD_MAX;
    }
    this.setTrackedBodies(this.liveHandles());
    this.capture();
  }

  private destroyFirst(): void {
    if (this.world === null) return;
    for (let i = 0; i < CAST_WORLD_MAX; i++) {
      const existing = this.slots[i];
      if (existing === 0n || existing === undefined) continue;
      this.world.destroyBody(existing);
      this.slots[i] = 0n;
      this.kinds[i] = -1;
      this.setTrackedBodies(this.liveHandles());
      this.capture();
      return;
    }
  }

  private capture(): void {
    if (this.world === null || this.extraValues === null) return;
    const hits = runCastWorld(this.world, this.origin, this.translation, this.mode, this.castType, this.radius, this.initialOverlap);
    const bodies: { kind: number; ignore: number; aabb: AABB }[] = [];
    for (let i = 0; i < CAST_WORLD_MAX; i++) {
      const handle = this.slots[i];
      if (handle === 0n || handle === undefined) continue;
      const kind = this.kinds[i] ?? -1;
      bodies.push({
        kind,
        ignore: (i & 0x7) === 0x7 ? 1 : 0,
        aabb: this.world.computeBodyAABB(handle),
      });
    }
    writeCastWorldBuffer(
      this.extraValues,
      this.origin,
      this.translation,
      this.mode,
      this.castType,
      this.radius,
      this.initialOverlap,
      hits,
      bodies,
    );
  }
}

function clampEnum(value: number, lo: number, hi: number): number {
  if (!Number.isFinite(value)) return lo;
  return Math.max(lo, Math.min(hi, value | 0));
}

new CastWorldWorker();
