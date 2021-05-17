#include "Skeleton.h"

vector<FMatrix> FSkeleton::GetJointOffset()
{
	vector<FMatrix> Result;

	if (BindPoses.size() == 0)
	{
		throw std::exception("skeleton didnt initialize!");
	}

	for (auto i : BindPoses)
	{
		FMatrix S = glm::identity<FMatrix>();
		S = glm::scale(S, i.Scale);

		FMatrix Q = glm::toMat4(i.Quat);

		FMatrix T = glm::identity<FMatrix>();
		T = glm::scale(T, i.Scale);

		FMatrix SQT = T * Q * S;
		Result.push_back(glm::inverse(SQT));
	}
	return Result;
}

