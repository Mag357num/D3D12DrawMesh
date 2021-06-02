#pragma once
#include "stdafx.h"
#include "ActorComponent.h"

// a object that has its own logic function
class AActor
{
protected:
	vector<shared_ptr<FActorComponent>> Components; // entity of actor

public:
	AActor() = default;
	~AActor() = default;
};
