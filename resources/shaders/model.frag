#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define NUM_ALLOWED_TEXTURES 24

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) flat in uint textureIdx;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunlight;
  mat4 normalMatrix;
} scene;

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D textures[NUM_ALLOWED_TEXTURES];

void main() {
  vec4 brightness = vec4(0.3 + abs(dot(normal, scene.sunlight)) * 0.7);
  brightness.w = 1.0f;

  outColor = texture(sampler2D(textures[nonuniformEXT(textureIdx % NUM_ALLOWED_TEXTURES)], textureSampler), uv) * brightness;
}
