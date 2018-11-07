#include "NeuralNet.h"
#include <random>
#include <iostream>

void NeuralNet::generate(uint32_t seed)
{
	srand(seed);

	double radius = 10.0;

	auto getRandCoord = [](double& limit) -> double
	{
		double amount = (double(rand()) / double(RAND_MAX)) * sqrt(limit);
		limit -= pow(amount, 2.0);

		return amount;
	};

	for (int j = 0; j < LAYERS; j++)
	{
		for (int i = 0; i < NEURONS_PER_LAYER; i++)
		{
			double available = radius * radius;

			double x = getRandCoord(available);
			double y = getRandCoord(available);
			double z = sqrt(available);

			neurons.push_back(Neuron{ x,y,z });
		}
	}	
}

void NeuralNet::generate()
{
	generate(0);
}
