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

	// scene
	shared_ptr<class FScene> LoadScene(const std::wstring& BinFileName);

	// mesh
	shared_ptr<FStaticMesh> LoadStaticMesh(const std::wstring& BinFileName);
	shared_ptr<class FSkeletalMesh> LoadSkeletalMesh(const std::wstring& BinFileName);

	// material
	shared_ptr<class FMaterialInterface> LoadMaterial(const string& MaterialFileName);

	// skeleton
	shared_ptr<class FSkeleton> LoadSkeleton(const std::wstring& BinFileName);

	// animation
	shared_ptr<class FAnimSequence> LoadAnimSequence(const std::wstring& BinFileName);

	// texture
	shared_ptr<RHI::FTexture> LoadTexture(const wstring& TexFileName);

	//void InitMaterialShaderMap();

private:
	unordered_map<string, shared_ptr<class FMaterial>> BaseMaterialMap;

	vector<FStaticMeshLOD> ReadStaticMeshLODsInSceneBinary(std::ifstream& Fin);
	vector<FSkeletalMeshLOD> ReadSkeletalMeshLods(std::ifstream& Fin);
	FTransform ReadComponentTransformInSceneBinary(std::ifstream& Fin);
	vector<shared_ptr<class FMaterialInterface>> ReadMaterialInfosInSceneBinary(std::ifstream& Fin);
};