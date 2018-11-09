#version 330 core

layout(location = 0) out vec3 color;

in vec4 gl_FragCoord;
in vec3 layerColor;

void main()
{	
	color = layerColor * gl_FragCoord.x;
}
