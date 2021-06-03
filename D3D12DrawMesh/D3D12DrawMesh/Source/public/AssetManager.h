#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "DynamicRHI.h"
#include "RHIResource.h"

class FAssetManager
{
private:
	static shared_ptr<FAssetManager> GAssetManager;

public:
	static FAssetManager* Get();
	static void CreateAssetManager();
	static void DestroyAssetManager();

	//TODO: regard component as a basic functional unit, so change this func to LoadStaticMeshComponent and CreateScene(StaticMeshActors)
	void LoadStaticMeshActors(const std::wstring& BinFileName, vector<shared_ptr<AStaticMeshActor>>& Actors);
	FStaticMeshComponent CreateStaticMeshComponent(const vector<FStaticVertex>& Vertices, const vector<uint32>& Indices, const FTransform& Transform);
	
	shared_ptr<class FSkeletalMesh> LoadSkeletalMesh(const std::wstring& BinFileName);
	shared_ptr<class FSkeleton> LoadSkeleton(const std::wstring& BinFileName);
	shared_ptr<class FAnimSequence> LoadAnimSequence(const std::wstring& BinFileName);

private:
	vector<FStaticMeshLOD> ReadStaticMeshLODs(std::ifstream& Fin);
	vector<FSkeletalMeshLOD> ReadSkeletalMeshLods(std::ifstream& Fin);
	FTransform ReadTransform(std::ifstream& Fin);
};