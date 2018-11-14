#pragma once
#include <cstdint>
#include <vector>

#include "Wireable.h"

struct Neuron : public Wireable
{
	double x;
	double y;
	double z;

	size_t layer;

	Neurons connections;

	Neuron(double x, double y, double z, size_t layer) : x(x), y(y), z(z), layer(layer) {}
	Neuron() : Neuron(0, 0, 0, 0) {}

	void addConnection(Neuron* neuron);

	virtual void wire() override;
};

typedef std::vector<Neuron*> Neurons;
