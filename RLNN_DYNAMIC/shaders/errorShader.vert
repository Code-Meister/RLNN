#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 offset;
layout(location = 2) in vec3 color;

uniform mat4 MVP;

out vec2 UV;

void main()
{
	gl_Position =  MVP * vec4(vertexPosition_modelspace + offset, 1);
	UV = (vertexPosition_modelspace.xy+vec2(1,1))/2.0;
	layerColor = color;
}

