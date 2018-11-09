#version 330 core

in vec2 UV;

layout(location = 0) out vec3 color;

uniform sampler2D newData;
uniform sampler2D oldData;

void main()
{	
	vec3 current = texture(newData,UV).rgb;

	vec3 prev = texture(oldData,UV).rgb;

	color = current + prev * 0.5;
}
