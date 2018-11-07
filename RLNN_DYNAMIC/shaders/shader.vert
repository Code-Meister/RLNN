#version 430 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexTransform;
layout(location = 2) in vec4 vertexColor;
void main()
{
  gl_Position.xyz = vertexPosition_modelspace + vertexTransform.xyz;
  gl_Position.w = 1.0;
}