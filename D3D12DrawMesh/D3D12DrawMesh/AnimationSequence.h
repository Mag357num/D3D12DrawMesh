#pragma once
#include "stdafx.h"
#include "Skeleton.h"
#include "Track.h"

class FAnimationSequence
{
private:
	FSkeleton* Skeleton;
	uint32 FrameNum;
	vector<FTrack> Tracks; // one track respondding to one joint
	unordered_map<string, uint16> TrackToJointIndexMap;

public:
	FAnimationSequence() = default;
	~FAnimationSequence() = default;

private:

};

