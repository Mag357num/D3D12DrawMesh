#pragma once
#include "stdafx.h"
#include "ActorComponent.h"

// a object that has its own logic function
class AActor
{
protected:
	vector<shared_ptr<FActorComponent>> Components; // entity of actor

public:
	vector<shared_ptr<FActorComponent>>& GetComs() { return Components; }

	AActor() = default;
	~AActor() = default;
};
