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
	shared_ptr<class FMaterialInterface> LoadMaterialInstance(const string& MaterialFileName);

	// skeleton
	shared_ptr<class FSkeleton> LoadSkeleton(const std::wstring& BinFileName);

	// animation
	shared_ptr<class FAnimSequence> LoadAnimSequence(const std::wstring& BinFileName);

	// texture
	shared_ptr<RHI::FTexture> LoadTexture(const wstring& TexFileName);

	//void InitMaterialShaderMap();

private:
	unordered_map<string, shared_ptr<class FMaterial>> BaseMaterialMap;
	unordered_map<string, shared_ptr<class FMaterialInstance>> MaterialInstanceMap;

	shared_ptr<class FCameraComponent> ReadCameraComponent(std::ifstream& Fin);
	shared_ptr<class FDirectionalLightComponent> ReadDLightComponent(std::ifstream& Fin);
	shared_ptr<class FPointLightComponent> ReadPLightComponent(std::ifstream& Fin);
	shared_ptr<FStaticMeshComponent> ReadStaticMeshComponent(std::ifstream& Fin);

	vector<shared_ptr<FStaticMeshLOD>> ReadStaticMeshLODsInSceneBinary(std::ifstream& Fin);
	vector<FSkeletalMeshLOD> ReadSkeletalMeshLods(std::ifstream& Fin);
	vector<shared_ptr<class FMaterialInterface>> ReadMaterialInfosInSceneBinary(std::ifstream& Fin);
};