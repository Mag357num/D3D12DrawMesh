#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "AnimSequenceBase.h"

class FAnimNode_Base // Animation BlendTree
{
public:
	virtual void Update() {}
	virtual void Evaluate() {}
};

class FAnimNode_SequencePlyer : public FAnimNode_Base
{
private:
	FAnimSequence* Sequence;

public:
	void Update() override {}
	void Evaluate() override {}
};