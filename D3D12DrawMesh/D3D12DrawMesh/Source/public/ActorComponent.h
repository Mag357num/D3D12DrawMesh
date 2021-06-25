#pragma once
#include "stdafx.h"
#include "Bounding.h"

enum class EComponentType
{
	STATICMESH_COMPONENT = 0,
	CAMERA_COMPONENT = 1,
	DIRECTIONALLIGHT_COMPONENT = 2,
	POINTLIGHT_COMPONENT = 3,
	SKELETALMESH_COMPONENT = 4,
	UNKNOW = 5
};

class FActorComponent : public std::enable_shared_from_this<FActorComponent>
{
protected:
	EComponentType Type;
public:
	FActorComponent() = default;
	virtual ~FActorComponent() = default;

	//void SetType(EComponentType T) { Type = T; }
	const EComponentType& GetType() { return Type; }
};

class AActor;
class FSceneComponent : public FActorComponent
{
protected:
	bool WorldMatrixDirty = true;
	FTransform Transform;
	FMatrix WorldMatrix;
	FBoxSphereBounds Bounding;
	FSceneComponent* AttachParent;
	vector<FSceneComponent*> AttachChildren;
	AActor* Owner;

public:
	FSceneComponent() = default;

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
	void SetOwner(AActor* Actor) { Owner = Actor; }

	const FTransform& GetTransform() const { return GetTransform_Base(); }
	const FMatrix& GetWorldMatrix() { return GetWorldMatrix_Base(); }
	FBoxSphereBounds& GetBounding() { return GetBounding_Base(); }
	FSceneComponent* GetAttachParent() { return AttachParent; }
	vector<FSceneComponent*> GetAttachChildren() { return AttachChildren; }
	AActor* GetOwner() { return Owner; }

	void AttachToComponent(FSceneComponent* InParent);
	void DetachFromComponent();

protected:
	void SetScale_Base(const FVector& Scale) { Transform.Scale = Scale; WorldMatrixDirty = true; Bounding.Scale(Scale); }
	void SetQuat_Base(const FQuat& Quat) { Transform.Quat = Quat; WorldMatrixDirty = true; Bounding.Quat(Quat); }
	void SetTranslate_Base(const FVector& Translate) { Transform.Translation = Translate; WorldMatrixDirty = true; Bounding.Translate(Translate); }
	void SetTransform_Base(const FTransform& Trans) { Transform = Trans; WorldMatrixDirty = true; Bounding.Transform(Trans); }
	const FTransform& GetTransform_Base() const { return Transform; }
	void SetBounding_Base(const FBoxSphereBounds& InBounding) { Bounding = InBounding; }
	FBoxSphereBounds& GetBounding_Base() { return Bounding; }
	//const FBox GetBoundingBox_Base() const { return Bounding.GetBox(); }

	void SetWorldMatrix_Base(const FMatrix& Matrix)
	{
		WorldMatrix = Matrix;

		FVector Skew;
		FVector4 Perspective;
		glm::decompose(Matrix, Transform.Scale, Transform.Quat, Transform.Translation, Skew, Perspective);

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
class FMeshComponent : public FSceneComponent
{
private:
	vector<shared_ptr<FMaterialInterface>> Materials;

public:
	FMeshComponent() = default;

	void SetMaterials(vector<shared_ptr<FMaterialInterface>> Mats) { Materials = Mats; }
	void SetMaterial(shared_ptr<FMaterialInterface> Mat, uint32 index);
	FMaterialInterface* GetMaterial(uint32 index) const { return Materials[index].get(); }
};