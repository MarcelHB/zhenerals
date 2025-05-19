#version 450
layout(location = 0) in vec2 uv;
layout(location = 1) in float opacity;
layout(location = 2) in vec2 uvCloud;

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D waterTexture;
layout(binding = 3) uniform texture2D cloudTexture;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 color = texture(sampler2D(waterTexture, textureSampler), uv);
  color.a = opacity;

  vec4 valueCloud = texture(sampler2D(cloudTexture, textureSampler), uvCloud);
  float cloudGray = clamp(((valueCloud.r + valueCloud.g + valueCloud.b) / 3.0) * 1.25, 0.0, 1.0);
  vec4 brightnessCloud = vec4(cloudGray, cloudGray, cloudGray, 1.0);

  outColor = color * brightnessCloud;
}
