#pragma once
#include <cstdint>
#include <vector>

#include "Neuron.h"
#include "NeuralRegion.h"

class NeuralNet : public Wireable
{
public:
	NeuralRegions regions;

	NeuralRegion* createRegion();

	virtual void wire() override;
};

typedef std::vector<NeuralNet*> NeuralNets;
