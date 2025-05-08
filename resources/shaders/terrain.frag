#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) flat in uvec2 textureIdx;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in float uvAlpha;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunLight;
} scene;

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
  vec4 color = texture(sampler2D(textures[nonuniformEXT(textureIdx.x)], textureSampler), uv);
  vec4 brightness = vec4(0.3 + dot(normal, scene.sunLight) * 0.7);

  vec4 color2 = texture(sampler2D(textures[nonuniformEXT(textureIdx.y)], textureSampler), uv);

  outColor = color * brightness * (1.0 - uvAlpha) + color2 * brightness * uvAlpha;
}
