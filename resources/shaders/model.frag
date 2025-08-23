#version 450
layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunlight;
  mat4 normalMatrix;
} scene;

layout(binding = 1) uniform sampler2D textureSampler;

void main() {
  vec4 brightness = vec4(0.3 + max(0.0, dot(normal, scene.sunlight)) * 0.7);

  outColor = texture(textureSampler, uv) * brightness;
}
