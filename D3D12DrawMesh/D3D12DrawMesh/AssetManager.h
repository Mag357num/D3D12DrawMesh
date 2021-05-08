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
private:
	static FAssetManager* GAssetManager;

public:
	static FAssetManager* Get();
	shared_ptr<FScene> LoadScene(const std::wstring& BinFileName);
	FMeshActor CreateMeshActor(uint32 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform);

private:
	void ReadMeshLODFromIfstream(std::ifstream& Fin, FMeshLOD& MeshLOD);
	void ReadMeshTransFromIfstream(std::ifstream& Fin, FTransform& Trans);
};