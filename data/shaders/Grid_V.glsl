#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 view; // camera perspective
uniform mat4 projection;
uniform mat4 model; // Object perspective i.e position / rotation / scale (where and how the shape is in our world)

void main() {
  mat4 mvp = projection * view * model;
  gl_Position = mvp * vec4(aPos, 1);
}
