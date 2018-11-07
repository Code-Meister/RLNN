#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 offset;
layout(location = 2) in vec3 color;

out vec2 UV;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

out vec3 layerColor;

void main()
{
	gl_Position =  MVP * (vec4(vertexPosition_modelspace + offset,1));
	//gl_Position += vec4(offset, 1.0);

	layerColor = color;
	UV = (vertexPosition_modelspace.xy+vec2(1,1))/2.0;
}

