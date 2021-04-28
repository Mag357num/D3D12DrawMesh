#include "AssetManager.h"

using RHI::FMesh;
using RHI::FMeshRes;
using RHI::GDynamicRHI;

void FAssetManager::LoadMeshesToScene(const std::wstring& BinFileName, FScene* TargetScene)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	uint32 ActorNum;
	Fin.read((char*)&ActorNum, sizeof(uint32));
	TargetScene->MeshActors.resize(ActorNum);

	for (uint32 i = 0; i < ActorNum; i++)
	{
		TargetScene->MeshActors[i].MeshLODs.resize(1); // TODO: change to the real lod num of mesh
		ReadMeshLODFromIfstream(Fin, TargetScene->MeshActors[i].MeshLODs[0]);
		ReadMeshTransFromIfstream(Fin, TargetScene->MeshActors[i].Transform);
	}

	Fin.close();
}

void FAssetManager::ReadMeshLODFromIfstream(std::ifstream& Fin, FMeshLOD& MeshLOD)
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
	Fin.read((char*)&Trans.Rotator, 3 * sizeof(float));
	Fin.read((char*)&Trans.Scale, 3 * sizeof(float));
}

