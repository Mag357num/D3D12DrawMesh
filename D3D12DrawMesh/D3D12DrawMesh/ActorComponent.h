#pragma once
#include "stdafx.h"

class FActorComponent
{
protected:
	FTransform Transform;
public:
	void SetTransform(const FTransform& Trans) { Transform = Trans; }

	const FTransform& GetTransform() { return Transform; }

	FActorComponent() = default;
	~FActorComponent() = default;
};
