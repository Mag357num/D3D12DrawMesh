#include "AnimSequence.h"

vector<FMatrix> FAnimSequence::Interpolate(float t)
{
	vector<FMatrix> Result;

	float frameLength = SequenceLength / FrameNum;

	for (uint32 i = 0; i < Tracks.size(); i++)
	{
		FMatrix S, Q, T;

		if (Tracks[i].ScaleSamples.size() == 0)
		{
			Tracks[i].ScaleSamples.push_back(FVector(1, 1, 1));
		}

		if (Tracks[i].QuatSamples.size() == 0)
		{
			Tracks[i].QuatSamples.push_back(FQuat(0, 0, 0, 1));
		}

		if (Tracks[i].TranslationSamples.size() == 0)
		{
			Tracks[i].TranslationSamples.push_back(FVector(0, 0, 0));
		}

		// Interpolate
		if (t <= frameLength)
		{
			Q = glm::toMat4(Tracks[i].QuatSamples[0]);
			T = glm::translate(glm::identity<FMatrix>(), Tracks[i].TranslationSamples[0]);
		}
		else if (t > (FrameNum - 1) * frameLength && t <= SequenceLength)
		{

		}
		else
		{

		}
	}

	return Result;
}

