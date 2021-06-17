#pragma once
#include "stdafx.h"

class FActorComponent
{
protected:
	bool WorldMatrixDirty = true;
	FTransform Transform;
	FMatrix WorldMatrix;

protected:
	void SetScale(const FVector& Scale) { Transform.Scale = Scale; WorldMatrixDirty = true; }
	void SetQuat(const FQuat& Quat) { Transform.Quat = Quat; WorldMatrixDirty = true; }
	void SetTranslate(const FVector& Translate) { Transform.Translation = Translate; WorldMatrixDirty = true; }
	void SetTransform(const FTransform& Trans) { Transform = Trans; WorldMatrixDirty = true; }
	void SetWorldMatrix(const FMatrix& Matrix);

	const FTransform& GetTransform() const { return Transform; }
	const FMatrix& GetWorldMatrix()
	{
		if (WorldMatrixDirty)
		{
			WorldMatrix = translate(Transform.Translation) * toMat4(Transform.Quat) * scale(Transform.Scale);
			WorldMatrixDirty = false;
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

class FMaterialInterface;
class FMeshComponent : public FActorComponent
{
private:
	vector<shared_ptr<FMaterialInterface>> Materials;

public:
	void SetMaterials(vector<shared_ptr<FMaterialInterface>> Mats) { Materials = Mats; }
	void SetMaterial(shared_ptr<FMaterialInterface> Mat, uint32 index);
	FMaterialInterface* GetMaterial(uint32 index) const { return Materials[index].get(); }
};