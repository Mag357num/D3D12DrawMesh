#include "Skeleton.h"

vector<FMatrix> FSkeleton::GetJointOffset()
{
	vector<FMatrix> Inv;
	vector<FMatrix> GlobalPoseMatrix;
	vector<FMatrix> BindPosesMatrix;
	
	if (BindPoses.size() == 0)
	{
		throw std::exception("skeleton didnt initialize!");
	}

	for (auto i : BindPoses)
	{
		FMatrix S = glm::scale(glm::identity<FMatrix>(), i.Scale);

		FMatrix Q = glm::toMat4(i.Quat);

		FMatrix T = glm::translate(glm::identity<FMatrix>(), i.Translation);

		FMatrix SQT = T * Q * S;
		BindPosesMatrix.push_back(SQT);
	}

	GlobalPoseMatrix.push_back(BindPosesMatrix[0]);

	for (uint32 i = 1; i < Joints.size(); i++)
	{
		GlobalPoseMatrix.push_back(GlobalPoseMatrix[Joints[i].ParentIndex] * BindPosesMatrix[i]);
	}

	for (auto i : GlobalPoseMatrix)
	{
		Inv.push_back(glm::inverse(i));
	}

	return Inv;
}

