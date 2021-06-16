#pragma once
#include "stdafx.h"

class FMaterialInterface;
class FActorComponent
{
protected:
	bool Dirty = true;
	FTransform Transform;
	FMatrix WorldMatrix;
	vector<shared_ptr<FMaterialInterface>> Materials;

public:
	void SetScale(const FVector& Scale) { Transform.Scale = Scale; Dirty = true; }
	void SetQuat(const FQuat& Quat) { Transform.Quat = Quat; Dirty = true; }
	void SetTranslate(const FVector& Translate) { Transform.Translation = Translate; Dirty = true; }
	void SetTransform(const FTransform& Trans) { Transform = Trans; Dirty = true; }
	void SetWorldMatrix(const FMatrix& Matrix);
	void SetMaterials(vector<shared_ptr<FMaterialInterface>> Mats) { Materials = Mats; }
	void SetMaterial(shared_ptr<FMaterialInterface> Mat, uint32 index);
	const FTransform& GetTransform() const { return Transform; }
	FMaterialInterface* GetMaterial(uint32 index) const { return Materials[index].get(); }
	const FMatrix& GetWorldMatrix()
	{
		if (Dirty)
		{
			WorldMatrix = translate(Transform.Translation) * toMat4(Transform.Quat) * scale(Transform.Scale);
			Dirty = false;
		}
		return WorldMatrix;
	}

	FActorComponent() = default;
	~FActorComponent() = default;

	template <typename T>
	inline T* As() { return static_cast<T*>(this); }
	template <typename T>
	inline T* TryAs() { return dynamic_cast<T*>(this); }
};
