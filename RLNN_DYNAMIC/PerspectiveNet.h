#pragma once
#include "Perspective.h"
#include <vector>

class PerspectiveNet
{
public:
	void initGLData();

	PerspectiveNet();
	~PerspectiveNet();

	void forwardPass();

	void render();

	std::vector<Perspective<300, 300>*> perspectives;
};

