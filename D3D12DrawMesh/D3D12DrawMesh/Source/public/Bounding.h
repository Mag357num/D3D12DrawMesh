#pragma once
#include "stdafx.h"

class FBox
{
private:
	FVector Min;
	FVector Max;
public:
	FBox() = default;
	FBox(FVector InMin, FVector InMax) { Min = InMin; Max = InMax; }

	FVector GetSize() { return (Max - Min); }
	FVector GetCenter() { return (Max + Min) * 0.5f; }

	FBox& FBox::operator+=(const FBox& Other)
	{
		Min.x = glm::min(Min.x, Other.Min.x);
		Min.y = glm::min(Min.y, Other.Min.y);
		Min.z = glm::min(Min.z, Other.Min.z);

		Max.x = glm::max(Max.x, Other.Max.x);
		Max.y = glm::max(Max.y, Other.Max.y);
		Max.z = glm::max(Max.z, Other.Max.z);

		return *this;
	}
};

class FBoxSphereBounds
{
private:
	/** Holds the origin of the bounding box and sphere. */
	FVector Origin;

	/** Holds the extent of the bounding box. */
	FVector BoxExtent;

	/** Holds the radius of the bounding sphere. */
	float SphereRadius;
public:
	FBoxSphereBounds() = default;
	FBoxSphereBounds(FVector InOrigin, FVector InBoxExtent) : Origin(InOrigin), BoxExtent(InBoxExtent), SphereRadius(glm::length(BoxExtent)) {}

	const FBox GetBox() const;

	void Scale(const FVector& Scale)
	{
		BoxExtent *= Scale;
		SphereRadius = glm::length(BoxExtent);
	}

	void Quat(const FQuat& Quat)
	{
		//BoxExtent *= Quat; // TODO: test the correctness
	}

	void Translate(const FVector& Translate)
	{
		Origin += Translate;
	}

	void Transform(const FTransform& Transform)
	{
		Scale(Transform.Scale);
		Quat(Transform.Quat);
		Translate(Transform.Translation);
	}
};