#pragma once
#include <cstdint>

#define MAX_CONNECTIONS 32
#define MAX_POTENTIAL 32
#define DECAY_RATE 1

struct Neuron
{
	double x;
	double y;
	double z;

	uint8_t layer;

	Neuron* neurons[MAX_CONNECTIONS];

	Neuron(double x, double y, double z, uint8_t layer) : x(x), y(y), z(z), layer(layer) {}
	Neuron() : Neuron(0, 0, 0, 0) {}
};
