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
	
	shared_ptr<class ACharacter> CreateCharacter();
	shared_ptr<class TSkeletalMeshComponent> CreateSkeletalMeshComponent();
	shared_ptr<class TSkeletalMesh> CreateSkeletalMesh(const std::wstring& SkeletalMeshFileName);
	shared_ptr<class FSkeleton> CreateSkeleton(const std::wstring& SkeletonFileName);
	shared_ptr<class FAnimSequence> CreateAnimSequence(const std::wstring& SequenceFileName);
	
	//TODO: regard component as a basic functional unit, so change this func to LoadStaticMeshComponent and CreateScene(StaticMeshActors)
	shared_ptr<FScene> LoadStaticMeshActorsCreateScene(const std::wstring& BinFileName);
	void LoadStaticMeshActors(const std::wstring& BinFileName, vector<TStaticMeshActor>& Actors);
	TStaticMeshComponent CreateStaticMeshComponent(const vector<FStaticVertex>& Vertices, const vector<uint32>& Indices, const FTransform& Transform);
	
	shared_ptr<class TSkeletalMesh> LoadSkeletalMesh(const std::wstring& BinFileName);
	shared_ptr<class FSkeleton> LoadSkeleton(const std::wstring& BinFileName);
	shared_ptr<class FAnimSequence> LoadAnimSequence(const std::wstring& BinFileName);

private:
	vector<FStaticMeshLOD> ReadStaticMeshLODs(std::ifstream& Fin);
	vector<FSkeletalMeshLOD> ReadSkeletalMeshLods(std::ifstream& Fin);
	FTransform ReadTransform(std::ifstream& Fin);
};