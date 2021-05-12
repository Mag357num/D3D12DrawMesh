#pragma once
#include "AnimSequenceBase.h"

class FSkeletalMeshComponent;
struct FAnimInstanceProxy
{
	FSkeletalMeshComponent* SkeletalMeshCom;

	//FAnimNode_Base* RootNode; // TODO: add animation blend tree to replace Sequence
	FAnimSequenceBase* Sequence;

	void UpdateAnimation(const float& ElapsedSeconds);
};

class FAnimInstance
{
private:
	FAnimInstanceProxy Proxy;

public:
	void initAnimation(FSkeletalMeshComponent* SkeletalMeshCom, FAnimSequenceBase* Sequence); // start timer counting
	void UpdateAnimation(const float& ElapsedSeconds); // invoke in main loop, make character to move

	FAnimInstance() = default;
	~FAnimInstance() = default;
};
