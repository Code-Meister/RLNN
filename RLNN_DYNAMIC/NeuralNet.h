#pragma once
#include <cstdint>
#include <vector>

#include "Neuron.h"

#define NEURONS_PER_LAYER 128
#define LAYERS 16
#define MAX_NEURONS (NEURONS_PER_LAYER * LAYERS)

typedef std::vector<Neuron> Neurons;
class NeuralNet
{
public:
	Neurons neurons;

	void generate();
	void generate(uint32_t seed);
};
