#include "Key.h"

const bool KeysPressed::IsEmpty() const
{
	return w || a || s || d || q || e || left || right || up || down;
}
