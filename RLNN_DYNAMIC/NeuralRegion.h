#pragma once

#include <vector>
#include "NeuralLayer.h"

class NeuralRegion : public Wireable
{
public:
	NeuralLayers layers;

	NeuralLayer* createLayer(NeuralLayerParams params);

	virtual void wire() override;
};

typedef std::vector<NeuralRegion*> NeuralRegions;
