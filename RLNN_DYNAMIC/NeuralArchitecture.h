#pragma once
#include "NeuralNet.h"

class NeuralArchitecture
{
public:
	NeuralNets nets;

	void build();

	Neuron* getRandomNeuron();

	NeuralNet* createNet();
};
