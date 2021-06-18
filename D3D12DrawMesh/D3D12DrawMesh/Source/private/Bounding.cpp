#include "Bounding.h"

const FBox FBoxSphereBounds::GetBox() const
{
	return FBox(Origin - BoxExtent, Origin + BoxExtent);
}
