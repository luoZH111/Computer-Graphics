#version 330 core

in vec4 color;
in vec2 texCoord;
in vec4 normal;
in vec3 faceIndecies;

out vec4 fColor;
out vec4 fNormal;

uniform sampler2D texture;

//uniform float isShadow;

void main()
{
//	if(isShadow==0.0)
//	{
		//fColor = vec4(1, 0, 0, 0 ) * texture2D( texture, texCoord );
		fColor = texture2D( texture, texCoord );
		//fColor = color;
		//fColor = vec4(0, 0, 0, 0 );
		fNormal = normal;
/*	}
	else if(isShadow==1.0)
	{
		fColor = vec4(0.0,0.0,0.0,1.0);
	}*/
}

