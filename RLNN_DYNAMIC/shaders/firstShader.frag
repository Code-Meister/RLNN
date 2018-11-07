#version 330 core

in vec2 UV;

layout(location = 0) out vec3 color;

uniform sampler2D framebuffer_data;

in vec4 gl_FragCoord;
in vec3 layerColor;

void main()
{	
	color =  layerColor;
}

//need to split up and render quad over texture to fade