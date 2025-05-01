#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvIn;
layout(location = 3) in uint textureIdxIn;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
} scene;

layout(location = 0) flat out uint textureIdxOut;
layout(location = 1) out vec2 uvOut;

void main() {
  textureIdxOut = textureIdxIn;
  uvOut = uvIn;

  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
