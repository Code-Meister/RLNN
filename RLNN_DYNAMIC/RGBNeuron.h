#pragma once
#include "Neuron.h"
#include <cstdint>

class RGBNeuron :
	public Neuron
{
public:
	RGBNeuron();
	~RGBNeuron();

	uint8_t red;
	uint8_t green;
	uint8_t blue;
};
