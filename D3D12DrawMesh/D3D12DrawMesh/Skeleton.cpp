#include "Skeleton.h"

vector<FMatrix> FSkeleton::GetJointOffset()
{
	vector<FMatrix> Inv;
	vector<FMatrix> ParentToRoot;
	vector<FMatrix> LocalToParent;
	
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
		LocalToParent.push_back(SQT);
	}

	ParentToRoot.push_back(LocalToParent[0]);

	for (uint32 i = 1; i < Joints.size(); i++)
	{
		//ParentToRoot.push_back(LocalToParent[i] * ParentToRoot[Joints[i].ParentIndex]);
		ParentToRoot.push_back(ParentToRoot[Joints[i].ParentIndex] * LocalToParent[i]);
		assert(Joints[i].ParentIndex < i);
	}

	for (auto i : ParentToRoot)
	{
		Inv.push_back(glm::inverse(i));
	}

	return Inv;
}

