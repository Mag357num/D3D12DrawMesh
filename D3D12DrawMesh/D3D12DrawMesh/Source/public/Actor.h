#pragma once
#include "stdafx.h"
#include "ActorComponent.h"

class AActor
{
protected:
	vector<shared_ptr<TActorComponent>> Components;

public:
	vector<shared_ptr<TActorComponent>>& GetComs() { return Components; }

	AActor() = default;
	~AActor() = default;
};
