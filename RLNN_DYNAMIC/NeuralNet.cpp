#include "NeuralNet.h"
#include <random>
#include <iostream>

void NeuralNet::generate(uint32_t seed)
{
	srand(seed);

	double radius = 1.0;

	auto getRandCoord = [](double& limit) -> double
	{
		/*std::random_device seed;
		std::mt19937 gen(seed());
		std::uniform_real_distribution<float> dist(-sqrt(limit), sqrt(limit));
		double amount = dist(gen);*/

		double amount = (double(rand()) / double(RAND_MAX)) * sqrt(limit);
		if (rand() % 2) amount *= -1.0;
		limit -= pow(amount, 2.0);

		

		return amount;
	};

	for (uint8_t j = 0; j < LAYERS; j++)
	{
		double newRad = radius * (j + 1);

		for (int i = 0; i < NEURONS_PER_LAYER; i++)
		{
			double available = newRad * newRad;

			double coords[3];

			coords[0] = getRandCoord(available);
			coords[1] = getRandCoord(available);
			coords[2] = sqrt(available);
			if (rand() % 2) coords[2] *= -1.0;
			uint8_t possibilities[6][3] =
			{
				{ 0, 1, 2 },
				{ 0, 2, 1 },
				{ 1, 0, 2 },
				{ 1, 2, 0 },
				{ 2, 0, 1 },
				{ 2, 1, 0 },
			};
			uint8_t order = rand() % 6;

			double x = coords[possibilities[order][0]];
			double y = coords[possibilities[order][1]];
			double z = coords[possibilities[order][2]];
			
			//std::cout << x << ", " << y << ", " << z << std::endl;

			neurons.push_back(Neuron{ x, y, z, j });
		}
	}	
}

void NeuralNet::generate()
{
	generate(0);
}
