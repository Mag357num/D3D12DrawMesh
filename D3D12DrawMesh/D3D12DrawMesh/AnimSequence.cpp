#include "AnimSequence.h"

vector<FMatrix> FAnimSequence::Interpolate(float t)
{
	vector<FMatrix> Result;

	float frameLength = SequenceLength / (SampleNum - 1); // there are (SampleNum - 1) frames 
	int frameIndex = floor(t / frameLength);
	float lerpPercent = (t - (frameLength * frameIndex)) / frameLength;

	//frameIndex = 12;
	//lerpPercent = 0.5866;

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

		//FVector SLerp = lerpPercent * S2 + (1 - lerpPercent) * S1;
		FVector SLerp = glm::lerp(S1, S2, lerpPercent);
		S = glm::scale(SLerp);

		FQuat QLerp = glm::slerp(Q1, Q2, lerpPercent);
		QLerp = glm::normalize(QLerp);
		Q = glm::toMat4(QLerp);

		FVector TLerp = glm::lerp(T1, T2, lerpPercent);
		T = glm::translate(TLerp);

		//FVector RotateQ1 = glm::eulerAngles( Q1 );
		//RotateQ1 = { glm::degrees(RotateQ1.x ), glm::degrees(RotateQ1.y ), glm::degrees(RotateQ1.z ) };

		//FVector RotateQ2 = glm::eulerAngles(Q2);
		//RotateQ2 = { glm::degrees(RotateQ2.x), glm::degrees(RotateQ2.y), glm::degrees(RotateQ2.z) };

		//FVector RotateQLerp = glm::eulerAngles(QLerp);
		//RotateQLerp = { glm::degrees(RotateQLerp.x), glm::degrees(RotateQLerp.y), glm::degrees(RotateQLerp.z) };



		Result.push_back(T * Q * S);
	}

	return Result;
}

