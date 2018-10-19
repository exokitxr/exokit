window.skin = (() => {

const scale = 1 / 18;
const offsetY = 22 + 13.5/2 - 8/2;
const rotationOrder = 'YXZ';

const SKIN_SHADER = {
  uniforms: {
    headRotation: {
      type: 'v4',
      value: new THREE.Vector4(),
    },
    leftArmRotation: {
      type: 'v4',
      value: new THREE.Vector4(),
    },
    rightArmRotation: {
      type: 'v4',
      value: new THREE.Vector4(),
    },
    theta: {
      type: 'f',
      value: 0,
    },
    headVisible: {
      type: 'f',
      value: 1,
    },
    hit: {
      type: 'f',
      value: 0,
    },
    map: {
      type: 't',
      value: null,
    },
  },
  vertexShader: [
    "uniform vec4 headRotation;",
    "uniform vec4 leftArmRotation;",
    "uniform vec4 rightArmRotation;",
    "uniform float theta;",
    "uniform float headVisible;",
    "attribute vec4 dh;",
    "attribute vec4 dl;",
    "attribute vec4 dr;",
    "attribute vec4 dy;",
    "varying vec2 vUv;",
`
vec3 applyQuaternion(vec3 vec, vec4 quat) {
return vec + 2.0 * cross( cross( vec, quat.xyz ) + quat.w * vec, quat.xyz );
}
`,
    "void main() {",
    "  float theta2 = theta * dy.w;",
    "  vec3 headPosition = dh.w > 0.0 ? (headVisible > 0.0 ?(applyQuaternion(position.xyz - dh.xyz, headRotation) + dh.xyz) : vec3(0.0)) : position.xyz;",
    "  vec3 limbPosition = vec3(headPosition.x, headPosition.y - dy.y + (dy.y*cos(theta2) - dy.z*sin(theta2)), headPosition.z + dy.z + (dy.z*cos(theta2) + dy.y*sin(theta2)));",
    "  vec3 leftArmPosition = dl.w > 0.0 ? applyQuaternion(limbPosition.xyz - dl.xyz, leftArmRotation) + dl.xyz : limbPosition.xyz;",
    "  vec3 rightArmPosition = dr.w > 0.0 ? applyQuaternion(leftArmPosition.xyz - dr.xyz, rightArmRotation) + dr.xyz : leftArmPosition.xyz;",
    "  gl_Position = projectionMatrix * modelViewMatrix * vec4(rightArmPosition, 1.0);",
    "  vUv = uv;",
    "}"
  ].join("\n"),
  fragmentShader: [
    "uniform float hit;",
    "uniform sampler2D map;",
    "varying vec2 vUv;",
    "void main() {",
    "  vec4 diffuseColor = texture2D(map, vUv);",
    "  if (diffuseColor.a < 0.5) {",
    "    discard;",
    "  }",
    "  if (hit > 0.5) {",
    "    diffuseColor.r += 0.3;",
    "  }",
    "  gl_FragColor = diffuseColor;",
    "}"
  ].join("\n")
};
const skinSize = new THREE.Vector3(1, 2, 1);

const headBox = (() => {
  const headTop = [
    new THREE.Vector2(0.125, 0.875),
    new THREE.Vector2(0.25, 0.875),
    new THREE.Vector2(0.25, 1),
    new THREE.Vector2(0.125, 1)
  ];
  const headBottom = [
    new THREE.Vector2(0.25, 0.875),
    new THREE.Vector2(0.375, 0.875),
    new THREE.Vector2(0.375, 1),
    new THREE.Vector2(0.25, 1)
  ];
  const headLeft = [
    new THREE.Vector2(0, 0.75),
    new THREE.Vector2(0.125, 0.75),
    new THREE.Vector2(0.125, 0.875),
    new THREE.Vector2(0, 0.875)
  ];
  const headFront = [
    new THREE.Vector2(0.125, 0.75),
    new THREE.Vector2(0.25, 0.75),
    new THREE.Vector2(0.25 ,0.875),
    new THREE.Vector2(0.125 ,0.875)
  ];
  const headRight = [
    new THREE.Vector2(0.25, 0.75),
    new THREE.Vector2(0.375, 0.75),
    new THREE.Vector2(0.375, 0.875),
    new THREE.Vector2(0.25, 0.875)
  ];
  const headBack = [
    new THREE.Vector2(0.375, 0.75),
    new THREE.Vector2(0.5, 0.75),
    new THREE.Vector2(0.5, 0.875),
    new THREE.Vector2(0.375, 0.875)
  ];

  const headBox = new THREE.BoxGeometry(8, 8, 8, 0, 0, 0);
  headBox.faceVertexUvs[0] = [];
  headBox.faceVertexUvs[0][0] = [headRight[3], headRight[0], headRight[2]];
  headBox.faceVertexUvs[0][1] = [headRight[0], headRight[1], headRight[2]];
  headBox.faceVertexUvs[0][2] = [headLeft[3], headLeft[0], headLeft[2]];
  headBox.faceVertexUvs[0][3] = [headLeft[0], headLeft[1], headLeft[2]];
  headBox.faceVertexUvs[0][4] = [headTop[3], headTop[0], headTop[2]];
  headBox.faceVertexUvs[0][5] = [headTop[0], headTop[1], headTop[2]];
  headBox.faceVertexUvs[0][6] = [headBottom[0], headBottom[3], headBottom[1]];
  headBox.faceVertexUvs[0][7] = [headBottom[3], headBottom[2], headBottom[1]];
  headBox.faceVertexUvs[0][8] = [headFront[3], headFront[0], headFront[2]];
  headBox.faceVertexUvs[0][9] = [headFront[0], headFront[1], headFront[2]];
  headBox.faceVertexUvs[0][10] = [headBack[3], headBack[0], headBack[2]];
  headBox.faceVertexUvs[0][11] = [headBack[0], headBack[1], headBack[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(headBox);
  const dhs = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      result[baseIndex + 0] = 0;
      result[baseIndex + 1] = offsetY;
      result[baseIndex + 2] = 0;
      result[baseIndex + 3] = 1;
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(dhs), 4);
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(0, offsetY, 0));
  return geometry;
})();

const bodyBox = (() => {
  const bodyTop = [
    new THREE.Vector2(0.3125, 0.6875),
    new THREE.Vector2(0.4375, 0.6875),
    new THREE.Vector2(0.4375, 0.75),
    new THREE.Vector2(0.3125, 0.75)
  ];
  const bodyBottom = [
    new THREE.Vector2(0.4375, 0.6875),
    new THREE.Vector2(0.5625, 0.6875),
    new THREE.Vector2(0.5625, 0.75),
    new THREE.Vector2(0.4375, 0.75)
  ];
  const bodyLeft = [
    new THREE.Vector2(0.25, 0.5),
    new THREE.Vector2(0.3125, 0.5),
    new THREE.Vector2(0.3125, 0.6875),
    new THREE.Vector2(0.25, 0.6875)
  ];
  const bodyFront = [
    new THREE.Vector2(0.3125, 0.5),
    new THREE.Vector2(0.4375, 0.5),
    new THREE.Vector2(0.4375, 0.6875),
    new THREE.Vector2(0.3125, 0.6875)
  ];
  const bodyRight = [
    new THREE.Vector2(0.4375, 0.5),
    new THREE.Vector2(0.5, 0.5),
    new THREE.Vector2(0.5, 0.6875),
    new THREE.Vector2(0.4375, 0.6875)
  ];
  const bodyBack = [
    new THREE.Vector2(0.5, 0.5),
    new THREE.Vector2(0.625, 0.5),
    new THREE.Vector2(0.625, 0.6875),
    new THREE.Vector2(0.5, 0.6875)
  ];
  const bodyBox = new THREE.BoxGeometry(8, 12, 4, 0, 0, 0);
  bodyBox.faceVertexUvs[0] = [];
  bodyBox.faceVertexUvs[0][0] = [bodyRight[3], bodyRight[0], bodyRight[2]];
  bodyBox.faceVertexUvs[0][1] = [bodyRight[0], bodyRight[1], bodyRight[2]];
  bodyBox.faceVertexUvs[0][2] = [bodyLeft[3], bodyLeft[0], bodyLeft[2]];
  bodyBox.faceVertexUvs[0][3] = [bodyLeft[0], bodyLeft[1], bodyLeft[2]];
  bodyBox.faceVertexUvs[0][4] = [bodyTop[3], bodyTop[0], bodyTop[2]];
  bodyBox.faceVertexUvs[0][5] = [bodyTop[0], bodyTop[1], bodyTop[2]];
  bodyBox.faceVertexUvs[0][6] = [bodyBottom[0], bodyBottom[3], bodyBottom[1]];
  bodyBox.faceVertexUvs[0][7] = [bodyBottom[3], bodyBottom[2], bodyBottom[1]];
  bodyBox.faceVertexUvs[0][8] = [bodyFront[3], bodyFront[0], bodyFront[2]];
  bodyBox.faceVertexUvs[0][9] = [bodyFront[0], bodyFront[1], bodyFront[2]];
  bodyBox.faceVertexUvs[0][10] = [bodyBack[3], bodyBack[0], bodyBack[2]];
  bodyBox.faceVertexUvs[0][11] = [bodyBack[0], bodyBack[1], bodyBack[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(bodyBox)
    .applyMatrix(new THREE.Matrix4().makeTranslation(0, -10 + offsetY, 0));
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  return geometry;
})();

const rightArmBox = (() => {
  const rightArmTop = [
    new THREE.Vector2(0.6875, 0.6875),
    new THREE.Vector2(0.75, 0.6875),
    new THREE.Vector2(0.75, 0.75),
    new THREE.Vector2(0.6875, 0.75),
  ];
  const rightArmBottom = [
    new THREE.Vector2(0.75, 0.6875),
    new THREE.Vector2(0.8125, 0.6875),
    new THREE.Vector2(0.8125, 0.75),
    new THREE.Vector2(0.75, 0.75)
  ];
  const rightArmLeft = [
    new THREE.Vector2(0.625, 0.5),
    new THREE.Vector2(0.6875, 0.5),
    new THREE.Vector2(0.6875, 0.6875),
    new THREE.Vector2(0.625, 0.6875)
  ];
  const rightArmFront = [
    new THREE.Vector2(0.8125, 0.5),
    new THREE.Vector2(0.875, 0.5),
    new THREE.Vector2(0.875, 0.6875),
    new THREE.Vector2(0.8125, 0.6875)
  ];
  const rightArmRight = [
    new THREE.Vector2(0.75, 0.5),
    new THREE.Vector2(0.8125, 0.5),
    new THREE.Vector2(0.8125, 0.6875),
    new THREE.Vector2(0.75, 0.6875)
  ];
  const rightArmBack = [
    new THREE.Vector2(0.6875, 0.5),
    new THREE.Vector2(0.75, 0.5),
    new THREE.Vector2(0.75, 0.6875),
    new THREE.Vector2(0.6875, 0.6875)
  ];

  const rightArmBox = new THREE.BoxGeometry(4, 12, 4, 0, 0, 0);
  for (let i = 0; i < rightArmBox.vertices.length; i++) {
    rightArmBox.vertices[i].y -= 12/2;
  }
  rightArmBox.faceVertexUvs[0] = [];
  rightArmBox.faceVertexUvs[0][0] = [rightArmRight[3], rightArmRight[0], rightArmRight[2]];
  rightArmBox.faceVertexUvs[0][1] = [rightArmRight[0], rightArmRight[1], rightArmRight[2]];
  rightArmBox.faceVertexUvs[0][2] = [rightArmLeft[3], rightArmLeft[0], rightArmLeft[2]];
  rightArmBox.faceVertexUvs[0][3] = [rightArmLeft[0], rightArmLeft[1], rightArmLeft[2]];
  rightArmBox.faceVertexUvs[0][4] = [rightArmTop[3], rightArmTop[0], rightArmTop[2]];
  rightArmBox.faceVertexUvs[0][5] = [rightArmTop[0], rightArmTop[1], rightArmTop[2]];
  rightArmBox.faceVertexUvs[0][6] = [rightArmBottom[0], rightArmBottom[3], rightArmBottom[1]];
  rightArmBox.faceVertexUvs[0][7] = [rightArmBottom[3], rightArmBottom[2], rightArmBottom[1]];
  rightArmBox.faceVertexUvs[0][8] = [rightArmFront[3], rightArmFront[0], rightArmFront[2]];
  rightArmBox.faceVertexUvs[0][9] = [rightArmFront[0], rightArmFront[1], rightArmFront[2]];
  rightArmBox.faceVertexUvs[0][10] = [rightArmBack[3], rightArmBack[0], rightArmBack[2]];
  rightArmBox.faceVertexUvs[0][11] = [rightArmBack[0], rightArmBack[1], rightArmBack[2]];

  const geometry = new THREE.BufferGeometry().fromGeometry(rightArmBox);
  const offset = new THREE.Vector3(-6, -10 + 12/2 + offsetY, 0);
  const drs = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      result[baseIndex + 0] = -offset.x;
      result[baseIndex + 1] = offset.y;
      result[baseIndex + 2] = offset.z;
      result[baseIndex + 3] = 1;
    }

    return result;
  })();
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = 1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(drs, 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(offset.x, offset.y, offset.z));
  return geometry;
})();

const leftArmBox = (() => {
  const leftArmTop = [
    new THREE.Vector2(0.5625, 0.1875),
    new THREE.Vector2(0.625, 0.1875),
    new THREE.Vector2(0.625, 0.25),
    new THREE.Vector2(0.5625, 0.25),
  ];
  const leftArmBottom = [
    new THREE.Vector2(0.625, 0.1875),
    new THREE.Vector2(0.6875, 0.1875),
    new THREE.Vector2(0.6875, 0.25),
    new THREE.Vector2(0.625, 0.25)
  ];
  const leftArmLeft = [
    new THREE.Vector2(0.5, 0),
    new THREE.Vector2(0.5625, 0),
    new THREE.Vector2(0.5625, 0.1875),
    new THREE.Vector2(0.5, 0.1875)
  ];
  const leftArmFront = [
    new THREE.Vector2(0.6875, 0),
    new THREE.Vector2(0.75, 0),
    new THREE.Vector2(0.75, 0.1875),
    new THREE.Vector2(0.6875, 0.1875)
  ];
  const leftArmRight = [
    new THREE.Vector2(0.625, 0),
    new THREE.Vector2(0.6875, 0),
    new THREE.Vector2(0.6875, 0.1875),
    new THREE.Vector2(0.625, 0.1875)
  ];
  const leftArmBack = [
    new THREE.Vector2(0.5625, 0),
    new THREE.Vector2(0.625, 0),
    new THREE.Vector2(0.625, 0.1875),
    new THREE.Vector2(0.5625, 0.1875)
  ];

  const leftArmBox = new THREE.BoxGeometry(4, 12, 4, 0, 0, 0);
  for (let i = 0; i < leftArmBox.vertices.length; i++) {
    leftArmBox.vertices[i].y -= 12/2;
  }
  leftArmBox.faceVertexUvs[0] = [];
  leftArmBox.faceVertexUvs[0][0] = [leftArmRight[3], leftArmRight[0], leftArmRight[2]];
  leftArmBox.faceVertexUvs[0][1] = [leftArmRight[0], leftArmRight[1], leftArmRight[2]];
  leftArmBox.faceVertexUvs[0][2] = [leftArmLeft[3], leftArmLeft[0], leftArmLeft[2]];
  leftArmBox.faceVertexUvs[0][3] = [leftArmLeft[0], leftArmLeft[1], leftArmLeft[2]];
  leftArmBox.faceVertexUvs[0][4] = [leftArmTop[3], leftArmTop[0], leftArmTop[2]];
  leftArmBox.faceVertexUvs[0][5] = [leftArmTop[0], leftArmTop[1], leftArmTop[2]];
  leftArmBox.faceVertexUvs[0][6] = [leftArmBottom[0], leftArmBottom[3], leftArmBottom[1]];
  leftArmBox.faceVertexUvs[0][7] = [leftArmBottom[3], leftArmBottom[2], leftArmBottom[1]];
  leftArmBox.faceVertexUvs[0][8] = [leftArmFront[3], leftArmFront[0], leftArmFront[2]];
  leftArmBox.faceVertexUvs[0][9] = [leftArmFront[0], leftArmFront[1], leftArmFront[2]];
  leftArmBox.faceVertexUvs[0][10] = [leftArmBack[3], leftArmBack[0], leftArmBack[2]];
  leftArmBox.faceVertexUvs[0][11] = [leftArmBack[0], leftArmBack[1], leftArmBack[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(leftArmBox);
  const offset = new THREE.Vector3(6, -10 + 12/2 + offsetY, 0);
  const dls = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      result[baseIndex + 0] = -offset.x; // because we rotate mesh Y by Math.PI
      result[baseIndex + 1] = offset.y;
      result[baseIndex + 2] = offset.z;
      result[baseIndex + 3] = 1;
    }

    return result;
  })();
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = -1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(dls, 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(offset.x, offset.y, offset.z));
  return geometry;
})();

const rightLegBox = (() => {
  const rightLegTop = [
    new THREE.Vector2(0.0625, 0.6875),
    new THREE.Vector2(0.125, 0.6875),
    new THREE.Vector2(0.125, 0.75),
    new THREE.Vector2(0.0625, 0.75),
  ];
  const rightLegBottom = [
    new THREE.Vector2(0.125, 0.6875),
    new THREE.Vector2(0.1875, 0.6875),
    new THREE.Vector2(0.1875, 0.75),
    new THREE.Vector2(0.125, 0.75)
  ];
  const rightLegLeft = [
    new THREE.Vector2(0.0625, 0.5),
    new THREE.Vector2(0, 0.5),
    new THREE.Vector2(0, 0.6875),
    new THREE.Vector2(0.0625, 0.6875)
  ];
  const rightLegFront = [
    new THREE.Vector2(0.1875, 0.5),
    new THREE.Vector2(0.25, 0.5),
    new THREE.Vector2(0.25, 0.6875),
    new THREE.Vector2(0.1875, 0.6875)
  ];
  const rightLegRight = [
    new THREE.Vector2(0.125, 0.5),
    new THREE.Vector2(0.1875, 0.5),
    new THREE.Vector2(0.1875, 0.6875),
    new THREE.Vector2(0.125, 0.6875)
  ];
  const rightLegBack = [
    new THREE.Vector2(0.125, 0.5),
    new THREE.Vector2(0.0625, 0.5),
    new THREE.Vector2(0.0625, 0.6875),
    new THREE.Vector2(0.125, 0.6875)
  ];
  const rightLegBox = new THREE.BoxGeometry(4, 12, 4, 0, 0, 0);
  for (let i = 0; i < rightLegBox.vertices.length; i++) {
    rightLegBox.vertices[i].y -= 12/2;
  }
  rightLegBox.faceVertexUvs[0] = [];
  rightLegBox.faceVertexUvs[0][0] = [rightLegRight[3], rightLegRight[0], rightLegRight[2]];
  rightLegBox.faceVertexUvs[0][1] = [rightLegRight[0], rightLegRight[1], rightLegRight[2]];
  rightLegBox.faceVertexUvs[0][2] = [rightLegLeft[3], rightLegLeft[0], rightLegLeft[2]];
  rightLegBox.faceVertexUvs[0][3] = [rightLegLeft[0], rightLegLeft[1], rightLegLeft[2]];
  rightLegBox.faceVertexUvs[0][4] = [rightLegTop[3], rightLegTop[0], rightLegTop[2]];
  rightLegBox.faceVertexUvs[0][5] = [rightLegTop[0], rightLegTop[1], rightLegTop[2]];
  rightLegBox.faceVertexUvs[0][6] = [rightLegBottom[0], rightLegBottom[3], rightLegBottom[1]];
  rightLegBox.faceVertexUvs[0][7] = [rightLegBottom[3], rightLegBottom[2], rightLegBottom[1]];
  rightLegBox.faceVertexUvs[0][8] = [rightLegFront[3], rightLegFront[0], rightLegFront[2]];
  rightLegBox.faceVertexUvs[0][9] = [rightLegFront[0], rightLegFront[1], rightLegFront[2]];
  rightLegBox.faceVertexUvs[0][10] = [rightLegBack[3], rightLegBack[0], rightLegBack[2]];
  rightLegBox.faceVertexUvs[0][11] = [rightLegBack[0], rightLegBack[1], rightLegBack[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(rightLegBox);
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = 1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(-2, -22 + 12/2 + offsetY, 0));
  return geometry;
})();

const leftLegBox = (() => {
  const leftLegTop = [
    new THREE.Vector2(0.3125, 0.1875),
    new THREE.Vector2(0.375, 0.1875),
    new THREE.Vector2(0.375, 0.25),
    new THREE.Vector2(0.3125, 0.25),
  ];
  const leftLegBottom = [
    new THREE.Vector2(0.375, 0.1875),
    new THREE.Vector2(0.4375, 0.1875),
    new THREE.Vector2(0.4375, 0.25),
    new THREE.Vector2(0.375, 0.25)
  ];
  const leftLegLeft = [
    new THREE.Vector2(0.25, 0),
    new THREE.Vector2(0.3125, 0),
    new THREE.Vector2(0.3125, 0.1875),
    new THREE.Vector2(0.25, 0.1875)
  ];
  const leftLegFront = [
    new THREE.Vector2(0.4375, 0),
    new THREE.Vector2(0.5, 0),
    new THREE.Vector2(0.5, 0.1875),
    new THREE.Vector2(0.4375, 0.1875)
  ];
  const leftLegRight = [
    new THREE.Vector2(0.4375, 0),
    new THREE.Vector2(0.375, 0),
    new THREE.Vector2(0.375, 0.1875),
    new THREE.Vector2(0.4375, 0.1875)
  ];
  const leftLegBack = [
    new THREE.Vector2(0.375, 0),
    new THREE.Vector2(0.3125, 0),
    new THREE.Vector2(0.3125, 0.1875),
    new THREE.Vector2(0.375, 0.1875)
  ];
  const leftLegBox = new THREE.BoxGeometry(4, 12, 4, 0, 0, 0);
  for (let i = 0; i < leftLegBox.vertices.length; i++) {
    leftLegBox.vertices[i].y -= 12/2;
  }
  leftLegBox.faceVertexUvs[0] = [];
  leftLegBox.faceVertexUvs[0][0] = [leftLegRight[3], leftLegRight[0], leftLegRight[2]];
  leftLegBox.faceVertexUvs[0][1] = [leftLegRight[0], leftLegRight[1], leftLegRight[2]];
  leftLegBox.faceVertexUvs[0][2] = [leftLegLeft[3], leftLegLeft[0], leftLegLeft[2]];
  leftLegBox.faceVertexUvs[0][3] = [leftLegLeft[0], leftLegLeft[1], leftLegLeft[2]];
  leftLegBox.faceVertexUvs[0][4] = [leftLegTop[3], leftLegTop[0], leftLegTop[2]];
  leftLegBox.faceVertexUvs[0][5] = [leftLegTop[0], leftLegTop[1], leftLegTop[2]];
  leftLegBox.faceVertexUvs[0][6] = [leftLegBottom[0], leftLegBottom[3], leftLegBottom[1]];
  leftLegBox.faceVertexUvs[0][7] = [leftLegBottom[3], leftLegBottom[2], leftLegBottom[1]];
  leftLegBox.faceVertexUvs[0][8] = [leftLegFront[3], leftLegFront[0], leftLegFront[2]];
  leftLegBox.faceVertexUvs[0][9] = [leftLegFront[0], leftLegFront[1], leftLegFront[2]];
  leftLegBox.faceVertexUvs[0][10] = [leftLegBack[3], leftLegBack[0], leftLegBack[2]];
  leftLegBox.faceVertexUvs[0][11] = [leftLegBack[0], leftLegBack[1], leftLegBack[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(leftLegBox);
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = -1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(2, -22 + 12/2 + offsetY, 0));
  return geometry;
})();

const head2Box = (() => {
  const head2Top = [
    new THREE.Vector2(0.625, 0.875),
    new THREE.Vector2(0.75, 0.875),
    new THREE.Vector2(0.75, 1),
    new THREE.Vector2(0.625, 1)
  ];
  const head2Bottom = [
    new THREE.Vector2(0.75, 0.875),
    new THREE.Vector2(0.875, 0.875),
    new THREE.Vector2(0.875, 1),
    new THREE.Vector2(0.75, 1)
  ];
  const head2Left = [
    new THREE.Vector2(0.5, 0.75),
    new THREE.Vector2(0.625, 0.75),
    new THREE.Vector2(0.625, 0.875),
    new THREE.Vector2(0.5, 0.875)
  ];
  const head2Front = [
    new THREE.Vector2(0.625, 0.75),
    new THREE.Vector2(0.75, 0.75),
    new THREE.Vector2(0.75, 0.875),
    new THREE.Vector2(0.625, 0.875)
  ];
  const head2Right = [
    new THREE.Vector2(0.75, 0.75),
    new THREE.Vector2(0.875, 0.75),
    new THREE.Vector2(0.875, 0.875),
    new THREE.Vector2(0.75, 0.875)
  ];
  const head2Back = [
    new THREE.Vector2(0.875, 0.75),
    new THREE.Vector2(1, 0.75),
    new THREE.Vector2(1, 0.875),
    new THREE.Vector2(0.875, 0.875)
  ];
  const head2Box = new THREE.BoxGeometry(9, 9, 9, 0, 0, 0);
  head2Box.faceVertexUvs[0] = [];
  head2Box.faceVertexUvs[0][0] = [head2Right[3], head2Right[0], head2Right[2]];
  head2Box.faceVertexUvs[0][1] = [head2Right[0], head2Right[1], head2Right[2]];
  head2Box.faceVertexUvs[0][2] = [head2Left[3], head2Left[0], head2Left[2]];
  head2Box.faceVertexUvs[0][3] = [head2Left[0], head2Left[1], head2Left[2]];
  head2Box.faceVertexUvs[0][4] = [head2Top[3], head2Top[0], head2Top[2]];
  head2Box.faceVertexUvs[0][5] = [head2Top[0], head2Top[1], head2Top[2]];
  head2Box.faceVertexUvs[0][6] = [head2Bottom[0], head2Bottom[3], head2Bottom[1]];
  head2Box.faceVertexUvs[0][7] = [head2Bottom[3], head2Bottom[2], head2Bottom[1]];
  head2Box.faceVertexUvs[0][8] = [head2Front[3], head2Front[0], head2Front[2]];
  head2Box.faceVertexUvs[0][9] = [head2Front[0], head2Front[1], head2Front[2]];
  head2Box.faceVertexUvs[0][10] = [head2Back[3], head2Back[0], head2Back[2]];
  head2Box.faceVertexUvs[0][11] = [head2Back[0], head2Back[1], head2Back[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(head2Box);
  const dhs = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      result[baseIndex + 0] = 0;
      result[baseIndex + 1] = offsetY;
      result[baseIndex + 2] = 0;
      result[baseIndex + 3] = 1;
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(dhs), 4);
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(0, offsetY, 0));
  return geometry;
})();

const body2Box = (() => {
  const body2Top = [
    new THREE.Vector2(0.3125, 0.4375),
    new THREE.Vector2(0.4375, 0.4375),
    new THREE.Vector2(0.4375, 0.5),
    new THREE.Vector2(0.3125, 0.5)
  ];
  const body2Bottom = [
    new THREE.Vector2(0.4375, 0.4375),
    new THREE.Vector2(0.5625, 0.4375),
    new THREE.Vector2(0.5625, 0.5),
    new THREE.Vector2(0.4375, 0.5)
  ];
  const body2Left = [
    new THREE.Vector2(0.25, 0.25),
    new THREE.Vector2(0.3125, 0.25),
    new THREE.Vector2(0.3125, 0.4375),
    new THREE.Vector2(0.25, 0.4375)
  ];
  const body2Front = [
    new THREE.Vector2(0.3125, 0.25),
    new THREE.Vector2(0.4375, 0.25),
    new THREE.Vector2(0.4375, 0.4375),
    new THREE.Vector2(0.3125, 0.4375)
  ];
  const body2Right = [
    new THREE.Vector2(0.4375, 0.25),
    new THREE.Vector2(0.5, 0.25),
    new THREE.Vector2(0.5, 0.4375),
    new THREE.Vector2(0.4375, 0.4375)
  ];
  const body2Back = [
    new THREE.Vector2(0.5, 0.25),
    new THREE.Vector2(0.625, 0.25),
    new THREE.Vector2(0.625, 0.4375),
    new THREE.Vector2(0.5, 0.4375)
  ];
  const body2Box = new THREE.BoxGeometry(9, 13.5, 5, 0, 0, 0);
  body2Box.faceVertexUvs[0] = [];
  body2Box.faceVertexUvs[0][0] = [body2Right[3], body2Right[0], body2Right[2]];
  body2Box.faceVertexUvs[0][1] = [body2Right[0], body2Right[1], body2Right[2]];
  body2Box.faceVertexUvs[0][2] = [body2Left[3], body2Left[0], body2Left[2]];
  body2Box.faceVertexUvs[0][3] = [body2Left[0], body2Left[1], body2Left[2]];
  body2Box.faceVertexUvs[0][4] = [body2Top[3], body2Top[0], body2Top[2]];
  body2Box.faceVertexUvs[0][5] = [body2Top[0], body2Top[1], body2Top[2]];
  body2Box.faceVertexUvs[0][6] = [body2Bottom[0], body2Bottom[3], body2Bottom[1]];
  body2Box.faceVertexUvs[0][7] = [body2Bottom[3], body2Bottom[2], body2Bottom[1]];
  body2Box.faceVertexUvs[0][8] = [body2Front[3], body2Front[0], body2Front[2]];
  body2Box.faceVertexUvs[0][9] = [body2Front[0], body2Front[1], body2Front[2]];
  body2Box.faceVertexUvs[0][10] = [body2Back[3], body2Back[0], body2Back[2]];
  body2Box.faceVertexUvs[0][11] = [body2Back[0], body2Back[1], body2Back[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(body2Box)
    .applyMatrix(new THREE.Matrix4().makeTranslation(0, -10 + offsetY, 0));
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  return geometry;
})();

const rightArm2Box = (() => {
  const rightArm2Top = [
    new THREE.Vector2(0.6875, 0.4375),
    new THREE.Vector2(0.75, 0.4375),
    new THREE.Vector2(0.75, 0.5),
    new THREE.Vector2(0.6875, 0.5),
  ];
  const rightArm2Bottom = [
    new THREE.Vector2(0.75, 0.4375),
    new THREE.Vector2(0.8125, 0.4375),
    new THREE.Vector2(0.8125, 0.5),
    new THREE.Vector2(0.75, 0.5)
  ];
  const rightArm2Left = [
    new THREE.Vector2(0.625, 0.25),
    new THREE.Vector2(0.6875, 0.25),
    new THREE.Vector2(0.6875, 0.4375),
    new THREE.Vector2(0.625, 0.4375)
  ];
  const rightArm2Front = [
    new THREE.Vector2(0.8125, 0.25),
    new THREE.Vector2(0.875, 0.25),
    new THREE.Vector2(0.875, 0.4375),
    new THREE.Vector2(0.8125, 0.4375)
  ];
  const rightArm2Right = [
    new THREE.Vector2(0.75, 0.25),
    new THREE.Vector2(0.8125, 0.25),
    new THREE.Vector2(0.8125, 0.4375),
    new THREE.Vector2(0.75, 0.4375)
  ];
  const rightArm2Back = [
    new THREE.Vector2(0.6875, 0.25),
    new THREE.Vector2(0.75, 0.25),
    new THREE.Vector2(0.75, 0.4375),
    new THREE.Vector2(0.6875, 0.4375)
  ];

  const rightArm2Box = new THREE.BoxGeometry(4.5, 13.5, 4.5, 0, 0, 0);
  rightArm2Box.faceVertexUvs[0] = [];
  rightArm2Box.faceVertexUvs[0][0] = [rightArm2Right[3], rightArm2Right[0], rightArm2Right[2]];
  rightArm2Box.faceVertexUvs[0][1] = [rightArm2Right[0], rightArm2Right[1], rightArm2Right[2]];
  rightArm2Box.faceVertexUvs[0][2] = [rightArm2Left[3], rightArm2Left[0], rightArm2Left[2]];
  rightArm2Box.faceVertexUvs[0][3] = [rightArm2Left[0], rightArm2Left[1], rightArm2Left[2]];
  rightArm2Box.faceVertexUvs[0][4] = [rightArm2Top[3], rightArm2Top[0], rightArm2Top[2]];
  rightArm2Box.faceVertexUvs[0][5] = [rightArm2Top[0], rightArm2Top[1], rightArm2Top[2]];
  rightArm2Box.faceVertexUvs[0][6] = [rightArm2Bottom[0], rightArm2Bottom[3], rightArm2Bottom[1]];
  rightArm2Box.faceVertexUvs[0][7] = [rightArm2Bottom[3], rightArm2Bottom[2], rightArm2Bottom[1]];
  rightArm2Box.faceVertexUvs[0][8] = [rightArm2Front[3], rightArm2Front[0], rightArm2Front[2]];
  rightArm2Box.faceVertexUvs[0][9] = [rightArm2Front[0], rightArm2Front[1], rightArm2Front[2]];
  rightArm2Box.faceVertexUvs[0][10] = [rightArm2Back[3], rightArm2Back[0], rightArm2Back[2]];
  rightArm2Box.faceVertexUvs[0][11] = [rightArm2Back[0], rightArm2Back[1], rightArm2Back[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(rightArm2Box)
    .applyMatrix(new THREE.Matrix4().makeTranslation(0, -12/2, 0));
  const offset = new THREE.Vector3(-6, -10 + offsetY + 12/2, 0);
  const drs = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      result[baseIndex + 0] = -offset.x;
      result[baseIndex + 1] = offset.y;
      result[baseIndex + 2] = offset.z;
      result[baseIndex + 3] = 1;
    }

    return result;
  })();
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = 1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(drs, 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(offset.x, offset.y, offset.z));
  return geometry;
})();

const leftArm2Box = (() => {
  const leftArm2Top = [
    new THREE.Vector2(0.8125, 0.1875),
    new THREE.Vector2(0.875, 0.1875),
    new THREE.Vector2(0.875, 0.25),
    new THREE.Vector2(0.8125, 0.25),
  ];
  const leftArm2Bottom = [
    new THREE.Vector2(0.875, 0.1875),
    new THREE.Vector2(0.9375, 0.1875),
    new THREE.Vector2(0.9375, 0.25),
    new THREE.Vector2(0.875, 0.25)
  ];
  const leftArm2Left = [
    new THREE.Vector2(0.75, 0),
    new THREE.Vector2(0.8125, 0),
    new THREE.Vector2(0.8125, 0.1875),
    new THREE.Vector2(0.75, 0.1875)
  ];
  const leftArm2Front = [
    new THREE.Vector2(0.9375, 0),
    new THREE.Vector2(1, 0),
    new THREE.Vector2(1, 0.1875),
    new THREE.Vector2(0.9375, 0.1875)
  ];
  const leftArm2Right = [
    new THREE.Vector2(0.875, 0),
    new THREE.Vector2(0.9375, 0),
    new THREE.Vector2(0.9375, 0.1875),
    new THREE.Vector2(0.875, 0.1875)
  ];
  const leftArm2Back = [
    new THREE.Vector2(0.8125, 0),
    new THREE.Vector2(0.875, 0),
    new THREE.Vector2(0.875, 0.1875),
    new THREE.Vector2(0.8125, 0.1875)
  ];

  const leftArm2Box = new THREE.BoxGeometry(4.5, 13.5, 4.5, 0, 0, 0);
  leftArm2Box.faceVertexUvs[0] = [];
  leftArm2Box.faceVertexUvs[0][0] = [leftArm2Right[3], leftArm2Right[0], leftArm2Right[2]];
  leftArm2Box.faceVertexUvs[0][1] = [leftArm2Right[0], leftArm2Right[1], leftArm2Right[2]];
  leftArm2Box.faceVertexUvs[0][2] = [leftArm2Left[3], leftArm2Left[0], leftArm2Left[2]];
  leftArm2Box.faceVertexUvs[0][3] = [leftArm2Left[0], leftArm2Left[1], leftArm2Left[2]];
  leftArm2Box.faceVertexUvs[0][4] = [leftArm2Top[3], leftArm2Top[0], leftArm2Top[2]];
  leftArm2Box.faceVertexUvs[0][5] = [leftArm2Top[0], leftArm2Top[1], leftArm2Top[2]];
  leftArm2Box.faceVertexUvs[0][6] = [leftArm2Bottom[0], leftArm2Bottom[3], leftArm2Bottom[1]];
  leftArm2Box.faceVertexUvs[0][7] = [leftArm2Bottom[3], leftArm2Bottom[2], leftArm2Bottom[1]];
  leftArm2Box.faceVertexUvs[0][8] = [leftArm2Front[3], leftArm2Front[0], leftArm2Front[2]];
  leftArm2Box.faceVertexUvs[0][9] = [leftArm2Front[0], leftArm2Front[1], leftArm2Front[2]];
  leftArm2Box.faceVertexUvs[0][10] = [leftArm2Back[3], leftArm2Back[0], leftArm2Back[2]];
  leftArm2Box.faceVertexUvs[0][11] = [leftArm2Back[0], leftArm2Back[1], leftArm2Back[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(leftArm2Box)
    .applyMatrix(new THREE.Matrix4().makeTranslation(0, -12/2, 0));
  const offset = new THREE.Vector3(6, -10 + offsetY + 12/2, 0);
  const dls = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      result[baseIndex + 0] = -offset.x; // because we rotate mesh Y by Math.PI
      result[baseIndex + 1] = offset.y;
      result[baseIndex + 2] = offset.z;
      result[baseIndex + 3] = 1;
    }

    return result;
  })();
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = -1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(dls, 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(offset.x, offset.y, offset.z));
  return geometry;
})();

const rightLeg2Box = (() => {
  const rightLeg2Top = [
    new THREE.Vector2(0.0625, 0.4375),
    new THREE.Vector2(0.125, 0.4375),
    new THREE.Vector2(0.125, 0.5),
    new THREE.Vector2(0.0625, 0.5),
  ];
  const rightLeg2Bottom = [
    new THREE.Vector2(0.125, 0.4375),
    new THREE.Vector2(0.1875, 0.4375),
    new THREE.Vector2(0.1875, 0.5),
    new THREE.Vector2(0.125, 0.5)
  ];
  const rightLeg2Left = [
    new THREE.Vector2(0.0625, 0.25),
    new THREE.Vector2(0, 0.25),
    new THREE.Vector2(0, 0.4375),
    new THREE.Vector2(0.0625, 0.4375)
  ];
  const rightLeg2Front = [
    new THREE.Vector2(0.1875, 0.25),
    new THREE.Vector2(0.25, 0.25),
    new THREE.Vector2(0.25, 0.4375),
    new THREE.Vector2(0.1875, 0.4375)
  ];
  const rightLeg2Right = [
    new THREE.Vector2(0.125, 0.25),
    new THREE.Vector2(0.1875, 0.25),
    new THREE.Vector2(0.1875, 0.4375),
    new THREE.Vector2(0.125, 0.4375)
  ];
  const rightLeg2Back = [
    new THREE.Vector2(0.125, 0.25),
    new THREE.Vector2(0.0625, 0.25),
    new THREE.Vector2(0.0625, 0.4375),
    new THREE.Vector2(0.125, 0.4375)
  ];

  const rightLeg2Box = new THREE.BoxGeometry(4.5, 12, 4.5, 0, 0, 0);
  for (let i = 0; i < rightLeg2Box.vertices.length; i++) {
    rightLeg2Box.vertices[i].y -= 12/2;
  }
  rightLeg2Box.faceVertexUvs[0] = [];
  rightLeg2Box.faceVertexUvs[0][0] = [rightLeg2Right[3], rightLeg2Right[0], rightLeg2Right[2]];
  rightLeg2Box.faceVertexUvs[0][1] = [rightLeg2Right[0], rightLeg2Right[1], rightLeg2Right[2]];
  rightLeg2Box.faceVertexUvs[0][2] = [rightLeg2Left[3], rightLeg2Left[0], rightLeg2Left[2]];
  rightLeg2Box.faceVertexUvs[0][3] = [rightLeg2Left[0], rightLeg2Left[1], rightLeg2Left[2]];
  rightLeg2Box.faceVertexUvs[0][4] = [rightLeg2Top[3], rightLeg2Top[0], rightLeg2Top[2]];
  rightLeg2Box.faceVertexUvs[0][5] = [rightLeg2Top[0], rightLeg2Top[1], rightLeg2Top[2]];
  rightLeg2Box.faceVertexUvs[0][6] = [rightLeg2Bottom[0], rightLeg2Bottom[3], rightLeg2Bottom[1]];
  rightLeg2Box.faceVertexUvs[0][7] = [rightLeg2Bottom[3], rightLeg2Bottom[2], rightLeg2Bottom[1]];
  rightLeg2Box.faceVertexUvs[0][8] = [rightLeg2Front[3], rightLeg2Front[0], rightLeg2Front[2]];
  rightLeg2Box.faceVertexUvs[0][9] = [rightLeg2Front[0], rightLeg2Front[1], rightLeg2Front[2]];
  rightLeg2Box.faceVertexUvs[0][10] = [rightLeg2Back[3], rightLeg2Back[0], rightLeg2Back[2]];
  rightLeg2Box.faceVertexUvs[0][11] = [rightLeg2Back[0], rightLeg2Back[1], rightLeg2Back[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(rightLeg2Box);
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = 1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(-2, -22 + offsetY + 12/2, 0));
  return geometry;
})();

const leftLeg2Box = (() => {
  const leftLeg2Top = [
    new THREE.Vector2(0.0625, 0.1875),
    new THREE.Vector2(0.125, 0.1875),
    new THREE.Vector2(0.125, 0.25),
    new THREE.Vector2(0.0625, 0.25),
  ];
  const leftLeg2Bottom = [
    new THREE.Vector2(0.125, 0.1875),
    new THREE.Vector2(0.1875, 0.1875),
    new THREE.Vector2(0.1875, 0.25),
    new THREE.Vector2(0.125, 0.25)
  ];
  const leftLeg2Left = [
    new THREE.Vector2(0, 0),
    new THREE.Vector2(0.0625, 0),
    new THREE.Vector2(0.0625, 0.1875),
    new THREE.Vector2(0, 0.1875)
  ];
  const leftLeg2Front = [
    new THREE.Vector2(0.1875, 0),
    new THREE.Vector2(0.25, 0),
    new THREE.Vector2(0.25, 0.1875),
    new THREE.Vector2(0.1875, 0.1875)
  ];
  const leftLeg2Right = [
    new THREE.Vector2(0.1875, 0),
    new THREE.Vector2(0.125, 0),
    new THREE.Vector2(0.125, 0.1875),
    new THREE.Vector2(0.1875, 0.1875)
  ];
  const leftLeg2Back = [
    new THREE.Vector2(0.125, 0),
    new THREE.Vector2(0.0625, 0),
    new THREE.Vector2(0.0625, 0.1875),
    new THREE.Vector2(0.125, 0.1875)
  ];

  const leftLeg2Box = new THREE.BoxGeometry(4.5, 12, 4.5, 0, 0, 0);
  for (let i = 0; i < leftLeg2Box.vertices.length; i++) {
    leftLeg2Box.vertices[i].y -= 12/2;
  }
  leftLeg2Box.faceVertexUvs[0] = [];
  leftLeg2Box.faceVertexUvs[0][0] = [leftLeg2Right[3], leftLeg2Right[0], leftLeg2Right[2]];
  leftLeg2Box.faceVertexUvs[0][1] = [leftLeg2Right[0], leftLeg2Right[1], leftLeg2Right[2]];
  leftLeg2Box.faceVertexUvs[0][2] = [leftLeg2Left[3], leftLeg2Left[0], leftLeg2Left[2]];
  leftLeg2Box.faceVertexUvs[0][3] = [leftLeg2Left[0], leftLeg2Left[1], leftLeg2Left[2]];
  leftLeg2Box.faceVertexUvs[0][4] = [leftLeg2Top[3], leftLeg2Top[0], leftLeg2Top[2]];
  leftLeg2Box.faceVertexUvs[0][5] = [leftLeg2Top[0], leftLeg2Top[1], leftLeg2Top[2]];
  leftLeg2Box.faceVertexUvs[0][6] = [leftLeg2Bottom[0], leftLeg2Bottom[3], leftLeg2Bottom[1]];
  leftLeg2Box.faceVertexUvs[0][7] = [leftLeg2Bottom[3], leftLeg2Bottom[2], leftLeg2Bottom[1]];
  leftLeg2Box.faceVertexUvs[0][8] = [leftLeg2Front[3], leftLeg2Front[0], leftLeg2Front[2]];
  leftLeg2Box.faceVertexUvs[0][9] = [leftLeg2Front[0], leftLeg2Front[1], leftLeg2Front[2]];
  leftLeg2Box.faceVertexUvs[0][10] = [leftLeg2Back[3], leftLeg2Back[0], leftLeg2Back[2]];
  leftLeg2Box.faceVertexUvs[0][11] = [leftLeg2Back[0], leftLeg2Back[1], leftLeg2Back[2]];
  const geometry = new THREE.BufferGeometry().fromGeometry(leftLeg2Box);
  const dys = (() => {
    const positions = geometry.getAttribute('position').array;
    const numPositions = positions.length / 3;
    const result = new Float32Array(numPositions * 4);

    for (let i = 0; i < numPositions; i++) {
      const baseIndex = i * 4;
      const basePositionIndex = i * 3;
      result[baseIndex + 0] = positions[basePositionIndex + 0];
      result[baseIndex + 1] = positions[basePositionIndex + 1];
      result[baseIndex + 2] = positions[basePositionIndex + 2];
      result[baseIndex + 3] = -1; // angle factor
    }

    return result;
  })();
  geometry.addAttribute('dh', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(new Float32Array(geometry.getAttribute('position').array.length / 3 * 4), 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeTranslation(2, -22 + offsetY + 12/2, 0));
  return geometry;
})();

const skinGeometry = (() => {
  const geometries = [
    headBox,
    bodyBox,
    rightArmBox,
    leftArmBox,
    rightLegBox,
    leftLegBox,
    head2Box,
    body2Box,
    rightArm2Box,
    leftArm2Box,
    rightLeg2Box,
    leftLeg2Box,
  ];

  const positions = new Float32Array(geometries[0].getAttribute('position').array.length * geometries.length);
  const uvs = new Float32Array(geometries[0].getAttribute('uv').array.length * geometries.length);
  const dhs = new Float32Array(geometries[0].getAttribute('dh').array.length * geometries.length);
  const dls = new Float32Array(geometries[0].getAttribute('dl').array.length * geometries.length);
  const drs = new Float32Array(geometries[0].getAttribute('dr').array.length * geometries.length);
  const dys = new Float32Array(geometries[0].getAttribute('dy').array.length * geometries.length);
  let positionIndex = 0;
  let dhIndex = 0;
  let uvIndex = 0;

  for (let i = 0; i < geometries.length; i++) {
    const newGeometry = geometries[i];
    const newPositions = newGeometry.getAttribute('position').array;
    positions.set(newPositions, positionIndex);
    const newUvs = newGeometry.getAttribute('uv').array;
    uvs.set(newUvs, uvIndex);
    const newDhs = newGeometry.getAttribute('dh').array;
    dhs.set(newDhs, dhIndex);
    const newDls = newGeometry.getAttribute('dl').array;
    dls.set(newDls, dhIndex);
    const newDrs = newGeometry.getAttribute('dr').array;
    drs.set(newDrs, dhIndex);
    const newDys = newGeometry.getAttribute('dy').array;
    dys.set(newDys, dhIndex);

    positionIndex += newPositions.length;
    dhIndex += newDhs.length;
    uvIndex += newUvs.length;
  }

  const geometry = new THREE.BufferGeometry();
  geometry.addAttribute('position', new THREE.BufferAttribute(positions, 3));
  geometry.addAttribute('uv', new THREE.BufferAttribute(uvs, 2));
  geometry.addAttribute('dh', new THREE.BufferAttribute(dhs, 4));
  geometry.addAttribute('dl', new THREE.BufferAttribute(dls, 4));
  geometry.addAttribute('dr', new THREE.BufferAttribute(drs, 4));
  geometry.addAttribute('dy', new THREE.BufferAttribute(dys, 4));
  geometry.applyMatrix(new THREE.Matrix4().makeRotationFromEuler(new THREE.Euler(0, Math.PI, 0, rotationOrder)));
  geometry.boundingSphere = new THREE.Sphere(
    new THREE.Vector3(0, 0, 0),
    2 / scale
  );
  return geometry;
})();

const skinMaterial = new THREE.ShaderMaterial({
  uniforms: THREE.UniformsUtils.clone(SKIN_SHADER.uniforms),
  vertexShader: SKIN_SHADER.vertexShader,
  fragmentShader: SKIN_SHADER.fragmentShader,
  side: THREE.DoubleSide,
  transparent: true,
});
skinMaterial.volatile = true;

const _requestImage = src => new Promise((accept, reject) => {
  const img = new Image();
  img.onload = () => {
    accept(img);
  };
  img.onerror = err => {
    reject(img);
  };
  // img.crossOrigin = 'Anonymous';
  img.src = src;
});
const _requestImageBitmap = src => _requestImage(src)
  .then(img => createImageBitmap(img, 0, 0, img.width, img.height, {
    imageOrientation: 'flipY',
  }));

const skin = ({limbs = false} = {}) => {
  const texture = new THREE.Texture();
	texture.magFilter = THREE.NearestFilter;
	texture.minFilter = THREE.NearestMipMapNearestFilter;

  const material = skinMaterial;

  const mesh = new THREE.Mesh(skinGeometry, material);
  mesh.scale.set(scale, scale, scale);
  mesh.rotation.order = rotationOrder;
  mesh.updateMatrixWorld();

  mesh.setImage = img => {
    texture.image = img;
    texture.needsUpdate = true;
  };

  mesh.size = skinSize;

  if (limbs) {
    const head = new THREE.Object3D();
    head.position.y = offsetY;
    mesh.add(head);
    mesh.head = head;
    const eye = new THREE.Object3D();
    eye.position.z = -8/2;
    head.add(eye);
    mesh.eye = eye;

    const leftArm = new THREE.Object3D();
    leftArm.position.set(-6, -10 + 12/2 + offsetY, 0);
    mesh.add(leftArm);
    const rightArm = new THREE.Object3D();
    rightArm.position.set(6, -10 + 12/2 + offsetY, 0);
    mesh.add(rightArm);
    mesh.arms = {
      left: leftArm,
      right: rightArm,
    };
  }

  mesh.onBeforeRender = () => {
    material.uniforms.map.value = texture;
  };

  mesh.destroy = () => {
    // material.dispose();
    texture.dispose();
  };

  return mesh;
};
skin.SKIN_SHADER = SKIN_SHADER;

return skin;

})();
