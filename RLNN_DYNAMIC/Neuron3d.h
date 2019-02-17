#pragma once

struct Position
{
	float x;
	float y;
	float z;
};

struct Color
{
	float r;
	float g;
	float b;
};

class Neuron3d
{
	Position position;
	Color color;
};

