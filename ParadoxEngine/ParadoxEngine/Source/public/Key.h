#pragma once
#include "stdafx.h"

struct KeysPressed
{
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;
	bool q = false;
	bool e = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;

	const bool IsEmpty() const;
};
