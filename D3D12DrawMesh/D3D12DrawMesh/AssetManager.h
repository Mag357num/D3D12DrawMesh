#pragma once
#include <unordered_map>
#include <memory>
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
	FStaticMeshComponent CreateMeshComponent(uint16 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform);

private:
	vector<FStaticMeshLOD> ReadMeshLODsFromIfstream(std::ifstream& Fin);
	FTransform ReadMeshTransformFromIfstream(std::ifstream& Fin);
};