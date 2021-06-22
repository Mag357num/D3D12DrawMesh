#include "Actor.h"

FBox AActor::GetComponentsBoundingBox()
{
	FBox Box;
	for (auto i : OwnedComponents)
	{
		Box += i->GetBounding().GetBox();
	}
	return Box;
}