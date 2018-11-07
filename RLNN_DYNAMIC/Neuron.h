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

	Neuron* neurons[MAX_CONNECTIONS];

	uint8_t potential;

	Neuron(double x, double y, double z) : x(x), y(y), z(z) {}
	Neuron() : Neuron(0, 0, 0) {}
};
