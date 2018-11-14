#include "Neuron.h"

inline void Neuron::addConnection(Neuron * neuron)
{
	connections.push_back(neuron);
}

void Neuron::wire()
{

}
