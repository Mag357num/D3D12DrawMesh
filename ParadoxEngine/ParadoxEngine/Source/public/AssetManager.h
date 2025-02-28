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

	void LoadStaticMeshActors(const std::wstring& BinFileName, vector<shared_ptr<AStaticMeshActor>>& Actors);
	shared_ptr<FStaticMesh> LoadStaticMesh(const std::wstring& BinFileName);

	shared_ptr<class FSkeletalMesh> LoadSkeletalMesh(const std::wstring& BinFileName);
	shared_ptr<class FSkeleton> LoadSkeleton(const std::wstring& BinFileName);
	shared_ptr<class FAnimSequence> LoadAnimSequence(const std::wstring& BinFileName);

	shared_ptr<RHI::FTexture> LoadTexture(const wstring& TexFileName);

	void InitMaterialShaderMap();

private:
	unordered_map<string, shared_ptr<class FMaterial>> BaseMaterialMap;

	vector<FStaticMeshLOD> ReadStaticMeshLODs(std::ifstream& Fin);
	vector<FSkeletalMeshLOD> ReadSkeletalMeshLods(std::ifstream& Fin);
	FTransform ReadTransform(std::ifstream& Fin);
	vector<shared_ptr<class FMaterialInterface>> ReadMaterials(std::ifstream& Fin);
};