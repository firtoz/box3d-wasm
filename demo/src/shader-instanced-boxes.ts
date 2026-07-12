import * as THREE from "three";

export function hexToRgb(hex: number, out: Float32Array, offset: number): void {
  out[offset] = ((hex >> 16) & 0xff) / 255;
  out[offset + 1] = ((hex >> 8) & 0xff) / 255;
  out[offset + 2] = (hex & 0xff) / 255;
}

export type ShaderInstanceMeshOptions = {
  shadows?: boolean;
};

/** @deprecated Use ShaderInstanceMeshOptions */
export type ShaderBoxMeshOptions = ShaderInstanceMeshOptions;

export type ShaderInstanceMesh = {
  mesh: THREE.Mesh;
  positionArray: Float32Array;
  quaternionArray: Float32Array;
  colorArray: Float32Array;
  positionAttribute: THREE.InstancedBufferAttribute;
  quaternionAttribute: THREE.InstancedBufferAttribute;
  colorAttribute: THREE.InstancedBufferAttribute;
  dispose(): void;
};

/** @deprecated Use ShaderInstanceMesh */
export type ShaderBoxMesh = ShaderInstanceMesh;

export function createShaderBoxMesh(
  count: number,
  size: number | [number, number, number] = 1,
  options: ShaderInstanceMeshOptions = {},
): ShaderInstanceMesh {
  const dimensions = typeof size === "number" ? [size, size, size] as const : size;
  const baseGeometry = new THREE.BoxGeometry(...dimensions);
  const mesh = createShaderInstanceMesh(baseGeometry, count, options);
  baseGeometry.dispose();
  return mesh;
}

export function createShaderInstanceMesh(
  baseGeometry: THREE.BufferGeometry,
  count: number,
  options: ShaderInstanceMeshOptions = {},
): ShaderInstanceMesh {
  const shadows = options.shadows ?? false;
  const geometry = new THREE.InstancedBufferGeometry();
  if (baseGeometry.index !== null) geometry.index = baseGeometry.index.clone();
  geometry.setAttribute("position", baseGeometry.getAttribute("position").clone());
  geometry.setAttribute("normal", baseGeometry.getAttribute("normal").clone());
  geometry.instanceCount = count;

  const positionArray = new Float32Array(count * 3);
  const quaternionArray = new Float32Array(count * 4);
  const colorArray = new Float32Array(count * 3);
  const positionAttribute = new THREE.InstancedBufferAttribute(positionArray, 3);
  const quaternionAttribute = new THREE.InstancedBufferAttribute(quaternionArray, 4);
  const colorAttribute = new THREE.InstancedBufferAttribute(colorArray, 3);
  positionAttribute.setUsage(THREE.DynamicDrawUsage);
  quaternionAttribute.setUsage(THREE.DynamicDrawUsage);
  colorAttribute.setUsage(THREE.DynamicDrawUsage);
  geometry.setAttribute("instancePosition", positionAttribute);
  geometry.setAttribute("instanceQuaternion", quaternionAttribute);
  geometry.setAttribute("instanceColor", colorAttribute);

  const material = new THREE.ShaderMaterial({
    vertexShader: `
      attribute vec3 instancePosition;
      attribute vec4 instanceQuaternion;
      attribute vec3 instanceColor;
      varying vec3 vColor;
      varying vec3 vNormal;
      vec3 applyQuat(vec3 v, vec4 q) {
        return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
      }
      void main() {
        vColor = instanceColor;
        vNormal = normalize(normalMatrix * applyQuat(normal, instanceQuaternion));
        vec3 transformed = applyQuat(position, instanceQuaternion) + instancePosition;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(transformed, 1.0);
      }
    `,
    fragmentShader: `
      varying vec3 vColor;
      varying vec3 vNormal;
      void main() {
        vec3 lightDir = normalize(vec3(0.45, 0.8, 0.35));
        float diffuse = max(dot(normalize(vNormal), lightDir), 0.0);
        vec3 color = vColor * (0.35 + 0.65 * diffuse);
        gl_FragColor = vec4(color, 1.0);
      }
    `,
  });
  const mesh = new THREE.Mesh(geometry, material);
  mesh.castShadow = shadows;
  mesh.receiveShadow = shadows;
  // Instance transforms live in custom attributes; Three.js only bounds the base
  // unit box at the origin, so frustum culling drops the whole draw when orbiting.
  mesh.frustumCulled = false;

  return {
    mesh,
    positionArray,
    quaternionArray,
    colorArray,
    positionAttribute,
    quaternionAttribute,
    colorAttribute,
    dispose() {
      geometry.dispose();
      material.dispose();
    },
  };
}

export function bindSnapshotTransforms(
  shaderMesh: ShaderInstanceMesh,
  positions: Float32Array,
  rotations: Float32Array,
  count: number,
  bodyOffset = 0,
): void {
  const geometry = shaderMesh.mesh.geometry as THREE.InstancedBufferGeometry;
  const pStart = bodyOffset * 3;
  const qStart = bodyOffset * 4;
  const positionAttribute = new THREE.InstancedBufferAttribute(positions.subarray(pStart, pStart + count * 3), 3);
  const quaternionAttribute = new THREE.InstancedBufferAttribute(rotations.subarray(qStart, qStart + count * 4), 4);
  positionAttribute.setUsage(THREE.DynamicDrawUsage);
  quaternionAttribute.setUsage(THREE.DynamicDrawUsage);
  geometry.setAttribute("instancePosition", positionAttribute);
  geometry.setAttribute("instanceQuaternion", quaternionAttribute);
  shaderMesh.positionAttribute = positionAttribute;
  shaderMesh.quaternionAttribute = quaternionAttribute;
  shaderMesh.positionArray = positions.subarray(pStart, pStart + count * 3);
  shaderMesh.quaternionArray = rotations.subarray(qStart, qStart + count * 4);
}
