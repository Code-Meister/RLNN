#pragma once
#include "Neuron.h"
#include <cstdint>

struct NeuralLayerParams
{
	size_t numNeurons;
	
	NeuralLayerParams() {}
	NeuralLayerParams(size_t numNeurons) : numNeurons(numNeurons) {}
	

};

class NeuralLayer : public Wireable
{
	Neurons neurons;

	NeuralLayer(NeuralLayerParams& params)
	{
		for (int i = 0; i < params.numNeurons; i++)
		{
			neurons.push_back(new Neuron);
		}
	}

	virtual void wire() override;
};

typedef std::vector<NeuralLayer*> NeuralLayers;