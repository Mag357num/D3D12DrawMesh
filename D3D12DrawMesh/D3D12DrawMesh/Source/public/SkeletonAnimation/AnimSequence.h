#pragma once
#include "stdafx.h"
#include "Skeleton.h"
#include "Track.h"

class FAnimSequence
{
private:
	TSkeleton* Skeleton;
	float SequenceLength; // Length (in seconds) of this AnimSequence if played back with a speed of 1.0.
	uint32 SampleNum;
	vector<FTrack> Tracks; // one track respondding to one joint
	vector<int> TrackToJointIndexMapTable;

public:
	float& GetSequenceLength() { return SequenceLength; }
	uint32& GetFrameNum() { return SampleNum; }
	vector<FTrack>& GetTracks() { return Tracks; }
	vector<int>& GetTrackToJointIndexMapTable() { return TrackToJointIndexMapTable; }

	vector<FMatrix> FAnimSequence::Interpolate(float t, TSkeleton* Ske, vector<enum class FRetargetType> ReList);

	void SetSkeleton(TSkeleton* Ske) { Skeleton = Ske; }

	FAnimSequence() = default;
	~FAnimSequence() = default;
};
