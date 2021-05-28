#include "AssetManager.h"
#include "AnimSequence.h"
#include "Character.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"

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
		TStaticMeshActor Actor;
		shared_ptr<TStaticMeshComponent> Com = make_shared<TStaticMeshComponent>();
		Com->GetStaticMesh()->SetMeshLODs(ReadStaticMeshLODs(Fin));
		Com->SetTransform(ReadTransform(Fin));

		Actor.SetStaticMeshComponent(Com);
		TargetScene->AddStaticMeshActor(Actor);
	}

	Fin.close();

	return TargetScene;
}

void FAssetManager::LoadStaticMeshActors(const std::wstring& BinFileName, vector<TStaticMeshActor>& Actors)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	uint32 ActorNum;
	Fin.read((char*)&ActorNum, sizeof(uint32));

	for (uint32 i = 0; i < ActorNum; i++)
	{
		TStaticMeshActor Actor;
		shared_ptr<TStaticMeshComponent> Com = make_shared<TStaticMeshComponent>();
		shared_ptr<TStaticMesh> SM = make_shared<TStaticMesh>();
		Com->SetStaticMesh(SM);
		Com->GetStaticMesh()->SetMeshLODs(ReadStaticMeshLODs(Fin));
		Com->SetTransform(ReadTransform(Fin));

		Actor.SetStaticMeshComponent(Com);
		Actors.push_back(Actor);
	}

	Fin.close();
}

vector<FStaticMeshLOD> FAssetManager::ReadStaticMeshLODs(std::ifstream& Fin)
{
	FStaticMeshLOD MeshLOD;

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	uint32 VertexStride;
	Fin.read((char*)&VertexStride, sizeof(int));

	uint32 BufferByteSize;
	uint32 VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * VertexStride;

	float VerticeSize = static_cast<float>(BufferByteSize) / sizeof(float);
	assert(VerticeSize - floor(VerticeSize) == 0);
	MeshLOD.ResizeVertices(static_cast<int>(BufferByteSize / sizeof(float)));
	Fin.read((char*)MeshLOD.GetVertices2().data(), BufferByteSize);

	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * sizeof(int);

	MeshLOD.ResizeIndices(VertexNum);
	Fin.read((char*)MeshLOD.GetIndices().data(), BufferByteSize);

	vector<FStaticMeshLOD> MeshLODs;
	MeshLODs.push_back(MeshLOD);
	return MeshLODs;
}

vector<FSkeletalMeshLOD> FAssetManager::ReadSkeletalMeshLods(std::ifstream& Fin)
{
	FSkeletalMeshLOD MeshLOD;

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	uint32 VertexStride;
	Fin.read((char*)&VertexStride, sizeof(int));

	uint32 BufferByteSize;
	uint32 VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * VertexStride;

	MeshLOD.GetVertice().resize(VertexNum);
	Fin.read((char*)MeshLOD.GetVertice().data(), BufferByteSize);

	// WeightVertex
	{
		Fin.read((char*)&VertexNum, sizeof(int)); // vertex num
		MeshLOD.GetWeightVertice().resize(VertexNum);
		for (uint32 i = 0; i < VertexNum; i++)
		{
			uint32 IndiceNum, WeightsNum; // weight num of each vertex

			Fin.read((char*)&IndiceNum, sizeof(int));
			MeshLOD.GetWeightVertice()[i].GetJointIndice().resize(IndiceNum);
			Fin.read((char*)MeshLOD.GetWeightVertice()[i].GetJointIndice().data(), IndiceNum * sizeof(int16_t));

			Fin.read((char*)&WeightsNum, sizeof(int));
			MeshLOD.GetWeightVertice()[i].GetJointWeights().resize(WeightsNum);
			Fin.read((char*)MeshLOD.GetWeightVertice()[i].GetJointWeights().data(), WeightsNum * sizeof(int8_t));
		}
	}

	uint32 IndiceNum;
	Fin.read((char*)&IndiceNum, sizeof(int));
	BufferByteSize = IndiceNum * sizeof(int);

	MeshLOD.GetIndice().resize(IndiceNum);
	Fin.read((char*)MeshLOD.GetIndice().data(), BufferByteSize);

	for (uint32 i = 0; i < VertexNum; i++)
	{
		array<uint16, 4> JointWeight;
		array<uint16, 4> JointIndice;

		for (uint32 j = 0; j < 4; j++)
		{
			JointWeight[j] = static_cast<uint16>(MeshLOD.GetWeightVertice()[i].GetJointWeights()[j]);
			JointIndice[j] = MeshLOD.GetWeightVertice()[i].GetJointIndice()[j];
		}

		FSkeletalVertex Vertex =
		{ 
			MeshLOD.GetVertice()[i].Pos,
			MeshLOD.GetVertice()[i].Nor,
			MeshLOD.GetVertice()[i].UV0,
			MeshLOD.GetVertice()[i].Color,
			JointWeight,
			JointIndice,
		};
		MeshLOD.GetSkeletalVertice().push_back(Vertex);
	}

	vector<FSkeletalMeshLOD> MeshLODs;
	MeshLODs.push_back(MeshLOD);
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


TStaticMeshComponent FAssetManager::CreateStaticMeshComponent(const vector<FStaticVertex>& Vertices, const vector<uint32>& Indices, const FTransform& Transform)
{
	TStaticMeshComponent Component;
	vector<FStaticMeshLOD> Lods;
	Lods.push_back(FStaticMeshLOD(Vertices, Indices));
	shared_ptr<TStaticMesh> StaticMesh = make_shared<TStaticMesh>();
	StaticMesh->SetMeshLODs(Lods);
	Component.SetStaticMesh(StaticMesh);
	Component.SetTransform(Transform);
	return Component;
}

shared_ptr<TSkeletalMesh> FAssetManager::LoadSkeletalMesh(const std::wstring& BinFileName)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	shared_ptr<TSkeletalMesh> SkeMesh = make_shared<TSkeletalMesh>();

	SkeMesh->SetSkeletalMeshLods(ReadSkeletalMeshLods(Fin));

	Fin.close();

	return SkeMesh;
}

shared_ptr<FSkeleton> FAssetManager::LoadSkeleton(const std::wstring& BinFileName)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	shared_ptr<FSkeleton> Ske = make_shared<FSkeleton>();

	uint32 JointNum;
	Fin.read((char*)&JointNum, sizeof(int));
	Ske->GetJoints().resize(JointNum);
	for (uint32 i = 0; i < JointNum; i++)
	{
		int CharNum;
		Fin.read((char*)&CharNum, sizeof(int));
		Ske->GetJoints()[i].JointName.resize(CharNum);
		Fin.read((char*)Ske->GetJoints()[i].JointName.data(), CharNum * sizeof(char)); // UE4 FString will output char string not wchar wstring

		Fin.read((char*)&Ske->GetJoints()[i].ParentIndex, sizeof(int));
	}

	Fin.read((char*)&JointNum, sizeof(int));
	Ske->GetBindPoses().resize(JointNum);
	for (uint32 i = 0; i < JointNum; i++)
	{
		Fin.read((char*)&Ske->GetBindPoses()[i].Quat, 4 * sizeof(float));
		Fin.read((char*)&Ske->GetBindPoses()[i].Translation, 3 * sizeof(float));
		Fin.read((char*)&Ske->GetBindPoses()[i].Scale, 3 * sizeof(float));
	}

	vector<string> JointName;
	int ElementNum;
	Fin.read((char*)&ElementNum, sizeof(int));
	JointName.resize(ElementNum);
	for (uint32 i = 0; i < ElementNum; i++)
	{
		int CharNum;
		int JointIndex;
		// read FString
		Fin.read((char*)&CharNum, sizeof(int));
		JointName[i].resize(CharNum);
		Fin.read((char*)JointName[i].data(), CharNum * sizeof(char));
		// read int32
		Fin.read((char*)&JointIndex, sizeof(int));
		Ske->GetNameToIndexMap().insert({ JointName[i], JointIndex});
	}

	Fin.close();

	return Ske;
}

shared_ptr<FAnimSequence> FAssetManager::LoadAnimSequence(const std::wstring& BinFileName)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	shared_ptr<FAnimSequence> Seq = make_shared<FAnimSequence>();

	Fin.read((char*)&Seq->GetSequenceLength(), sizeof(float));
	Fin.read((char*)&Seq->GetFrameNum(), sizeof(uint32));

	uint32 TracksNum;
	Fin.read((char*)&TracksNum, sizeof(int));
	Seq->GetTracks().resize(TracksNum);
	for (uint32 i = 0; i < TracksNum; i++)
	{
		uint32 ScalesNum;
		uint32 QuatsNum;
		uint32 TranslationsNum;
		Fin.read((char*)&ScalesNum, sizeof(int));
		Seq->GetTracks()[i].ScaleSamples.resize(ScalesNum);
		Fin.read((char*)Seq->GetTracks()[i].ScaleSamples.data(), ScalesNum * 3 * sizeof(float));
		if (ScalesNum == 0)
		{
			Seq->GetTracks()[i].ScaleSamples.push_back(FVector(1, 1, 1));
		}

		Fin.read((char*)&QuatsNum, sizeof(int));
		Seq->GetTracks()[i].QuatSamples.resize(QuatsNum);
		Fin.read((char*)Seq->GetTracks()[i].QuatSamples.data(), QuatsNum * 4 * sizeof(float));
		if (QuatsNum == 0)
		{
			Seq->GetTracks()[i].QuatSamples.push_back(FQuat(0, 0, 0, 1));
		}

		Fin.read((char*)&TranslationsNum, sizeof(int));
		Seq->GetTracks()[i].TranslationSamples.resize(TranslationsNum);
		Fin.read((char*)Seq->GetTracks()[i].TranslationSamples.data(), TranslationsNum * 3 * sizeof(float));
		if (TranslationsNum == 0)
		{
			Seq->GetTracks()[i].TranslationSamples.push_back(FVector(0, 0, 0));
		}
	}

	uint32 IndexTableNum;
	Fin.read((char*)&IndexTableNum, sizeof(int));
	Seq->GetTrackToJointIndexMapTable().resize(IndexTableNum);
	Fin.read((char*)Seq->GetTrackToJointIndexMapTable().data(), IndexTableNum * sizeof(int));

	Fin.close();
	
	return Seq;
}

shared_ptr<class ACharacter> FAssetManager::CreateCharacter()
{
	return make_shared<ACharacter>();
}

shared_ptr<class TSkeletalMeshComponent> FAssetManager::CreateSkeletalMeshComponent()
{
	return make_shared<TSkeletalMeshComponent>();
}

shared_ptr<class TSkeletalMesh> FAssetManager::CreateSkeletalMesh(const std::wstring& SkeletalMeshFileName)
{
	return FAssetManager::Get()->LoadSkeletalMesh(SkeletalMeshFileName);
}

shared_ptr<class FSkeleton> FAssetManager::CreateSkeleton(const std::wstring& SkeletonFileName)
{
	return FAssetManager::Get()->LoadSkeleton(SkeletonFileName);
}

shared_ptr<class FAnimSequence> FAssetManager::CreateAnimSequence(const std::wstring& SequenceFileName)
{
	return FAssetManager::Get()->LoadAnimSequence(SequenceFileName);
}