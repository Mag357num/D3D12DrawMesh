#pragma once
#include "stdafx.h"
#include "Bounding.h"

class FActorComponent
{
protected:
	bool WorldMatrixDirty = true;
	FTransform Transform;
	FMatrix WorldMatrix;
	FBoxSphereBounds Bounding;

public:
	FActorComponent() = default;
	~FActorComponent() = default;

	template <typename T>
	inline T* As() { return static_cast<T*>(this); }
	template <typename T>
	inline T* TryAs() { return dynamic_cast<T*>(this); }

public:
	void SetScale(const FVector& Scale) { SetScale_Base(Scale); }
	void SetQuat(const FQuat& Quat) { SetQuat_Base(Quat); }
	void SetTranslate(const FVector& Translate) { SetTranslate_Base(Translate); }
	void SetTransform(const FTransform& Trans) { SetTransform_Base(Trans); }
	void SetWorldMatrix(const FMatrix& Matrix) { SetWorldMatrix_Base(Matrix); }
	void SetBounding(const FBoxSphereBounds& InBounding) { SetBounding_Base(InBounding); }

	const FTransform& GetTransform() const { return GetTransform_Base(); }
	const FMatrix& GetWorldMatrix() { return GetWorldMatrix_Base(); }
	const FBoxSphereBounds& GetBounding() const { return GetBounding_Base(); }
	const FBox GetBoundingBox() const { return GetBoundingBox_Base(); }

protected:
	void SetScale_Base(const FVector& Scale) { Transform.Scale = Scale; WorldMatrixDirty = true; Bounding.Scale(Scale); }
	void SetQuat_Base(const FQuat& Quat) { Transform.Quat = Quat; WorldMatrixDirty = true; Bounding.Quat(Quat); }
	void SetTranslate_Base(const FVector& Translate) { Transform.Translation = Translate; WorldMatrixDirty = true; Bounding.Translate(Translate); }
	void SetTransform_Base(const FTransform& Trans) { Transform = Trans; WorldMatrixDirty = true; Bounding.Transform(Trans); }
	const FTransform& GetTransform_Base() const { return Transform; }
	void SetBounding_Base(const FBoxSphereBounds& InBounding) { Bounding = InBounding; }
	const FBoxSphereBounds& GetBounding_Base() const { return Bounding; }
	const FBox GetBoundingBox_Base() const { return Bounding.GetBox(); }

	void SetWorldMatrix_Base(const FMatrix& Matrix)
	{
		WorldMatrix = Matrix;

		Transform.Quat = glm::quat_cast(Matrix);
		Transform.Translation = FVector(Matrix[3][0], Matrix[3][1], Matrix[3][2]);  // FMatrix[column][row]
		Transform.Scale = FVector(1.f, 1.f, 1.f);
		Bounding.Transform(Transform);

		WorldMatrixDirty = false;
	}

	const FMatrix& GetWorldMatrix_Base()
	{
		if (WorldMatrixDirty)
		{
			WorldMatrix = translate(Transform.Translation) * toMat4(Transform.Quat) * scale(Transform.Scale);
			WorldMatrixDirty = false;
		}
		return WorldMatrix;
	}
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