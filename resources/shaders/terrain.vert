#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 uvIn;
layout(location = 3) in uint textureIdxIn;
layout(location = 4) in uint textureIdx2In;
layout(location = 5) in float uvAlphaIn;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
  vec3 sunLight;
} scene;

layout(location = 0) flat out uvec2 textureIdxOut;
layout(location = 1) out vec2 uvOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float uvAlphaOut;

void main() {
  textureIdxOut.x = textureIdxIn;
  textureIdxOut.y = textureIdx2In;

  uvOut = uvIn;
  normalOut = normalIn;
  uvAlphaOut = uvAlphaIn;

  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
