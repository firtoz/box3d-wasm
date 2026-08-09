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
