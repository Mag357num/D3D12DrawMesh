#pragma once
#include <unordered_map>
#include <memory>
#include "Types.h"
#include "stdafx.h"
#include "Scene.h"
#include "DynamicRHI.h"
#include "RHIResource.h"

class FAssetManager
{
private:
	static FAssetManager* GAssetManager;

public:
	static FAssetManager* Get();
	shared_ptr<FScene> LoadScene(const std::wstring& BinFileName);
	FStaticMeshActor CreateMeshActor(uint16 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform);

private:
	void ReadMeshLODFromIfstream(std::ifstream& Fin, FStaticMeshLOD& MeshLOD);
	void ReadMeshTransFromIfstream(std::ifstream& Fin, FTransform& Trans);
};