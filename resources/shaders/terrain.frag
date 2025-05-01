#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) flat in uint textureIdx;
layout(location = 1) in vec2 uv;

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
  outColor = texture(sampler2D(textures[textureIdx], textureSampler), uv);
}
