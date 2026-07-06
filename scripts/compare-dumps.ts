import { readFile } from "node:fs/promises";

type Vec3 = [number, number, number];
type Quat = [number, number, number, number];

interface DumpBody {
  p: Vec3;
  q: Quat;
  v: Vec3;
  w: Vec3;
  t: number;
  a: boolean;
}

interface DumpCheckpoint {
  frame: number;
  bodies: DumpBody[];
}

interface DumpOutput {
  checkpoints: DumpCheckpoint[];
}

interface Options {
  expectedPath: string | undefined;
  actualPath: string | undefined;
  epsilon: number;
  help: boolean;
}

function usage(): string {
  return [
    "Usage: compare-dumps [options] <expected.json> <actual.json>",
    "Options:",
    "  --help              Show this help",
    "  --epsilon <number>  Numeric tolerance for vector fields (default: 1e-5)",
  ].join("\n");
}

function parseOptions(argv: string[]): Options {
  const options: Options = { expectedPath: undefined, actualPath: undefined, epsilon: 1e-5, help: false };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i]!;
    if (arg === "--help") {
      options.help = true;
    } else if (arg === "--epsilon") {
      const value = argv[++i];
      if (value === undefined) throw new Error("Missing value for --epsilon");
      options.epsilon = Number(value);
      if (!Number.isFinite(options.epsilon) || options.epsilon < 0) throw new Error(`Invalid epsilon: ${value}`);
    } else if (arg.startsWith("-")) {
      throw new Error(`Unknown option: ${arg}`);
    } else if (options.expectedPath === undefined) {
      options.expectedPath = arg;
    } else if (options.actualPath === undefined) {
      options.actualPath = arg;
    } else {
      throw new Error(`Unexpected argument: ${arg}`);
    }
  }
  return options;
}

async function loadDump(path: string): Promise<DumpOutput> {
  const parsed = JSON.parse(await readFile(path, "utf8")) as DumpOutput;
  if (!Array.isArray(parsed.checkpoints)) throw new Error(`${path}: missing checkpoints array`);
  return parsed;
}

function compareArray(label: string, expected: readonly number[], actual: readonly number[], epsilon: number): string | undefined {
  if (expected.length !== actual.length) return `${label}: length mismatch expected ${expected.length}, got ${actual.length}`;
  let maxDelta = 0;
  let maxIndex = 0;
  for (let i = 0; i < expected.length; i++) {
    const delta = Math.abs(expected[i]! - actual[i]!);
    if (delta > maxDelta) {
      maxDelta = delta;
      maxIndex = i;
    }
  }
  if (maxDelta > epsilon) {
    return `${label}: max delta ${maxDelta} at index ${maxIndex}, expected ${expected[maxIndex]}, got ${actual[maxIndex]}`;
  }
  return undefined;
}

function compareDumps(expected: DumpOutput, actual: DumpOutput, epsilon: number): string[] {
  const findings: string[] = [];
  if (expected.checkpoints.length !== actual.checkpoints.length) {
    findings.push(`checkpoint count mismatch expected ${expected.checkpoints.length}, got ${actual.checkpoints.length}`);
    return findings;
  }

  for (let ci = 0; ci < expected.checkpoints.length; ci++) {
    const eCheckpoint = expected.checkpoints[ci]!;
    const aCheckpoint = actual.checkpoints[ci]!;
    const frameLabel = `checkpoint[${ci}] frame ${eCheckpoint.frame}`;
    if (eCheckpoint.frame !== aCheckpoint.frame) {
      findings.push(`checkpoint[${ci}] frame mismatch expected ${eCheckpoint.frame}, got ${aCheckpoint.frame}`);
      continue;
    }
    if (eCheckpoint.bodies.length !== aCheckpoint.bodies.length) {
      findings.push(`${frameLabel}: body count mismatch expected ${eCheckpoint.bodies.length}, got ${aCheckpoint.bodies.length}`);
      continue;
    }

    for (let bi = 0; bi < eCheckpoint.bodies.length; bi++) {
      const eBody = eCheckpoint.bodies[bi]!;
      const aBody = aCheckpoint.bodies[bi]!;
      const bodyLabel = `${frameLabel} body[${bi}]`;
      if (eBody.t !== aBody.t) findings.push(`${bodyLabel}: body type mismatch expected ${eBody.t}, got ${aBody.t}`);
      if (eBody.a !== aBody.a) findings.push(`${bodyLabel}: awake mismatch expected ${eBody.a}, got ${aBody.a}`);
      for (const key of ["p", "q", "v", "w"] as const) {
        const finding = compareArray(`${bodyLabel}.${key}`, eBody[key], aBody[key], epsilon);
        if (finding !== undefined) findings.push(finding);
      }
    }
  }

  return findings;
}

try {
  const options = parseOptions(process.argv.slice(2));
  if (options.help) {
    console.log(usage());
    process.exit(0);
  }
  if (options.expectedPath === undefined || options.actualPath === undefined) throw new Error(`${usage()}\n\nMissing input file`);
  const expected = await loadDump(options.expectedPath);
  const actual = await loadDump(options.actualPath);
  const findings = compareDumps(expected, actual, options.epsilon);
  if (findings.length > 0) {
    console.error(`Dump mismatch (${findings.length} findings):`);
    for (const finding of findings) console.error(`- ${finding}`);
    process.exit(1);
  }
  console.log("Dumps match");
} catch (error) {
  console.error(error instanceof Error ? error.message : error);
  process.exit(1);
}
