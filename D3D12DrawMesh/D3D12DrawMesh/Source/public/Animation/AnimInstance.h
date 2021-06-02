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
	vector<FMatrix> Palette;
	vector<FMatrix> Palette2;
	unordered_map<string, shared_ptr<FAnimSequence>> SequenceMap;
	string CurrentAnimation = "Idle";
	float TimePos = 0.0f;

	std::mutex Mutex;

public:
	void SetCurrentAnim(string Key) { CurrentAnimation = Key; }
	void initAnimation( FSkeletalMeshComponent* SkeCom ) { SkeletalMeshCom = SkeCom; }
	void TickAnimation(const float& TotalSeconds);
	void AddSequence( std::pair<string, shared_ptr<FAnimSequence>> Seq ) { SequenceMap.insert( Seq ); }
	vector<FMatrix> TickPalette( float dt );
	vector<FMatrix>& GetPalette();
	vector<FMatrix>& GetPalette2();

	FAnimInstance() = default;
	~FAnimInstance() = default;
};
