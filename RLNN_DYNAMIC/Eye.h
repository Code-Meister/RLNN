#pragma once
#include "NeuralLayer.h"
#include "RGBNeuron.h"

#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 128

class Eye
{
public:
	Eye();
	~Eye();

	NeuralLayer<RGBNeuron, IMAGE_WIDTH, IMAGE_HEIGHT> inputLayer;
};

