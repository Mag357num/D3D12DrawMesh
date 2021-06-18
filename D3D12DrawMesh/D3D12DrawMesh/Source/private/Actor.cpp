#include "Actor.h"

FBox AActor::GetComponentsBoundingBox()
{
	FBox Box;
	for (auto i : Components)
	{
		Box += i->GetBoundingBox();
	}
	return Box;
}

