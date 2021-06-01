#pragma once
#include "stdafx.h"

class FActorComponent
{
protected:
	bool Dirty = true;
	FTransform Transform;
	FMatrix TransMatrix;
public:
	const bool& IsDirty() const { return Dirty; }

	void SetDirtyValue(const bool& Value ) { Dirty = Value; }
	void SetScale(const FVector& Scale) { Transform.Scale = Scale; Dirty = true; }
	void SetQuat(const FQuat& Quat) { Transform.Quat = Quat; Dirty = true; }
	void SetTranslate(const FVector& Translate) { Transform.Translation = Translate; Dirty = true; }
	void SetTransform(const FTransform& Trans) { Transform = Trans; Dirty = true; }
	void SetMatrix(const FMatrix& Matrix);
	const FTransform& GetTransform() const { return Transform; }
	const FMatrix& GetTransMatrix()
	{
		if (Dirty)
		{
			TransMatrix = translate(Transform.Translation) * toMat4(Transform.Quat) * scale(Transform.Scale);
		}
		return TransMatrix;
	}

	FActorComponent() = default;
	~FActorComponent() = default;

	template <typename T>
	inline T* As() { return static_cast<T*>(this); }
	template <typename T>
	inline T* TryAs() { return dynamic_cast<T*>(this); }
};
