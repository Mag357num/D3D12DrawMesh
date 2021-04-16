#pragma once
#include <unordered_map>
#include <memory>
#include "Types.h"
#include "stdafx.h"
#include "FScene.h"
#include "DynamicRHI.h"
#include "RHIResource.h"

class FAssetManager
{
private:
	//std::unordered_map<std::string, std::weak_ptr<class FTextureBinData>> TextureBinDatas;
	//std::unordered_map<std::string, std::weak_ptr<class FStaticMeshRenderData>> StaticMeshRenderDatas;
	//std::unordered_map<std::string, std::weak_ptr<class FScene>> SceneDatas;

public:
	//std::shared_ptr<class FStaticMeshRenderData> LoadStaticMesh(const std::string& AssetPath);
	void LoadMeshesToScene(const std::wstring& BinFileName, FScene* TargetScene);
	//std::shared_ptr<class FTextureBinData> LoadTexture(const std::string& AssetPath);

private:
	void ReadMeshLODFromIfstream(std::ifstream& Fin, FMeshLOD& MeshLOD);
	void ReadMeshTransFromIfstream(std::ifstream& Fin, FTransform& Trans);
};