#pragma once
#include <functional>

typedef std::function<bool()> WireCondition;

class Wireable
{
	virtual void wire() = 0;
	
	WireCondition condition;
};
