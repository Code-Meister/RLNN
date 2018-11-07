#version 330 core

in vec2 UV;

layout(location = 0) out vec3 color;

uniform sampler2D framebuffer_data;

in vec4 gl_FragCoord;
in vec3 layerColor;

void main()
{	
	color = texture(framebuffer_data, UV).rgb;
}

