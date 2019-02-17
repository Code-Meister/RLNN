#pragma once
#include <vector>
#include "GL\glew.h"
#include <string>
#include "assert.h"

typedef std::string String;

struct LayerData
{
	float* weights = nullptr;
	float* biases = nullptr;

	size_t weightSize = 1;
	size_t biasSize = 1;

	~LayerData()
	{
		delete[] weights;
		delete[] biases;
	}

	virtual void setWeight(float value) {}
	virtual void setBias(float value) {}

	virtual void validate() {}
};

template <size_t... Dimensions>
struct LayerDataT : public LayerData
{
	static const size_t dimensions = sizeof...(Dimensions);
	const size_t dimensionSizes[dimensions] = { Dimensions... };

	LayerDataT()
	{
		for (int i = 0; i < dimensions; i++)
		{
			weightSize *= dimensionSizes[i];

			if (i == dimensions - 1)
			{
				biasSize = dimensionSizes[i];
			}
		}

		weights = new float[weightSize];
		biases = new float[biasSize];

		memset(weights, 0, sizeof(float) * weightSize);
		memset(biases, 0, sizeof(float) * biasSize);
	}

	size_t weightIndex = 0;
	size_t biasIndex = 0;

	virtual void setWeight(float value) override
	{
		weights[weightIndex++] = value;
	}

	virtual void setBias(float value) override
	{
		biases[biasIndex++] = value;
	}

	virtual void validate() override
	{
		assert(weightIndex == weightSize -1);
		assert(biasIndex == biasSize - 1);
	}
};

class StyleTransferNet
{
public:
	void initGLData();

	StyleTransferNet();
	~StyleTransferNet();

	void forwardPass();

	void render();

	void loadLayer(String filename, LayerData& layer);
};

