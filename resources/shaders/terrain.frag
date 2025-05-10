#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) flat in uvec2 textureIdx;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in float uvAlpha;
layout(location = 4) in vec2 uvCloud;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunLight;
} scene;

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D textures[];
layout(binding = 3) uniform texture2D cloudTexture;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 color = texture(sampler2D(textures[nonuniformEXT(textureIdx.x)], textureSampler), uv);
  vec4 brightness = vec4(0.3 + dot(normal, scene.sunLight) * 0.7);

  vec4 color2 = texture(sampler2D(textures[nonuniformEXT(textureIdx.y)], textureSampler), uv);

  vec4 valueCloud = texture(sampler2D(cloudTexture, textureSampler), uvCloud);
  float cloudGray = clamp(((valueCloud.r + valueCloud.g + valueCloud.b) / 3.0) * 1.25, 0.0, 1.0);
  vec4 brightnessCloud = vec4(cloudGray, cloudGray, cloudGray, 1.0);

  outColor =
    (color * brightness * (1.0 - uvAlpha)
      + color2 * brightness * uvAlpha)
      * brightnessCloud;
}
