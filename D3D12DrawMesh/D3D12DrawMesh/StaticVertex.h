#pragma once
#include "stdafx.h"

class FStaticVertex
{
private:
	FVector Pos;
	FVector Nor;
	FVector2 UV0;
	FVector4 Color;

public:
	FStaticVertex() = default;
	FStaticVertex(FVector Pos, FVector Nor, FVector2 UV0, FVector4 Color)
		: Pos(Pos), Nor(Nor), UV0(UV0), Color(Color) {}
	~FStaticVertex() = default;
};
