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
	shared_ptr<TScene> LoadStaticMeshActorsCreateScene(const std::wstring& BinFileName);
	void LoadStaticMeshActors(const std::wstring& BinFileName, vector<AStaticMeshActor>& Actors);
	TStaticMeshComponent CreateStaticMeshComponent(const vector<TStaticVertex>& Vertices, const vector<uint32>& Indices, const FTransform& Transform);
	
	shared_ptr<class TSkeletalMesh> LoadSkeletalMesh(const std::wstring& BinFileName);
	shared_ptr<class TSkeleton> LoadSkeleton(const std::wstring& BinFileName);
	shared_ptr<class FAnimSequence> LoadAnimSequence(const std::wstring& BinFileName);

private:
	vector<TStaticMeshLOD> ReadStaticMeshLODs(std::ifstream& Fin);
	vector<TSkeletalMeshLOD> ReadSkeletalMeshLods(std::ifstream& Fin);
	FTransform ReadTransform(std::ifstream& Fin);
};