#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) flat in uint textureIdx;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunLight;
} scene;

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
  vec4 color = texture(sampler2D(textures[nonuniformEXT(textureIdx)], textureSampler), uv);
  vec4 brightness = vec4(0.3 + dot(normal, scene.sunLight) * 0.7);

  outColor = color * brightness;
}
