/**
 * One-time (or toggleable) install of assertActive wrappers on object-API prototypes.
 *
 * Class methods are authored bare (no assert calls). When asserts are compiled in,
 * `installObjectAssertGuards` wraps the listed methods once at module load so each
 * call runs `assertActive` then the bare body. Benches can uninstall to measure
 * true bare BodyRef cost (path B) vs asserts-on with guards (path C).
 */
import { areObjectAssertsCompiledIn } from "./object-assert-flags";

// eslint-disable-next-line @typescript-eslint/no-explicit-any
type AnyFn = (...args: any[]) => any;

export type ObjectAssertGuardTarget = {
  proto: object;
  methods: readonly string[];
  assert: (self: object) => void;
};

/** Joint creators that must also assert both BodyRef args. */
export type ObjectAssertBodyPairTarget = {
  proto: object;
  methods: readonly string[];
  assertWorld: (self: object) => void;
};

type GuardRegistry = {
  targets: ObjectAssertGuardTarget[];
  bodyPairs: ObjectAssertBodyPairTarget[];
};

const bareByProto = new Map<object, Map<string, AnyFn>>();
let registry: GuardRegistry | null = null;
let installed = false;

function saveBare(proto: object, name: string, fn: AnyFn): void {
  let map = bareByProto.get(proto);
  if (map === undefined) {
    map = new Map();
    bareByProto.set(proto, map);
  }
  if (!map.has(name)) map.set(name, fn);
}

function wrapSelfAssert(proto: object, name: string, assert: (self: object) => void): void {
  const record = proto as Record<string, AnyFn>;
  let bare = bareByProto.get(proto)?.get(name);
  if (bare === undefined) {
    bare = record[name];
    if (typeof bare !== "function") {
      throw new Error(`object assert guard: missing method ${String(name)}`);
    }
    saveBare(proto, name, bare);
  }
  record[name] = function (this: object, ...args: unknown[]) {
    assert(this);
    return bare!.apply(this, args);
  };
}

function wrapBodyPair(
  proto: object,
  name: string,
  assertWorld: (self: object) => void,
): void {
  const record = proto as Record<string, AnyFn>;
  let bare = bareByProto.get(proto)?.get(name);
  if (bare === undefined) {
    bare = record[name];
    if (typeof bare !== "function") {
      throw new Error(`object assert guard: missing body-pair method ${String(name)}`);
    }
    saveBare(proto, name, bare);
  }
  record[name] = function (
    this: object,
    bodyA: { assertActive(): void },
    bodyB: { assertActive(): void },
    ...rest: unknown[]
  ) {
    assertWorld(this);
    bodyA.assertActive();
    bodyB.assertActive();
    return bare!.call(this, bodyA, bodyB, ...rest);
  };
}

function applyInstall(reg: GuardRegistry): void {
  for (const target of reg.targets) {
    for (const name of target.methods) {
      wrapSelfAssert(target.proto, name, target.assert);
    }
  }
  for (const target of reg.bodyPairs) {
    for (const name of target.methods) {
      wrapBodyPair(target.proto, name, target.assertWorld);
    }
  }
  installed = true;
}

function applyUninstall(): void {
  for (const [proto, methods] of bareByProto) {
    const record = proto as Record<string, AnyFn>;
    for (const [name, fn] of methods) {
      record[name] = fn;
    }
  }
  installed = false;
}

/** Install guards from a registry (idempotent). No-op when compile asserts are stripped. */
export function installObjectAssertGuards(reg: GuardRegistry): void {
  registry = reg;
  if (!areObjectAssertsCompiledIn()) return;
  if (installed) return;
  applyInstall(reg);
}

/** Restore bare prototypes (bench path B). */
export function uninstallObjectAssertGuards(): void {
  if (!installed) return;
  applyUninstall();
}

/**
 * Toggle whether listed methods are wrapped with assertActive.
 * When compile asserts are stripped, always stays bare.
 */
export function setObjectAssertGuardsEnabled(enabled: boolean): void {
  if (!areObjectAssertsCompiledIn()) return;
  if (registry === null) {
    throw new Error("setObjectAssertGuardsEnabled: guards were never registered");
  }
  if (enabled && !installed) applyInstall(registry);
  else if (!enabled && installed) applyUninstall();
}

export function areObjectAssertGuardsInstalled(): boolean {
  return installed;
}
