#pragma once
#include "stdafx.h"

class FActorComponent
{
protected:
	bool Dirty = true;
	FTransform Transform;
	FMatrix TransMatrix;
public:
	void SetTransform(const FTransform& Trans)
	{
		Transform = Trans;
		TransMatrix = translate(Trans.Translation) * toMat4(Trans.Quat) * scale(Trans.Scale);
		Dirty = false;
	}

	void SetScale(const FVector Scale) { Transform.Scale = Scale; Dirty = true; }
	void SetQuat(const FQuat Quat) { Transform.Quat = Quat; Dirty = true; }
	void SetTranslate(const FVector Translate) { Transform.Translation = Translate; Dirty = true; }

	const bool& IsDirty() const { return Dirty; }
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
