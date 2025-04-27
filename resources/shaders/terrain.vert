#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint textureIdx;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
} scene;

void main() {
  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
