#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 uvIn;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunlight;
  mat4 normalMatrix;
} scene;

layout(location = 0) out vec2 uvOut;
layout(location = 1) out vec3 normalOut;

void main() {
  uvOut = uvIn;
  normalOut = normalize(scene.normalMatrix * vec4(normalIn, 1.0)).xyz;

  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
