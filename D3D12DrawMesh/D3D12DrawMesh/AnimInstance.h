#pragma once
#include "AnimSequence.h"

class FSkeletalMeshComponent;
struct FAnimInstanceProxy
{
	FSkeletalMeshComponent* SkeletalMeshCom;

	//FAnimNode_Base* RootNode; // TODO: add animation blend tree to replace Sequence
	shared_ptr<FAnimSequence> Sequence;

	void UpdateAnimation(const float& ElapsedSeconds);
};

class FAnimInstance
{
private:
	FAnimInstanceProxy Proxy;

public:
	void initAnimation(FSkeletalMeshComponent* SkeletalMeshCom, shared_ptr<FAnimSequence> Sequence); // start timer counting
	void UpdateAnimation(const float& ElapsedSeconds); // invoke in main loop, make character to move

	FAnimInstanceProxy& GetProxy() { return Proxy; }

	FAnimInstance() = default;
	~FAnimInstance() = default;
};
