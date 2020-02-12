#version 330 core

in vec4 vPosition;
in vec4 vColor;
out vec4 color;
uniform mat4 matrix;

void main()
{
    gl_Position = matrix * vPosition;
    color = vColor;
}
