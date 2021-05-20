#include "AnimSequence.h"

vector<FMatrix> FAnimSequence::Interpolate(float t)
{
	vector<FMatrix> Result;

	float frameLength = SequenceLength / (SampleNum - 1); // there are (SampleNum - 1) frames 
	int frameIndex = floor(t / frameLength);
	//frameIndex = 0;
	float lerpPercent = (t - (frameLength * frameIndex)) / frameLength;
	//lerpPercent = 0;

	for (uint32 i = 0; i < Tracks.size(); i++)
	{
		FVector S1, S2, T1, T2;
		FQuat Q1, Q2;

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
			S1 = Tracks[i].ScaleSamples[frameIndex];
			S2 = Tracks[i].ScaleSamples[frameIndex + 1];
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
			Q1 = Tracks[i].QuatSamples[frameIndex];
			Q2 = Tracks[i].QuatSamples[frameIndex + 1];
		}

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
			T1 = Tracks[i].TranslationSamples[frameIndex];
			T2 = Tracks[i].TranslationSamples[frameIndex + 1];
		}

		FMatrix S, Q, T;

		FVector SLerp = lerpPercent * S2 + (1 - lerpPercent) * S1;
		S = glm::scale(SLerp);

		FQuat QLerp = glm::lerp(Q1, Q2, lerpPercent);
		Q = glm::toMat4(QLerp);

		FVector TLerp = lerpPercent * T2 + (1 - lerpPercent) * T1;
		T = glm::translate(TLerp);

		//FVector Rotate = glm::eulerAngles(QLerp);
		//FVector Rotate2 = { glm::degrees(Rotate.x), glm::degrees(Rotate.y), glm::degrees(Rotate.z) };

		//if (i == 45) // lowarm_twist_01_r
		//{
		//	int a = 10;
		//}

		//if (i == 55) // thigh_r
		//{
		//	int a = 10;
		//}

		//if (i == 60) // thigh_twist_01_r
		//{
		//	int a = 10;
		//}

		//if (i == 56) // calf_r
		//{
		//	int a = 10;
		//}

		Result.push_back(T * Q * S);
	}

	return Result;
}

