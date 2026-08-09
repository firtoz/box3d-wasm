import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { parseObjText, type ParsedObjMesh } from "./parse-obj";

let cached: ParsedObjMesh | null = null;

/** Parsed `building.obj` for dump + Node-side scene setup. */
export function getBuildingMeshData(): ParsedObjMesh {
  if (cached !== null) return cached;
  const here = dirname(fileURLToPath(import.meta.url));
  const objPath = join(here, "../../../../box3d/data/meshes/building.obj");
  const text = readFileSync(objPath, "utf8");
  cached = parseObjText(text);
  return cached;
}
