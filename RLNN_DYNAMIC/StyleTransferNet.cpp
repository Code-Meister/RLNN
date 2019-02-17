#include "StyleTransferNet.h"

#include "GLAbstraction.h"
#include <functional>

#pragma warning(disable:4996)
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include "cnpy/cnpy.h"

typedef std::string String;

struct Layer
{
	virtual void debug()
	{
		std::cout << "COCKS" << std::endl;
	}

	uint16_t width;
	uint16_t height;
	uint16_t depth;

	String name;

	void getDim()
	{
		std::cout << "[";

		if (width < 10) std::cout << " ";
		if (width < 100) std::cout << " ";

		std::cout << int(width) << ", ";

		if (height < 10) std::cout << " ";
		if (height < 100) std::cout << " ";
		std::cout << int(height) << ", ";

		if (depth < 10) std::cout << " ";
		if (depth < 100) std::cout << " ";
		std::cout << int(depth) << "] ";
	}

	virtual void calcDim(Layer* layer)
	{
		width = layer->width;
		height = layer->height;
		depth = layer->depth;
	}

	virtual void getType()
	{
		std::cout << "NIGGERS ";
	}

	Layer() {};
	Layer(String name, uint16_t width, uint16_t height, uint16_t depth) : name(name), width(width), height(height), depth(depth) {}
};

typedef std::vector<Layer*> Layers;

//interpolate data
struct UpsampleLayer : public Layer
{
	uint8_t upsample_factor;
	UpsampleLayer(String name, uint8_t upsample_factor) : Layer(name, 0, 0, 0), upsample_factor(upsample_factor) {}

	virtual void debug() override
	{
		std::cout << "Upsample Layer, " << "Factor: " << int(upsample_factor) << std::endl;
	}

	virtual void getType() override
	{
		std::cout << "UPSAMPLE ";
	}

	virtual void calcDim(Layer* layer) override
	{
		width = layer->width * upsample_factor;
		height = layer->height * upsample_factor;
		depth = layer->depth;
	}
};

//pads using "mirror of data"
struct ReflectionPad2dLayer : public Layer
{
	uint8_t padding;
	ReflectionPad2dLayer(String name, uint8_t padding) :
		Layer(name, 0, 0, 0),
		padding(padding) {}

	virtual void debug() override
	{
		std::cout << "Refl Pad 2D Layer, " << "Padding: " << int(padding) << std::endl;
	}

	virtual void calcDim(Layer* layer) override
	{
		width = layer->width + padding * 2;
		height = layer->height + padding * 2;
		depth = layer->depth;
	}

	virtual void getType() override
	{
		std::cout << "REFLCTN  ";
	}
};

struct Conv2dLayer : public Layer
{
	size_t in_channels;
	size_t out_channels;
	uint8_t kernal_size;
	uint8_t stride;

	Conv2dLayer(String name, size_t in_channels, size_t out_channels, uint8_t kernal_size, uint8_t stride) :
		Layer(name, 0, 0, 0),
		in_channels(in_channels),
		out_channels(out_channels),
		kernal_size(kernal_size),
		stride(stride) {}

	virtual void debug() override
	{
		std::cout << "Conv 2D Layer, " <<
			"in_channels: " << in_channels <<
			" out_channels: " << out_channels <<
			" kernal_size: " << int(kernal_size) <<
			" stride: " << int(stride) << std::endl;
	}

	virtual void calcDim(Layer* layer) override
	{
		width = (layer->width - kernal_size) / stride + 1;
		height = (layer->height - kernal_size) / stride + 1;
		depth = out_channels;
	}

	virtual void getType() override
	{
		std::cout << "CONV 2D  ";
	}
};

struct InstanceNorm2dLayer : public Layer
{
	size_t out_channels;
	InstanceNorm2dLayer(String name, size_t out_channels) :
		Layer(name, 0, 0, 0),
		out_channels(out_channels) {}

	virtual void debug() override
	{
		std::cout << "Norm Layer, " <<
			"out_channels: " << out_channels << std::endl;
	}

	virtual void getType() override
	{
		std::cout << "INSTNORM ";
	}
};

struct ReLULayer : public Layer
{
	//virtual 

	virtual void debug() override
	{
		std::cout << "Relu Layer" << std::endl;
	}

	ReLULayer(String name) :
		Layer(name, 0, 0, 0)
	{}

	virtual void getType() override
	{
		std::cout << "RELU     ";
	}
};

struct TransformNetConvLayer
{
	size_t in_channels;
	size_t out_channels;

	uint8_t kernel_size;
	uint8_t stride;
	uint8_t upsample;

	bool instance_norm;
	bool relu;

	TransformNetConvLayer(String name, Layers& layers, size_t in_channels, size_t out_channels, uint8_t kernel_size = 3, uint8_t stride = 1, uint8_t upsample = 0, bool instance_norm = true, bool relu = true)
		:
		in_channels(in_channels),
		out_channels(out_channels),
		kernel_size(kernel_size),
		stride(stride),
		upsample(upsample),
		instance_norm(instance_norm),
		relu(relu)
	{
		if (upsample) layers.push_back(new UpsampleLayer(name, upsample));
		layers.push_back(new ReflectionPad2dLayer(name, kernel_size / 2));
		layers.push_back(new Conv2dLayer(name, in_channels, out_channels, kernel_size, stride));
		if (instance_norm) layers.push_back(new InstanceNorm2dLayer(name, out_channels));
		if (relu) layers.push_back(new ReLULayer(name));
	}
};

static Layer inputLayer("Input Layer", 256, 256, 3);

template <size_t count>
class TransformNet
{
	Layers layers;

	size_t resIndex;
	size_t upsampIndex;

public:
	TransformNet()
	{
		TransformNetConvLayer("Downsample 1", layers, 3, count, 9);
		TransformNetConvLayer("Downsample 2", layers, count, count * 2, 3, 2);
		TransformNetConvLayer("Downsample 3", layers, count * 2, count * 4, 3, 2);

		resIndex = layers.size();
		//Residual
		TransformNetConvLayer("Residual 1", layers, count * 4, count * 4);
		TransformNetConvLayer("Residual 1", layers, count * 4, count * 4, 3, 1, 0, true, false);
		TransformNetConvLayer("Residual 2", layers, count * 4, count * 4);
		TransformNetConvLayer("Residual 2", layers, count * 4, count * 4, 3, 1, 0, true, false);
		TransformNetConvLayer("Residual 3", layers, count * 4, count * 4);
		TransformNetConvLayer("Residual 3", layers, count * 4, count * 4, 3, 1, 0, true, false);
		TransformNetConvLayer("Residual 4", layers, count * 4, count * 4);
		TransformNetConvLayer("Residual 4", layers, count * 4, count * 4, 3, 1, 0, true, false);
		TransformNetConvLayer("Residual 5", layers, count * 4, count * 4);
		TransformNetConvLayer("Residual 5", layers, count * 4, count * 4);

		upsampIndex = layers.size();
		//Upsampling
		TransformNetConvLayer("Upsample 1", layers, count * 4, count * 2, 3, 1, 2);
		TransformNetConvLayer("Upsample 2", layers, count * 2, count, 3, 1, 2);
		TransformNetConvLayer("Upsample 3", layers, count, 3, 9, 1, 0, false, false);

		Layer* prevLayer = &inputLayer;
		for (Layer* layer : layers)
		{
			layer->calcDim(prevLayer);

			prevLayer = layer;
		}
	}

	void forward(String styleImage, String contentImage)
	{
		Layer* prevLayer = &inputLayer;

		for (int i = 0; i < layers.size(); i++)
		{
			Layer* layer = layers[i];

			if (i == 0)
			{
				std::cout << "DOWNSAMPLING LAYERS\r\n";
			}
			else if (i == resIndex)
			{
				std::cout << "\r\n\r\nRESIDUAL LAYERS\r\n";
			}
			else if (i == upsampIndex)
			{
				std::cout << "\r\n\r\nUPSAMPLING LAYERS\r\n";
			}

			if (layer->name != prevLayer->name) std::cout << std::endl << layer->name << std::endl;

			layer->getType();
			prevLayer->getDim();
			std::cout << " -> ";
			layer->getDim();

			std::cout << std::endl;

			prevLayer = layer;
		}
	}
};


typedef cnpy::NpyArray NArray;

std::vector<NArray> weights;
std::vector<NArray> biases;


void loadFiles()
{
	weights.push_back(cnpy::npy_load("data/downsampling.1.weight.npy"));

	weights.push_back(cnpy::npy_load("data/downsampling.5.weight.npy"));

	{
		NArray& arr = weights[0];
		size_t len = 1;
		for (int i = 0; i < arr.shape.size(); i++)
		{
			len *= arr.shape[i];
		}

		float* weightData = weights[0].data<float>();
		for (int i = 0; i < len; i++)
		{
			std::cout << weightData[i] << std::endl;

			//if (i && !(i % 10)) std::cin.get();
		}

		std::cin.get();
	}

	weights.push_back(cnpy::npy_load("data/downsampling.9.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.0.conv.1.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.0.conv.5.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.1.conv.1.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.1.conv.5.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.2.conv.1.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.2.conv.5.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.3.conv.1.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.3.conv.5.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.4.conv.1.weight.npy"));
	weights.push_back(cnpy::npy_load("data/residuals.4.conv.5.weight.npy"));
	weights.push_back(cnpy::npy_load("data/upsampling.2.weight.npy"));
	weights.push_back(cnpy::npy_load("data/upsampling.7.weight.npy"));
	weights.push_back(cnpy::npy_load("data/upsampling.11.weight.npy"));

	biases.push_back(cnpy::npy_load("data/downsampling.1.bias.npy"));
	biases.push_back(cnpy::npy_load("data/downsampling.5.bias.npy"));
	biases.push_back(cnpy::npy_load("data/downsampling.9.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.0.conv.1.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.0.conv.5.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.1.conv.1.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.1.conv.5.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.2.conv.1.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.2.conv.5.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.3.conv.1.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.3.conv.5.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.4.conv.1.bias.npy"));
	biases.push_back(cnpy::npy_load("data/residuals.4.conv.5.bias.npy"));
	biases.push_back(cnpy::npy_load("data/upsampling.2.bias.npy"));
	biases.push_back(cnpy::npy_load("data/upsampling.7.bias.npy"));
	biases.push_back(cnpy::npy_load("data/upsampling.11.bias.npy"));

	for (int j = 0; j < weights.size(); j++)
	{
		NArray& arr = weights[j];

		for (int i = 0; i < arr.shape.size(); i++)
		{
			std::cout << arr.shape[i] << " ";
		}

		std::cout << std::endl;
	}

	std::cout << std::endl;

	for (int j = 0; j < biases.size(); j++)
	{
		NArray& arr = biases[j];

		for (int i = 0; i < arr.shape.size(); i++)
		{
			std::cout << arr.shape[i] << " ";
		}

		std::cout << std::endl;
	}
	std::cin.get();

}

int main()
{
	
}


void StyleTransferNet::initGLData()
{
	
}

StyleTransferNet::StyleTransferNet()
{
	loadFiles();

	net = new TransformNet<32>;
	net.forward("style.bmp", "content.bmp");

	std::cin.get();
	return 0;

	initGLData();
}

StyleTransferNet::~StyleTransferNet()
{
	
}

void StyleTransferNet::forwardPass()
{
	
}

void StyleTransferNet::render()
{
	
}

void StyleTransferNet::loadLayer(String filename, LayerData& layer)
{
	String weightsFile = "VGG16/" + filename + "W.txt";
	String biasesFile = "VGG16/" + filename + "b.txt";

	printf("Loading file %s\n", weightsFile.c_str());
	printf("Loading file %s\n\n", biasesFile.c_str());

	auto loadData = [&](String filename, void(LayerData::*callback)(float))
	{
		String fileData;

		std::ifstream weightsFileStream(filename, std::ios::in);
		if (weightsFileStream.is_open())
		{
			std::stringstream sstr;
			sstr << weightsFileStream.rdbuf();
			fileData = sstr.str();
			weightsFileStream.close();

			String buffer;
			for (int i = 0; i < fileData.length(); i++)
			{
				char c = fileData[i];
				if (c == ' ') continue;
				if (c == '\r' || c == '\n') continue;
				if (c == ',')
				{
					float value = strtof(buffer.c_str(), nullptr);

					(layer.*callback)(value);

					buffer = "";
					continue;
				}

				buffer += c;
			}
		}
		else
		{

		}
	};

	loadData(weightsFile, &LayerData::setWeight);
	loadData(biasesFile, &LayerData::setBias);

	layer.validate();
}