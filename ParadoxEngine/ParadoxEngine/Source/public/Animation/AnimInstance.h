#pragma once
#include "AnimSequence.h"

// Retarget type determine joint use bindpose SQT data or animation sequence SQT data
// Have a effect on FAnimSequence::Interpolate()
enum class FRetargetType
{
	SKELETON = 0,
	ANIMATION = 1,
};

class FSkeletalMeshComponent;
class FAnimInstance
{
private:
	FSkeletalMeshComponent* SkeletalMeshCom;
	vector<FMatrix> Palette_GameThread; // create two data obj for two thread, otherwise there will be race
	vector<FMatrix> Palette_RenderThread;
	unordered_map<string, shared_ptr<FAnimSequence>> SequenceMap;
	string CurrentAnimation = "Idle";
	float TimePos = 0.0f;

public:
	void SetCurrentAnim(string Key) { CurrentAnimation = Key; }
	void initAnimation( FSkeletalMeshComponent* SkeCom ) { SkeletalMeshCom = SkeCom; }
	void TickAnimation(const float& TotalSeconds);
	void AddSequence( std::pair<string, shared_ptr<FAnimSequence>> Seq ) { SequenceMap.insert( Seq ); }
	vector<FMatrix> TickPalette( float dt );
	vector<FMatrix>& GetPalette_GameThread();
	vector<FMatrix>& GetPalette_RenderThread();

	FAnimInstance() = default;
	~FAnimInstance() = default;
};
