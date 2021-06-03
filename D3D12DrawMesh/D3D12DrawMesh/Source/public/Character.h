#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"
#include "Key.h"

class ACharacter : public AActor
{
private:
	float MoveSpeed = 300.f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;

	bool PosDirty = true;

public:
	void SetCurrentAnim(string Key);
	void SetSkeletalMeshCom(shared_ptr<FSkeletalMeshComponent> SkeMesh) { Components.push_back(SkeMesh); }
	void Tick( const float& ElapsedSeconds );
	void TickPosition(const float& ElapsedSeconds);

	FSkeletalMeshComponent* GetSkeletalMeshCom() { return Components[0]->As<FSkeletalMeshComponent>(); }

	const bool& IsPosDirty() const { return PosDirty; }
	void SetPosDirty(const bool& Dirty) { PosDirty = Dirty; }

	ACharacter() = default;
	~ACharacter() = default;
};

