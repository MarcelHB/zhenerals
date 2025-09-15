#version 450
layout(location = 0) in vec3 colorIn;

layout(location = 0) out vec4 colorOut;

void main() {
  colorOut = vec4(colorIn, 1.0);
}
