#pragma once
#include "stdafx.h"
#include "Joint.h"

using FPose = FTransform;

class FSkeleton
{
private:
	vector<FJoint> Joints;
	vector<FPose> BindPoses;
	

public:
	FSkeleton() = default;
	~FSkeleton() = default;

private:

};

