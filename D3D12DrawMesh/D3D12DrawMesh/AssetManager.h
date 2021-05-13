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
	//TODO: regard component as a basic functional unit, so change this func to LoadStaticMeshComponent and CreateScene(StaticMeshActors)
	shared_ptr<FScene> LoadStaticMeshActorsCreateScene(const std::wstring& BinFileName); // TODO: 
	FStaticMeshComponent CreateStaticMeshComponent(uint16 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform);
	
	class shared_ptr<FSkeletalMesh> LoadSkeletalMesh(const std::wstring& BinFileName);

private:
	vector<FStaticMeshLOD> ReadStaticMeshLODs(std::ifstream& Fin);
	vector<FSkeletalMeshLOD> ReadSkeletalMeshLods(std::ifstream& Fin);
	FTransform ReadTransform(std::ifstream& Fin);
};