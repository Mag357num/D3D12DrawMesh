#pragma once
#include "AnimSequence.h"

class FSkeletalMeshComponent;
struct FAnimInstanceProxy
{
	FSkeletalMeshComponent* SkeletalMeshCom;
	vector<FMatrix> Palette;

	shared_ptr<FAnimSequence> Sequence;
	//FAnimNode_Base* RootNode; // TODO: add animation blend tree to replace Sequence



	void UpdateAnimation(const float& TotalSeconds);
	vector<FMatrix> GetFinalTransforms(float dt);
};

class FAnimInstance
{
private:
	FAnimInstanceProxy Proxy;

public:
	void initAnimation(FSkeletalMeshComponent* SkeletalMeshCom, shared_ptr<FAnimSequence> Sequence); // start timer counting
	void UpdateAnimation(const float& TotalSeconds); // invoke in main loop, make character to move

	FAnimInstanceProxy& GetProxy() { return Proxy; }

	FAnimInstance() = default;
	~FAnimInstance() = default;
};
