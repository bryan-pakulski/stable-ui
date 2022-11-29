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
    vec4 totalOffset = vec4(offset.x, offset.y, 0.0, 0.0);

    gl_Position = model * viewProjection * vec4(aPos, 1.0) + totalOffset;
    ourColor = aColor;
    TexCoord = aTexCoord;
}