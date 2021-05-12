#pragma once
#include "stdafx.h"

class FAnimaInstance
{
private:

public:
	void initAnimation(); // start timer counting
	void UpdateAnimation(); // invoke in main loop, make character to move

	FAnimaInstance() = default;
	~FAnimaInstance() = default;

private:

};

