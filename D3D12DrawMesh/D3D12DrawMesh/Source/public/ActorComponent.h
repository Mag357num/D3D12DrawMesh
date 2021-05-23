#pragma once
#include "stdafx.h"

class FActorComponent
{
protected:
	FTransform Transform;
public:
	void SetTransform(const FTransform& Trans) { Transform = Trans; }

	FTransform& GetTransform() { return Transform; }

	FActorComponent() = default;
	~FActorComponent() = default;

	template <typename T>
	inline T* As() { return static_cast<T*>(this); }
	template <typename T>
	inline T* TryAs() { return dynamic_cast<T*>(this); }
};
