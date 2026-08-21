#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 uvIn;
layout(location = 3) in uint pivotIdx;
layout(location = 4) in uint textureIdx;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunlight;
  mat4 normalMatrix;
  uint pivotEnabler;
} scene;

layout(binding = 3) uniform Pivot {
  mat4 matrix;
} pivotMatrices[];

layout(location = 0) out vec2 uvOut;
layout(location = 1) out vec3 normalOut;
layout(location = 2) flat out uint textureIdxOut;

void main() {
  uvOut = vec2(uvIn.x, 1.0 - uvIn.y);
  normalOut = normalize(scene.normalMatrix * vec4(normalIn, 1.0)).xyz;
  textureIdxOut = textureIdx;

  gl_Position = scene.mvpMatrix
    * pivotMatrices[nonuniformEXT(pivotIdx * scene.pivotEnabler)].matrix
    * vec4(position, 1.0);
}
