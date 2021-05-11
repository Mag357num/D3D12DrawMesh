#include "AssetManager.h"

using RHI::FMesh;
using RHI::FMeshRes;
using RHI::GDynamicRHI;

FAssetManager* FAssetManager::GAssetManager = new FAssetManager;

FAssetManager* FAssetManager::Get()
{
	return GAssetManager;
}

shared_ptr<FScene> FAssetManager::LoadScene(const std::wstring& BinFileName)
{
	shared_ptr<FScene> TargetScene = make_shared<FScene>();

	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	uint32 ActorNum;
	Fin.read((char*)&ActorNum, sizeof(uint32));
	TargetScene->GetSceneSMActorArray().resize(ActorNum);

	for (uint32 i = 0; i < ActorNum; i++)
	{
		TargetScene->GetSceneSMActorArray()[i].MeshLODs.resize(1); // TODO: change to the real lod num of mesh
		ReadMeshLODFromIfstream(Fin, TargetScene->GetSceneSMActorArray()[i].MeshLODs[0]);
		ReadMeshTransFromIfstream(Fin, TargetScene->GetSceneSMActorArray()[i].Transform);

		// TODO: add a func to read shader file name, so different mesh can have different shader
		TargetScene->GetSceneSMActorArray()[i].ShaderFileName = L"Shadow_SceneColor.hlsl";
	}

	// TODO: hard code
	TargetScene->GetSceneSMActorArray()[6].ShaderFileName = L"Shadow_SceneColor_Sun.hlsl";

	Fin.close();

	return TargetScene;
}

void FAssetManager::ReadMeshLODFromIfstream(std::ifstream& Fin, FStaticMeshLOD& MeshLOD)
{
	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	Fin.read((char*)&MeshLOD.GetVertexStride(), sizeof(int));

	uint32 BufferByteSize;
	uint32 BufferElementSize;
	Fin.read((char*)&BufferElementSize, sizeof(int));
	BufferByteSize = BufferElementSize * MeshLOD.GetVertexStride();

	float VerticeSize = static_cast<float>(BufferByteSize) / sizeof(float);
	assert(VerticeSize - floor(VerticeSize) == 0);
	MeshLOD.ResizeVertices(static_cast<int>(BufferByteSize / sizeof(float)));
	Fin.read((char*)MeshLOD.GetVertices().data(), BufferByteSize);

	Fin.read((char*)&BufferElementSize, sizeof(int));
	BufferByteSize = BufferElementSize * sizeof(int);

	MeshLOD.ResizeIndices(BufferElementSize);
	Fin.read((char*)MeshLOD.GetIndices().data(), BufferByteSize);
}

void FAssetManager::ReadMeshTransFromIfstream(std::ifstream& Fin, FTransform& Trans)
{
	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	Fin.read((char*)&Trans.Translation, 3 * sizeof(float));
	Fin.read((char*)&Trans.Rotation, 3 * sizeof(float));
	Fin.read((char*)&Trans.Scale, 3 * sizeof(float));
}

FStaticMeshActor FAssetManager::CreateMeshActor(uint16 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform)
{
	FStaticMeshActor Actor;
	Actor.MeshLODs.resize(1); // TODO: only consider one mip
	Actor.MeshLODs[0].SetVertexStride(VertexStride);
	Actor.MeshLODs[0].SetVertices(Vertices);
	Actor.MeshLODs[0].SetIndices(Indices);
	Actor.Transform.Translation = Transform.Translation;
	Actor.Transform.Rotation = Transform.Rotation;
	Actor.Transform.Scale = Transform.Scale;
	return Actor;
}

