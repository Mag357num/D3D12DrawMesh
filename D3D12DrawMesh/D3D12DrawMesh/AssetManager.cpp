#include "AssetManager.h"

using RHI::FMesh;
using RHI::FMeshRes;
using RHI::GDynamicRHI;

FAssetManager* FAssetManager::GAssetManager = new FAssetManager;

FAssetManager* FAssetManager::Get()
{
	return GAssetManager;
}

shared_ptr<FScene> FAssetManager::LoadStaticMeshActorsCreateScene(const std::wstring& BinFileName)
{
	shared_ptr<FScene> TargetScene = make_shared<FScene>();

	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	uint32 ActorNum;
	Fin.read((char*)&ActorNum, sizeof(uint32));

	for (uint32 i = 0; i < ActorNum; i++)
	{
		AStaticMeshActor Actor;
		shared_ptr<FStaticMeshComponent> Com = make_shared<FStaticMeshComponent>();

		Com->GetStaticMesh().SetMeshLODs(ReadStaticMeshLODs(Fin));
		Com->SetTransform(ReadTransform(Fin));

		// TODO: add a func to read shader file name, so different mesh can have different shader
		if (i == 6) // TODO: hard coding
		{
			Com->SetShaderFileName(L"Shadow_SceneColor_Sun.hlsl");
		}
		else
		{
			Com->SetShaderFileName(L"Shadow_SceneColor.hlsl");
		}

		Actor.GetComs().push_back(Com);
		TargetScene->GetStaticMeshActors().push_back(Actor);
	}

	Fin.close();

	return TargetScene;
}

vector<FStaticMeshLOD> FAssetManager::ReadStaticMeshLODs(std::ifstream& Fin)
{
	FStaticMeshLOD MeshLOD;

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	Fin.read((char*)&MeshLOD.GetVertexStride(), sizeof(int));

	uint32 BufferByteSize;
	uint32 VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * MeshLOD.GetVertexStride();

	float VerticeSize = static_cast<float>(BufferByteSize) / sizeof(float);
	assert(VerticeSize - floor(VerticeSize) == 0);
	MeshLOD.ResizeVertices(static_cast<int>(BufferByteSize / sizeof(float)));
	Fin.read((char*)MeshLOD.GetVertices().data(), BufferByteSize);

	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * sizeof(int);

	MeshLOD.ResizeIndices(VertexNum);
	Fin.read((char*)MeshLOD.GetIndices().data(), BufferByteSize);

	vector<FStaticMeshLOD> MeshLODs;
	MeshLODs.push_back(MeshLOD); // TODO: default consider there is only one lod
	return MeshLODs;
}

vector<FSkeletalMeshLOD> FAssetManager::ReadSkeletalMeshLods(std::ifstream& Fin)
{
	FSkeletalMeshLOD MeshLOD;

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	Fin.read((char*)&MeshLOD.GetVertexStride(), sizeof(int));

	uint32 BufferByteSize;
	uint32 VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * MeshLOD.GetVertexStride();

	MeshLOD.ResizeVertices(VertexNum);
	Fin.read((char*)MeshLOD.GetVertices().data(), BufferByteSize);

	// WeightVertex
	{
		Fin.read((char*)&VertexNum, sizeof(int)); // vertex num
		MeshLOD.GetWeightVertices().resize(VertexNum);
		for (uint32 i = 0; i < VertexNum; i++)
		{
			uint32 WeightsNum; // weight num of each vertex
			Fin.read((char*)&WeightsNum, sizeof(int));
			MeshLOD.GetWeightVertices()[i].GetJointIndice().resize(WeightsNum);
			MeshLOD.GetWeightVertices()[i].GetJointWeights().resize(WeightsNum);
			Fin.read((char*)MeshLOD.GetWeightVertices()[i].GetJointIndice().data(), WeightsNum * sizeof(int16_t));
			Fin.read((char*)MeshLOD.GetWeightVertices()[i].GetJointWeights().data(), WeightsNum * sizeof(int8_t));
		}
	}

	uint32 IndiceNum;
	Fin.read((char*)&IndiceNum, sizeof(int));
	BufferByteSize = IndiceNum * sizeof(int);

	MeshLOD.ResizeIndices(IndiceNum);
	Fin.read((char*)MeshLOD.GetIndices().data(), BufferByteSize);

	vector<FSkeletalMeshLOD> MeshLODs;
	MeshLODs.push_back(MeshLOD); // TODO: default consider there is only one lod
	return MeshLODs;
}

FTransform FAssetManager::ReadTransform(std::ifstream& Fin)
{
	FTransform Trans;
	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	Fin.read((char*)&Trans.Translation, 3 * sizeof(float));
	Fin.read((char*)&Trans.Quat, 4 * sizeof(float));
	Fin.read((char*)&Trans.Scale, 3 * sizeof(float));

	return Trans;
}

FStaticMeshComponent FAssetManager::CreateStaticMeshComponent(uint16 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform)
{
	FStaticMeshComponent Component;
	vector<FStaticMeshLOD> Lods;
	Lods.push_back(FStaticMeshLOD(VertexStride, Vertices, Indices));
	UStaticMesh StaticMesh;
	StaticMesh.SetMeshLODs(Lods);
	Component.SetStaticMesh(StaticMesh);
	Component.SetTransform(Transform);
	return Component;
}

class shared_ptr<FSkeletalMesh> FAssetManager::LoadSkeletalMesh(const std::wstring& BinFileName)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	shared_ptr<FSkeletalMesh> SkeMesh = make_shared<FSkeletalMesh>();

	SkeMesh->SetSkeletalMeshLods(ReadSkeletalMeshLods(Fin));

	return SkeMesh;
}
