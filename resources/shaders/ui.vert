#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uvIn;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  mat4 uvMatrix;
} scene;

layout(location = 0) out vec2 uvOut;

void main() {
  uvOut = (scene.uvMatrix * vec4(uvIn, 1.0, 1.0)).xy;
  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
