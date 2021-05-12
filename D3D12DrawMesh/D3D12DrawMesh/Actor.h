#pragma once
#include "stdafx.h"
#include "ActorComponent.h"

class AActor
{
private:
	vector<shared_ptr<FActorComponent>> Components;

public:
	vector<shared_ptr<FActorComponent>>& GetComs() { return Components; }

	AActor() = default;
	~AActor() = default;
};
