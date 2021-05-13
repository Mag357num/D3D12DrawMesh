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
	shared_ptr<FScene> LoadStaticMeshActorsCreateScene(const std::wstring& BinFileName); // TODO: divide to two func
	FStaticMeshComponent CreateMeshComponent(uint16 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform);

private:
	vector<FStaticMeshLOD> ReadMeshLODs(std::ifstream& Fin);
	FTransform ReadMeshTransform(std::ifstream& Fin);
};