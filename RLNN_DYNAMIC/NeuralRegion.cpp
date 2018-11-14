#include "NeuralRegion.h"

NeuralLayer * NeuralRegion::createLayer(NeuralLayerParams params)
{
	NeuralLayer* layer = new NeuralLayer(params);

	layers.push_back(layer);

	return layer;
}

void NeuralRegion::wire()
{

}
