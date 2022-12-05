#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 viewProjection;       // camera perspective
uniform mat4 model;                // Object perspective i.e position / rotation / scale (where and how the shape is in our world)
uniform vec2 offset;               // x,y offset 

void main()
{
    vec2 offset_mod = offset * 2;
    gl_Position = (model * viewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0)) + (viewProjection * vec4(offset_mod.x, offset_mod.y, 0.0, 1.0));
    ourColor = aColor;
    TexCoord = aTexCoord;
}