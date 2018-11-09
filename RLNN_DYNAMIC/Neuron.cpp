#include "Neuron.h"

void Neuron::addConnection(Neuron * neuron)
{
	if (connectionCount < MAX_CONNECTIONS)
	{
		connections[connectionCount++] = neuron;
	}
}
