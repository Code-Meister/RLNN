#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 offset;
layout(location = 2) in vec3 color;

uniform mat4 MVP;

out vec3 layerColor;

void main()
{
	gl_Position =  MVP * vec4(vertexPosition_modelspace + offset, 1);

	layerColor = color;
}

