#version 330 core

in vec3 vPosition;
out vec4 color;

uniform mat4 modelViewprojectMatrix;

void main() 
{
    gl_Position = modelViewprojectMatrix * vec4(vPosition, 1.0);
	
	color = vec4(vPosition+vec3(0.5,0.5,0.5), 1.0);
}