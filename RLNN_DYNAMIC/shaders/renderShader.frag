#version 330 core

in vec2 UV;

layout(location = 0) out vec3 color;

uniform sampler2D originalImage;
uniform sampler2D renderedImage;

void main()
{	
	color = abs(texture(originalImage, UV).rgb - texture(renderedImage, UV).rgb);
}
