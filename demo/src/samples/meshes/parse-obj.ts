/** Minimal Wavefront OBJ parser (vertices + triangular faces). */

export interface ParsedObjMesh {
  vertices: number[];
  indices: number[];
}

export function parseObjText(text: string): ParsedObjMesh {
  const positions: number[] = [];
  const indices: number[] = [];

  for (const rawLine of text.split(/\r?\n/)) {
    const line = rawLine.trim();
    if (line.startsWith("v ")) {
      const parts = line.split(/\s+/);
      positions.push(Number(parts[1]), Number(parts[2]), Number(parts[3]));
    } else if (line.startsWith("f ")) {
      const parts = line.split(/\s+/).slice(1);
      const face: number[] = [];
      for (const part of parts) {
        const vertexIndex = Number(part.split("/")[0]) - 1;
        face.push(vertexIndex);
      }
      for (let i = 1; i + 1 < face.length; i++) {
        indices.push(face[0]!, face[i]!, face[i + 1]!);
      }
    }
  }

  return { vertices: positions, indices };
}

/** Match `LoadTempMesh` / `CreateMeshData` vertex scaling (`zUp` swaps to `{y,z,x}`). */
export function transformObjVertices(vertices: number[], scale: number, zUp: boolean): number[] {
  const out = new Array<number>(vertices.length);
  for (let i = 0; i + 2 < vertices.length; i += 3) {
    const x = scale * vertices[i]!;
    const y = scale * vertices[i + 1]!;
    const z = scale * vertices[i + 2]!;
    if (zUp) {
      out[i] = y;
      out[i + 1] = z;
      out[i + 2] = x;
    } else {
      out[i] = x;
      out[i + 1] = y;
      out[i + 2] = z;
    }
  }
  return out;
}
