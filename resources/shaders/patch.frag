#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  mat4 uvMatrix;
  vec3 sunlight;
} scene;

layout(binding = 1) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 brightness = vec4(0.4 + max(0.0, dot(normal, scene.sunlight)) * 0.6);
  vec4 color = texture(textureSampler, uv);

  outColor = color * brightness;
}
