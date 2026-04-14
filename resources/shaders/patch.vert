#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 uvIn;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  mat4 uvMatrix;
  vec3 sunLight;
} scene;

layout(location = 0) out vec2 uvOut;
layout(location = 1) out vec3 normalOut;

void main() {
  uvOut = (scene.uvMatrix * vec4(uvIn, 1.0, 1.0)).xy;
  // r/n only facing up, always
  normalOut = normalIn;

  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
