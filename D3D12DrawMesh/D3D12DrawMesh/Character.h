#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"

class ACharacter : public AActor
{
public:
	void SetSkeletalMeshCom(shared_ptr<FSkeletalMeshComponent> SkeMesh) { Components.push_back(SkeMesh); }
	void Tick(const float& ElapsedSeconds);

	FSkeletalMeshComponent* GetSkeletalMeshCom() { return Components[0]->As<FSkeletalMeshComponent>(); }
	FVector GetLocation() { return GetSkeletalMeshCom()->GetTransform().Translation; }

	void OnKeyDown(const WPARAM& key);
	void OnKeyUp(const WPARAM& key);

	ACharacter() = default;
	~ACharacter() = default;

private:
	struct KeysPressed
	{
		bool w;
		bool a;
		bool s;
		bool d;
		bool q;
		bool e;

		bool left;
		bool right;
		bool up;
		bool down;
	};

	KeysPressed Keys;

	float MoveSpeed = 300.f;
	float TurnSpeed = 1.570796327f;

};

