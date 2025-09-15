#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colorIn;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
} scene;

layout(location = 0) out vec3 colorOut;

void main() {
  colorOut = colorIn;

  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
