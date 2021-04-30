#pragma once
#include <unordered_map>
#include <memory>
#include "Types.h"
#include "stdafx.h"
#include "FScene.h"
#include "DynamicRHI.h"
#include "RHIResource.h"

class FAssetManager
{
public:
	void LoadScene(const std::wstring& BinFileName, FScene* TargetScene);

private:
	void ReadMeshLODFromIfstream(std::ifstream& Fin, FMeshLOD& MeshLOD);
	void ReadMeshTransFromIfstream(std::ifstream& Fin, FTransform& Trans);
};