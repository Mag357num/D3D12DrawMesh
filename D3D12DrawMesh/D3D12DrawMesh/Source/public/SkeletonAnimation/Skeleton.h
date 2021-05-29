#pragma once
#include "stdafx.h"
#include "Joint.h"

using FPose = FTransform;

class TSkeleton
{
private:
	vector<FJoint> Joints;
	vector<FPose> BindPoses;
	unordered_map<string, int> NameToIndexMap;

public:
	vector<FJoint>& GetJoints() { return Joints; }
	vector<FPose>& GetBindPoses() { return BindPoses; }
	vector<FMatrix> GetJointOffset(); // Bind Pose inverse matrix: to transform vertex into joint space
	unordered_map<string, int>& GetNameToIndexMap() { return NameToIndexMap; }

	TSkeleton() = default;
	~TSkeleton() = default;
};
