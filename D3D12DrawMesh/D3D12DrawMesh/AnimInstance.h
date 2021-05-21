#pragma once
#include "AnimSequence.h"

class FSkeletalMeshComponent;
struct FAnimInstanceProxy
{
	FSkeletalMeshComponent* SkeletalMeshCom;
	vector<FMatrix> Palette;

	shared_ptr<FAnimSequence> Sequence;
	unordered_map<string, shared_ptr<FAnimSequence>> SequenceMap;
	//FAnimNode_Base* RootNode; // TODO: add animation blend tree to replace Sequence

	float TimePos = 0.0f;

	void AddSequence(std::pair<string, shared_ptr<FAnimSequence>> Seq) { SequenceMap.insert(Seq); }
	vector<FMatrix>& GetPalette() { return Palette; }
	void UpdateAnimation(const float& TotalSeconds);
	vector<FMatrix> UpdatePalette(float dt);
};

class FAnimInstance
{
private:
	FAnimInstanceProxy Proxy;

public:
	void initAnimation(FSkeletalMeshComponent* SkeletalMeshCom); // start timer counting
	void AddSequence(std::pair<string, shared_ptr<FAnimSequence>> Seq) { Proxy.AddSequence(Seq); }
	void UpdateAnimation(const float& TotalSeconds); // invoke in main loop, make character to move

	FAnimInstanceProxy& GetProxy() { return Proxy; }

	FAnimInstance() = default;
	~FAnimInstance() = default;
};
