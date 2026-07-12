import { mkdir, readFile, readdir, writeFile } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";
import { BodyType, Box3DRuntime, type PhysicsWorld, type Quat, type Vec3 } from "../packages/box3d-wasm/src/index";

type ModuleFactory = (options: { wasmBinary: ArrayBuffer; locateFile(path: string): string }) => Promise<unknown>;
type ModuleImport = { default: ModuleFactory };

interface DumpBody {
  p: Vec3;
  q: Quat;
  v: Vec3;
  w: Vec3;
  t: BodyType;
  a: boolean;
}

interface DumpCheckpoint {
  frame: number;
  bodies: DumpBody[];
  rays?: unknown;
}

interface DumpOutput {
  checkpoints: DumpCheckpoint[];
}

interface DumpInteraction {
  frame: number;
  action: string;
  args?: readonly number[];
}

interface WasmDumpInstance {
  world: PhysicsWorld;
  handles: number[];
  state?: unknown;
  /** Optional cleanup after `world.destroy` (e.g. restore global stall threshold). */
  dispose?: () => void;
}

interface WasmDumpSample {
  id: string;
  name: string;
  cppName: string;
  create(runtime: Box3DRuntime): WasmDumpInstance;
  step?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], frame: number, dt: number, state: unknown) => void;
  postStep?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], frame: number, dt: number, state: unknown) => void;
  interactionSchedule: readonly DumpInteraction[];
  runInteraction?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], interaction: DumpInteraction, frame: number, state: unknown) => void;
  checkpointExtras?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], frame: number, state: unknown) => Record<string, unknown> | undefined;
}

interface SceneDumpModule {
  dumpSampleId?: string;
  dumpSampleName?: string;
  dumpCppSampleName?: string;
  dumpCreate?: (runtime: Box3DRuntime) => WasmDumpInstance;
  dumpGroundSize?: () => Vec3;
  dumpGroundPosition?: Vec3;
  dumpBuildDynamicBodies?: (world: PhysicsWorld, runtime: Box3DRuntime) => number[];
  dumpNoPhysics?: boolean;
  dumpStep?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], frame: number, dt: number, state: unknown) => void;
  dumpPostStep?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], frame: number, dt: number, state: unknown) => void;
  dumpInteractionSchedule?: readonly DumpInteraction[];
  dumpRunInteraction?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], interaction: DumpInteraction, frame: number, state: unknown) => void;
  dumpCheckpointExtras?: (world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], frame: number, state: unknown) => Record<string, unknown> | undefined;
  dumpVariants?: readonly {
    id: string;
    name: string;
    cppName: string;
    interactionSchedule: readonly DumpInteraction[];
  }[];
}

interface FrontendSampleMeta {
  id: string;
  name: string;
}

interface Options {
  sampleName: string | undefined;
  outputPath: string | undefined;
  checkpointInterval: number;
  maxFrame: number;
  startFrame: number;
  exactFrames: number[];
  maxFrameExplicit: boolean;
  disableSleepTerm: boolean;
  listJson: boolean;
  help: boolean;
}

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(scriptDir, "..");
const wasmDir = join(repoRoot, "demo", "public", "wasm");
const samplesDir = join(repoRoot, "demo", "src", "samples");

function usage(samples: readonly WasmDumpSample[] = []): string {
  const supported = samples.length > 0 ? samples.map((sample) => `${sample.name} (${sample.id})`).join(", ") : "frontend samples with dump-enabled *-scene.ts modules";
  return [
    "Usage: wasm-dump [options] <SampleName> [output.json]",
    "Options:",
    "  --help                         Show this help",
    "  --list-json                    Print supported WASM dump samples as JSON",
    "  --checkpoint-interval <frames> Emit periodic checkpoints (default: 50)",
    "  --max-frames <frame>           Last frame to simulate (default: 300)",
    "  --start-frame <frame>          First frame eligible for dumping (default: 0)",
    "  --frames <a,b,c>               Dump exact frames instead of periodic checkpoints",
    "  --disable-sleep-term           Keep simulating even after bodies sleep",
    "",
    `Supported samples: ${supported}`,
  ].join("\n");
}

function parseNonNegativeInt(value: string, label: string): number {
  if (!/^\d+$/.test(value)) throw new Error(`Invalid integer for ${label}: ${value}`);
  return Number(value);
}

function parseFrameList(value: string): number[] {
  const frames = value.split(",").map((part) => parseNonNegativeInt(part, "--frames"));
  if (frames.length === 0) throw new Error("--frames requires at least one frame");
  return frames;
}

function parseOptions(argv: string[]): Options {
  const options: Options = { sampleName: undefined, outputPath: undefined, checkpointInterval: 50, maxFrame: 300, startFrame: 0, exactFrames: [], maxFrameExplicit: false, disableSleepTerm: false, listJson: false, help: false };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i]!;
    if (arg === "--help") {
      options.help = true;
    } else if (arg === "--list-json") {
      options.listJson = true;
    } else if (arg === "--disable-sleep-term") {
      options.disableSleepTerm = true;
    } else if (arg === "--checkpoint-interval" || arg === "--max-frames" || arg === "--start-frame" || arg === "--frames") {
      const value = argv[++i];
      if (value === undefined) throw new Error(`Missing value for ${arg}`);
      if (arg === "--frames") {
        options.exactFrames = parseFrameList(value);
      } else {
        const parsed = parseNonNegativeInt(value, arg);
        if (arg === "--checkpoint-interval") {
          if (parsed === 0) throw new Error("--checkpoint-interval must be greater than zero");
          options.checkpointInterval = parsed;
        } else if (arg === "--max-frames") {
          options.maxFrame = parsed;
          options.maxFrameExplicit = true;
        } else {
          options.startFrame = parsed;
        }
      }
    } else if (arg.startsWith("-")) {
      throw new Error(`Unknown option: ${arg}`);
    } else if (options.sampleName === undefined) {
      options.sampleName = arg;
    } else if (options.outputPath === undefined) {
      options.outputPath = arg;
    } else {
      throw new Error(`Unexpected argument: ${arg}`);
    }
  }

  if (!options.maxFrameExplicit && options.exactFrames.length > 0) {
    options.maxFrame = Math.max(options.maxFrame, ...options.exactFrames);
  }

  return options;
}

function shouldDumpFrame(options: Options, frame: number): boolean {
  if (frame < options.startFrame) return false;
  if (options.exactFrames.length > 0) return options.exactFrames.includes(frame);
  return frame % options.checkpointInterval === 0;
}

async function loadRuntime(): Promise<Box3DRuntime> {
  const jsPath = join(wasmDir, "box3d-web.js");
  const wasmPath = join(wasmDir, "box3d-web.wasm");
  const oldWindow = globalThis.window;
  const oldProcess = globalThis.process;

  // The current Emscripten artifact is web-only. Hide Node while importing,
  // but keep nextTick for Emscripten pthread startup internals under Bun.
  Object.assign(globalThis, { window: {} });
  Object.assign(globalThis, { process: { nextTick: oldProcess.nextTick.bind(oldProcess) } });

  try {
    const moduleImport = (await import(pathToFileURL(jsPath).href)) as ModuleImport;
    const wasmBinary = await Bun.file(wasmPath).arrayBuffer();
    const module = await moduleImport.default({ wasmBinary, locateFile: (path: string) => pathToFileURL(join(wasmDir, path)).href });
    return new Box3DRuntime(module as ConstructorParameters<typeof Box3DRuntime>[0]);
  } finally {
    Object.assign(globalThis, { process: oldProcess });
    if (oldWindow === undefined) {
      delete (globalThis as { window?: unknown }).window;
    } else {
      Object.assign(globalThis, { window: oldWindow });
    }
  }
}

async function findSceneFiles(dir: string): Promise<string[]> {
  const entries = await readdir(dir, { withFileTypes: true });
  const files: string[] = [];
  for (const entry of entries) {
    const path = join(dir, entry.name);
    if (entry.isDirectory()) {
      files.push(...await findSceneFiles(path));
    } else if (entry.isFile() && entry.name.endsWith("-scene.ts")) {
      files.push(path);
    }
  }
  return files;
}

async function loadFrontendSampleMetadata(): Promise<FrontendSampleMeta[]> {
  const indexPath = join(samplesDir, "index.ts");
  const indexSource = await readFile(indexPath, "utf8");
  const importedFiles: string[] = [];
  const importPattern = /from\s+["'](\.\/[^"']+)["']/g;
  for (const match of indexSource.matchAll(importPattern)) {
    const importPath = match[1]!;
    if (importPath === "./types") continue;
    importedFiles.push(join(samplesDir, `${importPath.slice(2)}.ts`));
  }

  const metadata: FrontendSampleMeta[] = [];
  const seen = new Set<string>();
  const genericPattern = /createGenericSample\(\s*["']([^"']+)["']\s*,\s*["']([^"']+)["']/g;
  const objectPattern = /id:\s*["']([^"']+)["'][\s\S]{0,300}?name:\s*["']([^"']+)["']/g;
  for (const file of importedFiles) {
    let source: string;
    try {
      source = await readFile(file, "utf8");
    } catch {
      continue;
    }
    for (const pattern of [genericPattern, objectPattern]) {
      pattern.lastIndex = 0;
      for (const match of source.matchAll(pattern)) {
        const id = match[1]!;
        const name = match[2]!;
        if (seen.has(id)) continue;
        seen.add(id);
        metadata.push({ id, name });
      }
    }
  }
  return metadata;
}

async function loadWasmDumpSamples(): Promise<WasmDumpSample[]> {
  const sceneById = new Map<string, SceneDumpModule>();
  for (const file of await findSceneFiles(samplesDir)) {
    const scene = (await import(pathToFileURL(file).href)) as SceneDumpModule;
    if (scene.dumpSampleId === undefined) continue;
    if (scene.dumpCreate === undefined && (scene.dumpGroundSize === undefined || scene.dumpBuildDynamicBodies === undefined)) continue;
    sceneById.set(scene.dumpSampleId, scene);
  }

  function makeCreate(scene: SceneDumpModule): (runtime: Box3DRuntime) => WasmDumpInstance {
    return (runtime) => {
      if (scene.dumpCreate !== undefined) return scene.dumpCreate(runtime);
      if (scene.dumpNoPhysics) {
        const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
        return { world, handles: [] };
      }
      const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
      const ground = world.createBody({ type: BodyType.Static, position: scene.dumpGroundPosition ?? [0, -1, 0] });
      runtime.createHullShape(ground, scene.dumpGroundSize!());
      return { world, handles: [ground, ...scene.dumpBuildDynamicBodies!(world, runtime)] };
    };
  }

  const samples: WasmDumpSample[] = [];
  for (const frontendSample of await loadFrontendSampleMetadata()) {
    const scene = sceneById.get(frontendSample.id);
    if (scene === undefined) continue;
    samples.push({
      id: frontendSample.id,
      name: frontendSample.name,
      cppName: scene.dumpCppSampleName ?? scene.dumpSampleName ?? frontendSample.name,
      create: makeCreate(scene),
      step: scene.dumpStep,
      postStep: scene.dumpPostStep,
      interactionSchedule: scene.dumpInteractionSchedule ?? [],
      runInteraction: scene.dumpRunInteraction,
      checkpointExtras: scene.dumpCheckpointExtras,
    });
    for (const variant of scene.dumpVariants ?? []) {
      samples.push({
        id: variant.id,
        name: variant.name,
        cppName: variant.cppName,
        create: makeCreate(scene),
        step: scene.dumpStep,
        postStep: scene.dumpPostStep,
        interactionSchedule: variant.interactionSchedule,
        runInteraction: scene.dumpRunInteraction,
        checkpointExtras: scene.dumpCheckpointExtras,
      });
    }
  }
  return samples;
}

function findSample(samples: readonly WasmDumpSample[], name: string): WasmDumpSample | undefined {
  return samples.find((sample) => sample.id === name || sample.name === name || sample.cppName === name);
}

function dumpBodies(world: PhysicsWorld, handles: readonly number[]): DumpBody[] {
  return handles.map((handle) => {
    const transform = world.getBodyTransform(handle);
    return {
      p: transform.position,
      q: transform.rotation,
      v: world.getBodyLinearVelocity(handle),
      w: world.getBodyAngularVelocity(handle),
      t: world.getBodyType(handle),
      a: world.bodyIsAwake(handle),
    };
  });
}

async function main(): Promise<void> {
  const options = parseOptions(process.argv.slice(2));
  const samples = await loadWasmDumpSamples();
  if (options.help) {
    console.log(usage(samples));
    return;
  }
  if (options.listJson) {
    console.log(JSON.stringify({ samples: samples.map((sample) => ({ id: sample.id, name: sample.name, cppName: sample.cppName })) }));
    return;
  }
  if (options.sampleName === undefined) throw new Error(`${usage(samples)}\n\nMissing sample name`);
  const sample = findSample(samples, options.sampleName);
  if (sample === undefined) throw new Error(`Unsupported sample: ${options.sampleName}`);

  console.error(`Running WASM sample: ${sample.name}`);
  const runtime = await loadRuntime();
  const instance = sample.create(runtime);
  const { world, handles, state } = instance;
  const output: DumpOutput = { checkpoints: [] };
  const dt = Math.fround(1 / 60);

  for (let frame = 0; frame <= options.maxFrame; frame++) {
    for (const interaction of sample.interactionSchedule) {
      if (interaction.frame === frame) {
        if (sample.runInteraction === undefined) throw new Error(`Sample ${sample.id} is missing dumpRunInteraction for action ${interaction.action} at frame ${frame}`);
        sample.runInteraction(world, runtime, handles, interaction, frame, state);
      }
    }
    if (frame > 0) {
      sample.step?.(world, runtime, handles, frame, dt, state);
      world.step(dt, 4);
      sample.postStep?.(world, runtime, handles, frame, dt, state);
    }
    if (shouldDumpFrame(options, frame)) {
      const extras = sample.checkpointExtras?.(world, runtime, handles, frame, state) ?? {};
      output.checkpoints.push({ frame, bodies: dumpBodies(world, handles), ...extras });
      if (sample.step === undefined && sample.interactionSchedule.length === 0 && !options.disableSleepTerm && world.getAwakeBodyCount() === 0 && frame >= 100) {
        console.error(`All bodies asleep at frame ${frame}, terminating.`);
        break;
      }
    }
  }

  world.destroy();
  instance.dispose?.();
  runtime.destroy();

  const json = `${JSON.stringify(output)}\n`;
  if (options.outputPath !== undefined) {
    await mkdir(dirname(options.outputPath), { recursive: true });
    await writeFile(options.outputPath, json);
  } else {
    process.stdout.write(json);
  }
  console.error(`Done. ${output.checkpoints.length} checkpoints written.`);
}

try {
  await main();
} catch (error) {
  console.error(error instanceof Error ? error.message : error);
  process.exit(1);
}

process.exit(0);
