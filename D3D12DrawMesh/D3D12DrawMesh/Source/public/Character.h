#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"
#include "Key.h"

class ACharacter : public ASkeletalMeshActor
{
private:
	float MoveSpeed = 300.f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;

public:
	ACharacter() = default;
	~ACharacter() = default;

	void Tick(const float& ElapsedSeconds);
	void TickPosition(const float& ElapsedSeconds);
};

