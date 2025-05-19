#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uvIn;
layout(location = 2) in float opacityIn;
layout(location = 3) in vec2 uvCloudIn;

layout(binding = 0) uniform Scene {
  mat4 mvpMatrix;
} scene;

layout(location = 0) out vec2 uvOut;
layout(location = 1) out float opacityOut;
layout(location = 2) out vec2 uvCloudOut;

void main() {
  uvOut = uvIn;
  uvCloudOut = uvCloudIn;
  opacityOut = opacityIn;

  gl_Position = scene.mvpMatrix * vec4(position, 1.0);
}
