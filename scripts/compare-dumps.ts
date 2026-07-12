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

interface DumpRayHit {
  h: number;
  f: number;
  p: Vec3;
  n: Vec3;
}

interface DumpRays {
  o: number;
  r: DumpRayHit[];
}

interface DumpCheckpoint {
  frame: number;
  bodies: DumpBody[];
  rays?: DumpRays;
}

interface DumpOutput {
  checkpoints: DumpCheckpoint[];
}

interface Options {
  expectedPath: string | undefined;
  actualPath: string | undefined;
  epsilon: number;
  summary: boolean;
  help: boolean;
}

interface FieldDelta {
  label: string;
  frame: number;
  body: number;
  field: string;
  index: number;
  expected: number;
  actual: number;
  absDelta: number;
  ulpDelta: number;
}

function usage(): string {
  return [
    "Usage: compare-dumps [options] <expected.json> <actual.json>",
    "Options:",
    "  --help              Show this help",
    "  --epsilon <number>  Numeric tolerance for vector fields (default: 1e-5)",
    "  --summary           Print first divergence and aggregate worst deltas",
  ].join("\n");
}

function parseOptions(argv: string[]): Options {
  const options: Options = { expectedPath: undefined, actualPath: undefined, epsilon: 1e-5, summary: false, help: false };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i]!;
    if (arg === "--help") {
      options.help = true;
    } else if (arg === "--summary") {
      options.summary = true;
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

function floatToBits(value: number): number {
  const buffer = new ArrayBuffer(4);
  new Float32Array(buffer)[0] = value;
  return new Uint32Array(buffer)[0]!;
}

function ulpDelta(expected: number, actual: number): number {
  if (!Number.isFinite(expected) || !Number.isFinite(actual)) return Number.POSITIVE_INFINITY;
  if (expected === actual) return 0;
  const a = floatToBits(expected);
  const b = floatToBits(actual);
  // Convert IEEE bits to a monotonic integer ordering so signed floats compare correctly.
  const orderedA = a & 0x80000000 ? ~(a - 1) >>> 0 : a | 0x80000000;
  const orderedB = b & 0x80000000 ? ~(b - 1) >>> 0 : b | 0x80000000;
  return Math.abs(orderedA - orderedB);
}

function compareArray(
  frame: number,
  body: number,
  field: string,
  expected: readonly number[],
  actual: readonly number[],
  epsilon: number,
): { finding?: string; deltas: FieldDelta[] } {
  const deltas: FieldDelta[] = [];
  if (expected.length !== actual.length) {
    return {
      finding: `checkpoint frame ${frame} body[${body}].${field}: length mismatch expected ${expected.length}, got ${actual.length}`,
      deltas,
    };
  }
  let maxDelta = 0;
  let maxIndex = 0;
  for (let i = 0; i < expected.length; i++) {
    const e = expected[i]!;
    const a = actual[i]!;
    const absDelta = Math.abs(e - a);
    deltas.push({
      label: `frame ${frame} body[${body}].${field}[${i}]`,
      frame,
      body,
      field,
      index: i,
      expected: e,
      actual: a,
      absDelta,
      ulpDelta: ulpDelta(e, a),
    });
    if (absDelta > maxDelta) {
      maxDelta = absDelta;
      maxIndex = i;
    }
  }
  if (maxDelta > epsilon) {
    const worst = deltas[maxIndex]!;
    return {
      finding: `checkpoint frame ${frame} body[${body}].${field}: max delta ${maxDelta} (${worst.ulpDelta} ulp) at index ${maxIndex}, expected ${expected[maxIndex]}, got ${actual[maxIndex]}`,
      deltas,
    };
  }
  return { deltas };
}

function compareScalar(
  frame: number,
  label: string,
  expected: number,
  actual: number,
  epsilon: number,
): { finding?: string; deltas: FieldDelta[] } {
  const absDelta = Math.abs(expected - actual);
  const deltas: FieldDelta[] = [{
    label: `frame ${frame} ${label}`,
    frame,
    body: -1,
    field: label,
    index: 0,
    expected,
    actual,
    absDelta,
    ulpDelta: ulpDelta(expected, actual),
  }];
  if (absDelta > epsilon) {
    return {
      finding: `checkpoint frame ${frame} ${label}: delta ${absDelta} (${deltas[0]!.ulpDelta} ulp), expected ${expected}, got ${actual}`,
      deltas,
    };
  }
  return { deltas };
}

function compareRays(
  frame: number,
  expected: DumpRays | undefined,
  actual: DumpRays | undefined,
  epsilon: number,
): { findings: string[]; deltas: FieldDelta[] } {
  const findings: string[] = [];
  const deltas: FieldDelta[] = [];
  if (expected === undefined && actual === undefined) return { findings, deltas };
  if (expected === undefined || actual === undefined) {
    findings.push(`checkpoint frame ${frame}: rays presence mismatch expected ${expected !== undefined}, got ${actual !== undefined}`);
    return { findings, deltas };
  }
  {
    const result = compareScalar(frame, "rays.o", expected.o, actual.o, epsilon);
    deltas.push(...result.deltas);
    if (result.finding !== undefined) findings.push(result.finding);
  }
  if (expected.r.length !== actual.r.length) {
    findings.push(`checkpoint frame ${frame}: ray count mismatch expected ${expected.r.length}, got ${actual.r.length}`);
    return { findings, deltas };
  }
  for (let i = 0; i < expected.r.length; i++) {
    const e = expected.r[i]!;
    const a = actual.r[i]!;
    if (e.h !== a.h) findings.push(`checkpoint frame ${frame} ray[${i}].h: expected ${e.h}, got ${a.h}`);
    {
      const result = compareScalar(frame, `ray[${i}].f`, e.f, a.f, epsilon);
      deltas.push(...result.deltas);
      if (result.finding !== undefined) findings.push(result.finding);
    }
    for (const key of ["p", "n"] as const) {
      const result = compareArray(frame, i, `ray.${key}`, e[key], a[key], epsilon);
      deltas.push(...result.deltas.map((d) => ({ ...d, field: `ray[${i}].${key}`, label: `frame ${frame} ray[${i}].${key}[${d.index}]` })));
      if (result.finding !== undefined) {
        findings.push(result.finding.replace(`body[${i}].ray.${key}`, `ray[${i}].${key}`));
      }
    }
  }
  return { findings, deltas };
}

function compareDumps(expected: DumpOutput, actual: DumpOutput, epsilon: number): { findings: string[]; deltas: FieldDelta[] } {
  const findings: string[] = [];
  const deltas: FieldDelta[] = [];
  if (expected.checkpoints.length !== actual.checkpoints.length) {
    findings.push(`checkpoint count mismatch expected ${expected.checkpoints.length}, got ${actual.checkpoints.length}`);
    return { findings, deltas };
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
        const result = compareArray(eCheckpoint.frame, bi, key, eBody[key], aBody[key], epsilon);
        deltas.push(...result.deltas);
        if (result.finding !== undefined) findings.push(result.finding);
      }
    }

    const rayCompare = compareRays(eCheckpoint.frame, eCheckpoint.rays, aCheckpoint.rays, epsilon);
    findings.push(...rayCompare.findings);
    deltas.push(...rayCompare.deltas);
  }

  return { findings, deltas };
}

function printSummary(findings: string[], deltas: FieldDelta[], epsilon: number): void {
  const over = deltas.filter((d) => d.absDelta > epsilon).sort((a, b) => b.absDelta - a.absDelta);
  if (over.length === 0) {
    console.log("Summary: no numeric field exceeded epsilon");
    return;
  }
  const firstByFrame = [...over].sort((a, b) => a.frame - b.frame || a.body - b.body || a.field.localeCompare(b.field) || a.index - b.index)[0]!;
  const worstAbs = over[0]!;
  const worstUlp = [...over].sort((a, b) => b.ulpDelta - a.ulpDelta)[0]!;
  const frames = [...new Set(over.map((d) => d.frame))].sort((a, b) => a - b);
  console.error("Summary:");
  console.error(`- first divergence: ${firstByFrame.label} abs=${firstByFrame.absDelta} ulp=${firstByFrame.ulpDelta} expected=${firstByFrame.expected} actual=${firstByFrame.actual}`);
  console.error(`- worst abs delta: ${worstAbs.label} abs=${worstAbs.absDelta} ulp=${worstAbs.ulpDelta}`);
  console.error(`- worst ulp delta: ${worstUlp.label} abs=${worstUlp.absDelta} ulp=${worstUlp.ulpDelta}`);
  console.error(`- divergent frames: ${frames.join(",")}`);
  console.error(`- findings: ${findings.length}; fields over epsilon: ${over.length}`);
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
  const { findings, deltas } = compareDumps(expected, actual, options.epsilon);
  if (findings.length > 0) {
    console.error(`Dump mismatch (${findings.length} findings):`);
    for (const finding of findings) console.error(`- ${finding}`);
    if (options.summary) printSummary(findings, deltas, options.epsilon);
    process.exit(1);
  }
  if (options.summary) printSummary(findings, deltas, options.epsilon);
  console.log("Dumps match");
} catch (error) {
  console.error(error instanceof Error ? error.message : error);
  process.exit(1);
}
