#include "NeuralNet.h"
#include <random>
#include <iostream>

NeuralRegion* NeuralNet::createRegion()
{
	NeuralRegion* region = new NeuralRegion;

	regions.push_back(region);

	return region;
}

void NeuralNet::wire()
{

}
