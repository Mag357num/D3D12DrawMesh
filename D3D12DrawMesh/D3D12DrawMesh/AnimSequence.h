#pragma once
#include "stdafx.h"
#include "Skeleton.h"
#include "Track.h"

class FAnimSequence
{
private:
	FSkeleton* Skeleton;
	float SequenceLength; // Length (in seconds) of this AnimSequence if played back with a speed of 1.0.
	uint32 FrameNum;
	vector<FTrack> Tracks; // one track respondding to one joint
	vector<int> TrackToJointIndexMapTable;

public:
	float& GetSequenceLength() { return SequenceLength; }
	uint32& GetFrameNum() { return FrameNum; }
	vector<FTrack>& GetTracks() { return Tracks; }
	vector<int>& GetTrackToJointIndexMapTable() { return TrackToJointIndexMapTable; }

	FAnimSequence() = default;
	~FAnimSequence() = default;
};
