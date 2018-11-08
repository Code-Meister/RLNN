#version 330 core

in vec2 UV;

layout(location = 0) out vec3 color;

uniform sampler2D framebuffer_data;

void main()
{	
	color = texture(framebuffer_data, UV).rgb;
}

