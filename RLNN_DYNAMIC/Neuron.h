#pragma once
#include <cstdint>

#define MAX_CONNECTIONS 32
#define MAX_POTENTIAL 32
#define DECAY_RATE 1

class Neuron
{
	float x;
	float y;
	float z;

	Neuron* neurons[MAX_CONNECTIONS];

	uint8_t potential;


};

