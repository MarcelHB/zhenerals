#version 450
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uvIn;

layout(location = 0) out vec2 uvOut;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
} scene;

void main() {
  uvOut = uvIn;
  gl_Position = scene.mvpMatrix * vec4(position, 1.0, 1.0);
}
