#pragma once
#include "stdafx.h"

class TActorComponent
{
protected:
	FTransform Transform;
public:
	void SetTransform(const FTransform& Trans) { Transform = Trans; }

	FTransform& GetTransform() { return Transform; }

	TActorComponent() = default;
	~TActorComponent() = default;

	template <typename T>
	inline T* As() { return static_cast<T*>(this); }
	template <typename T>
	inline T* TryAs() { return dynamic_cast<T*>(this); }
};
