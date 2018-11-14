#include "NeuralArchitecture.h"

void NeuralArchitecture::generate(uint32_t seed)
{
	for (int i = 0; i < MAX_NEURONS; i++)
	{
		offsets[i * 3 + 0] = (GLfloat)net.neurons[i].x;
		offsets[i * 3 + 1] = (GLfloat)net.neurons[i].y;
		offsets[i * 3 + 2] = (GLfloat)net.neurons[i].z;

		colors[i * 3 + 0] = layerColors[net.neurons[i].layer * 3 + 0];
		colors[i * 3 + 1] = layerColors[net.neurons[i].layer * 3 + 1];
		colors[i * 3 + 2] = layerColors[net.neurons[i].layer * 3 + 2];

		Neuron* n1 = &net.neurons[i];
		for (int j = 0; j < MAX_CONNECTIONS; j++)
		{
			Neuron* n2 = n1->connections[j];

			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 0] = n1->x;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 1] = n1->y;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 2] = n1->z;

			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 3] = n2->x;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 4] = n2->y;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 5] = n2->z;
		}
	}

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

	for (size_t j = 0; j < LAYERS; j++)
	{
		double newRad = radius * (j + 1);

		for (size_t i = 0; i < NEURONS_PER_LAYER; i++)
		{
			double available = newRad * newRad;

			uint32_t d1 = ((sqrt(NEURONS_PER_LAYER) + 1) / 2) * 2;

			while (NEURONS_PER_LAYER % d1) d1--;
			uint32_t d2 = NEURONS_PER_LAYER / d1;

			uint32_t rows = d2 > d1 ? d2 : d1;
			uint32_t cols = d2 > d1 ? d1 : d2;

			uint32_t col = i % cols;
			uint32_t row = i / cols;

			double x = double(j) * radius - (double(LAYERS) * radius / 2.0);
			double y = col * (radius / 10.0) - ((cols - 1) * radius / 10.0) / 2.0;
			double z = row * (radius / 10.0) - ((rows - 1) * radius / 10.0) / 2.0;

			/*
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
			*/

			neurons.push_back(Neuron{ x, y, z, j });
		}
	}
}

void NeuralArchitecture::generate()
{
	generate(0);
}

void NeuralArchitecture::wire()
{
	for (Neuron& neuron : neurons)
	{
		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			Neuron* newNeuron = getRandomNeuron();
			neuron.addConnection(newNeuron);
		}
	}
}

void NeuralArchitecture::build()
{

}

Neuron* NeuralArchitecture::getRandomNeuron()
{
	size_t index = rand() % neurons.size();

	return &neurons[index];
}

NeuralNet * NeuralArchitecture::createNet()
{
	NeuralNet* net = new NeuralNet;

	nets.push_back(net);

	return net;
}
