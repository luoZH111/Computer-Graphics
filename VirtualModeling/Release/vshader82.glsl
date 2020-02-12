#version 330 core

in  vec4 vPosition;
in  vec4 vColor;

in vec4 vNormal;

out vec4 color;
out vec3 N;
out vec3 V;

uniform float isShadow;
uniform mat4 modelViewMatrix;
uniform mat4 projMatrix;
uniform mat4 rotateMatrix;
uniform vec4 draw_color;

void main() 
{
    //color = vColor;
    color = draw_color;

	if(isShadow==0.0)
	{
		gl_Position = projMatrix * modelViewMatrix * vPosition;
		// 将顶点变换到相机坐标系下
		vec4 vertPos_cameraspace = modelViewMatrix * vPosition;
		// 将法向量变换到相机坐标系下并传入片元着色器
		N = (modelViewMatrix * vNormal).xyz;
		// 对顶点坐标做透视投影
		V = vertPos_cameraspace.xyz / vertPos_cameraspace.w;
	}
	else if(isShadow==1.0)  
	{
		vec4 v1 = projMatrix * modelViewMatrix * vPosition;
		vec4 v2 = vec4(v1.xyz / v1.w, 1.0);
		gl_Position = rotateMatrix * v2;
	}
} 
