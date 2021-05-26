#include "AnimSequence.h"
#include "AnimInstance.h"

vector<FMatrix> FAnimSequence::Interpolate(float t, FSkeleton* Ske, vector<FRetargetType> ReList)
{
	vector<FMatrix> Result;
	vector<FPose>& BindPoses = Ske->GetBindPoses();

	float FrameLength = SequenceLength / (SampleNum - 1); // there are (SampleNum - 1) frames 
	int FrameIndex = floor(t / FrameLength);
	float LerpPercent = (t - (FrameLength * FrameIndex)) / FrameLength;

	FVector S1, S2, T1, T2;
	FQuat Q1, Q2;
	FMatrix MS, MQ, MT;
	for (uint32 i = 0; i < Tracks.size(); i++)
	{
		// Interpolate
		if (Tracks[i].ScaleSamples.size() == 0)
		{
			S1 = FVector(1, 1, 1);
			S2 = S1;
		}
		else if (Tracks[i].ScaleSamples.size() == 1)
		{
			S1 = Tracks[i].ScaleSamples[0];
			S2 = S1;
		}
		else
		{
			S1 = Tracks[i].ScaleSamples[FrameIndex];
			S2 = Tracks[i].ScaleSamples[FrameIndex + 1];
		}

		if (Tracks[i].QuatSamples.size() == 0)
		{
			Q1 = FQuat(0, 0, 0, 1);
			Q2 = Q1;
		}
		else if (Tracks[i].QuatSamples.size() == 1)
		{
			Q1 = Tracks[i].QuatSamples[0];
			Q2 = Q1;
		}
		else
		{
			Q1 = Tracks[i].QuatSamples[FrameIndex];
			Q2 = Tracks[i].QuatSamples[FrameIndex + 1];
		}

		if (ReList[i] == FRetargetType::ANIMATION)
		{
			if (Tracks[i].TranslationSamples.size() == 0)
			{
				T1 = FVector(0, 0, 0);
				T2 = T1;
			}
			else if (Tracks[i].TranslationSamples.size() == 1)
			{
				T1 = Tracks[i].TranslationSamples[0];
				T2 = T1;
			}
			else
			{
				T1 = Tracks[i].TranslationSamples[FrameIndex];
				T2 = Tracks[i].TranslationSamples[FrameIndex + 1];
			}
		}
		else if (ReList[i] == FRetargetType::SKELETON)
		{
			T1 = BindPoses[i].Translation;
			T2 = T1;
		}

		MS = scale(lerp(S1, S2, LerpPercent));
		MQ = toMat4(normalize(slerp(Q1, Q2, LerpPercent)));
		MT = translate(lerp(T1, T2, LerpPercent));

		Result.push_back(MT * MQ * MS);
	}

	return Result;
}

