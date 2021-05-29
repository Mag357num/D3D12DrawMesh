#pragma once
#include "stdafx.h"

struct TStaticVertex
{
	FVector Pos;
	FVector Nor;
	FVector2 UV0;
	FVector4 Color;

	TStaticVertex() = default;
	TStaticVertex(FVector Pos, FVector Nor, FVector2 UV0, FVector4 Color)
		: Pos(Pos), Nor(Nor), UV0(UV0), Color(Color) {}
	~TStaticVertex() = default;
};
