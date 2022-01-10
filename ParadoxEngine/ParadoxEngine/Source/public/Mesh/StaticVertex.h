#pragma once
#include "stdafx.h"

struct FStaticVertex
{
	FVector Pos;
	FVector Nor;
	FVector2 UV0;
	FVector4 Color;

	FStaticVertex() = default;
	FStaticVertex(FVector Pos, FVector Nor, FVector2 UV0, FVector4 Color)
		: Pos(Pos), Nor(Nor), UV0(UV0), Color(Color) {}
	~FStaticVertex() = default;
};
