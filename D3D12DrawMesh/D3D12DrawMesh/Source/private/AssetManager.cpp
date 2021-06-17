#include "AssetManager.h"
#include "AnimSequence.h"
#include "Character.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"
#include "Material.h"
#include "FrameResourceManager.h"
#include "RHIResource.h"

using RHI::GDynamicRHI;

shared_ptr<FAssetManager> FAssetManager::GAssetManager = nullptr;

FAssetManager* FAssetManager::Get()
{
	return GAssetManager.get();
}

void FAssetManager::CreateAssetManager()
{
	assert(GAssetManager == nullptr);
	GAssetManager = make_shared<FAssetManager>();
}

void FAssetManager::DestroyAssetManager()
{
	assert(GAssetManager != nullptr);
	GAssetManager = nullptr;
}

shared_ptr<class FScene> FAssetManager::LoadScene(const std::wstring& BinFileName)
{
	shared_ptr<FScene> Scene = make_shared<FScene>();
	vector<shared_ptr<AStaticMeshActor>>& Actors = Scene->GetStaticMeshActors();

	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("ERROR: open file faild.");
	}

	uint32 SceneActorNum;
	Fin.read((char*)&SceneActorNum, sizeof(uint32));

	for (uint32 i = 0; i < SceneActorNum; i++)
	{
		// actor name
		string ActorName;
		{
			int CharNum;
			Fin.read((char*)&CharNum, sizeof(int));
			ActorName.resize(CharNum);
			Fin.read((char*)ActorName.data(), CharNum * sizeof(char));
			ActorName = ActorName.c_str();
		}

		// static mesh components
		{
			int Num;
			Fin.read((char*)&Num, sizeof(int));
			for (uint32 j = 0; j < Num; j++)
			{
				shared_ptr<FStaticMeshComponent> Com = make_shared<FStaticMeshComponent>();
				shared_ptr<FStaticMesh> SM = make_shared<FStaticMesh>();
				Com->SetStaticMesh(SM);
				Com->GetStaticMesh()->SetMeshLODs(ReadStaticMeshLODsInSceneBinary(Fin));
				Com->SetTransform(ReadComponentTransformInSceneBinary(Fin));
				Com->SetMaterials(ReadMaterialInfosInSceneBinary(Fin));
			}
		}

		// camera components
		{
			int Num;
			Fin.read((char*)&Num, sizeof(int));
			for (uint32 j = 0; j < Num; j++)
			{
				//shared_ptr<FCameraComponent> Com = make_shared<FStaticMeshComponent>();
			}
		}

	}

	Fin.close();
	return Scene;
}

shared_ptr<FStaticMesh> FAssetManager::LoadStaticMesh(const std::wstring& BinFileName)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	shared_ptr<FStaticMesh> SM = make_shared<FStaticMesh>();
	SM->SetMeshLODs(ReadStaticMeshLODsInSceneBinary(Fin));
	return SM;
}

vector<FStaticMeshLOD> FAssetManager::ReadStaticMeshLODsInSceneBinary(std::ifstream& Fin)
{
	FStaticMeshLOD MeshLOD;

	if (!Fin.is_open())
	{
		throw std::exception("ERROR: open file faild.");
	}

	uint32 VertexStride;
	Fin.read((char*)&VertexStride, sizeof(int));

	uint32 BufferByteSize;
	uint32 VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * VertexStride;

	float VerticeSize = static_cast<float>(BufferByteSize) / sizeof(float);
	assert(VerticeSize - floor(VerticeSize) == 0);
	MeshLOD.Vertice.resize(static_cast<int>(BufferByteSize / sizeof(float)));
	Fin.read((char*)MeshLOD.Vertice.data(), BufferByteSize);

	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * sizeof(int);

	MeshLOD.Indice.resize(VertexNum);
	Fin.read((char*)MeshLOD.Indice.data(), BufferByteSize);

	vector<FStaticMeshLOD> MeshLODs;
	MeshLODs.push_back(MeshLOD);
	return MeshLODs;
}

vector<FSkeletalMeshLOD> FAssetManager::ReadSkeletalMeshLods(std::ifstream& Fin)
{
	FSkeletalMeshLOD MeshLOD;

	if (!Fin.is_open())
	{
		throw std::exception("ERROR:  open file faild.");
	}

	uint32 VertexStride;
	Fin.read((char*)&VertexStride, sizeof(int));

	uint32 BufferByteSize;
	uint32 VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	BufferByteSize = VertexNum * VertexStride;

	vector<FStaticVertex> StaticVertice;
	StaticVertice.resize(VertexNum);
	Fin.read((char*)StaticVertice.data(), BufferByteSize);

	// WeightVertex
	{
		Fin.read((char*)&VertexNum, sizeof(int)); // vertex num
		MeshLOD.SkinnedWeightVertexArray.resize(VertexNum);
		for (uint32 i = 0; i < VertexNum; i++)
		{
			uint32 IndiceNum, WeightsNum; // weight num of each vertex

			Fin.read((char*)&IndiceNum, sizeof(int));
			MeshLOD.SkinnedWeightVertexArray[i].GetJointIndice().resize(IndiceNum);
			Fin.read((char*)MeshLOD.SkinnedWeightVertexArray[i].GetJointIndice().data(), IndiceNum * sizeof(int16_t));

			Fin.read((char*)&WeightsNum, sizeof(int));
			MeshLOD.SkinnedWeightVertexArray[i].GetJointWeights().resize(WeightsNum);
			Fin.read((char*)MeshLOD.SkinnedWeightVertexArray[i].GetJointWeights().data(), WeightsNum * sizeof(int8_t));
		}
	}

	uint32 IndiceNum;
	Fin.read((char*)&IndiceNum, sizeof(int));
	BufferByteSize = IndiceNum * sizeof(int);

	MeshLOD.Indice.resize(IndiceNum);
	Fin.read((char*)MeshLOD.Indice.data(), BufferByteSize);

	for (uint32 i = 0; i < VertexNum; i++)
	{
		array<uint16, 4> JointWeight;
		array<uint16, 4> JointIndice;

		for (uint32 j = 0; j < 4; j++)
		{
			JointWeight[j] = static_cast<uint16>(MeshLOD.SkinnedWeightVertexArray[i].GetJointWeights()[j]);
			JointIndice[j] = MeshLOD.SkinnedWeightVertexArray[i].GetJointIndice()[j];
		}

		FSkeletalVertex Vertex =
		{ 
			StaticVertice[i].Pos,
			StaticVertice[i].Nor,
			StaticVertice[i].UV0,
			StaticVertice[i].Color,
			JointWeight,
			JointIndice,
		};
		MeshLOD.SkeletalVertexArray.push_back(Vertex);
	}

	vector<FSkeletalMeshLOD> MeshLODs;
	MeshLODs.push_back(MeshLOD);
	return MeshLODs;
}

FTransform FAssetManager::ReadComponentTransformInSceneBinary(std::ifstream& Fin)
{
	FTransform Trans;
	if (!Fin.is_open())
	{
		throw std::exception("ERROR:  open file faild.");
	}

	Fin.read((char*)&Trans.Translation, 3 * sizeof(float));
	Fin.read((char*)&Trans.Quat, 4 * sizeof(float));
	Fin.read((char*)&Trans.Scale, 3 * sizeof(float));

	return Trans;
}

vector<shared_ptr<FMaterialInterface>> FAssetManager::ReadMaterialInfosInSceneBinary(std::ifstream& Fin)
{
	vector<shared_ptr<FMaterialInterface>> Mats;

	int MatNum;
	Fin.read((char*)&MatNum, sizeof(int));
	for (uint32 i = 0; i < MatNum; i++)
	{
		// read FString
		int CharNum;
		Fin.read((char*)&CharNum, sizeof(int));
		string MaterialName;
		MaterialName.resize(CharNum);
		Fin.read((char*)MaterialName.data(), CharNum * sizeof(char));
		MaterialName = MaterialName.c_str();
		MaterialName = "Resouce\\Material\\" + MaterialName + ".material";

		Mats.push_back(LoadMaterial(MaterialName));
	}

	return Mats;
}

shared_ptr<class FSkeletalMesh> FAssetManager::LoadSkeletalMesh(const std::wstring& BinFileName)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("ERROR:  open file faild.");
	}

	shared_ptr<FSkeletalMesh> SkeMesh = make_shared<FSkeletalMesh>();

	SkeMesh->SetSkeletalMeshLods(ReadSkeletalMeshLods(Fin));

	Fin.close();

	return SkeMesh;
}

shared_ptr<FMaterialInterface> FAssetManager::LoadMaterial(const string& MaterialFileName)
{
	std::ifstream Fin(MaterialFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("ERROR:  open file faild.");
	}

	shared_ptr<FMaterialInterface> Mat;
	bool IsMaterialInstance;
	Fin.read((char*)&IsMaterialInstance, sizeof(bool));
	char padding[3];
	Fin.read((char*)&padding, 3 * sizeof(char));

	// read FString
	int CharNum;
	Fin.read((char*)&CharNum, sizeof(int));
	string BaseMaterialName;
	BaseMaterialName.resize(CharNum);
	Fin.read((char*)BaseMaterialName.data(), CharNum * sizeof(char));
	BaseMaterialName = BaseMaterialName.c_str();

	bool HasBaseMaterial = BaseMaterialMap.find(BaseMaterialName) != BaseMaterialMap.end();
	if (IsMaterialInstance)
	{
		if (!HasBaseMaterial) // if there isn't base material of this instance
		{
			BaseMaterialMap.insert({ BaseMaterialName, dynamic_pointer_cast<FMaterial>(LoadMaterial("Resource\\Material\\" + BaseMaterialName + ".material")) });
		}
		Mat = make_shared<FMaterialInstance>(BaseMaterialMap[BaseMaterialName].get());
	}
	else
	{
		if (!HasBaseMaterial)
		{
			BaseMaterialMap.insert({ BaseMaterialName, dynamic_pointer_cast<FMaterial>(LoadMaterial("Resource\\Material\\" + BaseMaterialName + ".material")) });
		}
		Mat = BaseMaterialMap[BaseMaterialName]; // base materials create in assetmanager when engine init.
	}

	int ScalarNum;
	Fin.read((char*)&ScalarNum, sizeof(int));
	for (uint32 i = 0; i < ScalarNum; i++)
	{
		float Scalar;
		Fin.read((char*)&Scalar, sizeof(float));
		Mat->ChangeScalarParams(i, Scalar);
	}

	int VectorNum;
	Fin.read((char*)&VectorNum, sizeof(int));
	for (uint32 i = 0; i < VectorNum; i++)
	{
		FVector4 Vector;
		Fin.read((char*)&Vector, 4 * sizeof(float));
		Mat->ChangeVectorParams(i, Vector);
	}

	int TextureNum;
	Fin.read((char*)&TextureNum, sizeof(int));
	for (uint32 i = 0; i < TextureNum; i++)
	{
		int CharNum;
		Fin.read((char*)&CharNum, sizeof(int));
		string Texture;
		Texture.resize(CharNum);
		Fin.read((char*)Texture.data(), CharNum * sizeof(char));
		Texture = Texture.c_str();
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		Mat->ChangeTextureParams(i, converter.from_bytes(Texture));
	}
}

shared_ptr<FSkeleton> FAssetManager::LoadSkeleton(const std::wstring& BinFileName)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("ERROR:  open file faild.");
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
	uint32 ElementNum;
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
		throw std::exception("ERROR:  open file faild.");
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
			Seq->GetTracks()[i].ScaleSamples.push_back(FVector(1.f, 1.f, 1.f));
		}

		Fin.read((char*)&QuatsNum, sizeof(int));
		Seq->GetTracks()[i].QuatSamples.resize(QuatsNum);
		Fin.read((char*)Seq->GetTracks()[i].QuatSamples.data(), QuatsNum * 4 * sizeof(float));
		if (QuatsNum == 0)
		{
			Seq->GetTracks()[i].QuatSamples.push_back(FQuat(1.f, 0.f, 0.f, 0.f)); // w x y z
		}

		Fin.read((char*)&TranslationsNum, sizeof(int));
		Seq->GetTracks()[i].TranslationSamples.resize(TranslationsNum);
		Fin.read((char*)Seq->GetTracks()[i].TranslationSamples.data(), TranslationsNum * 3 * sizeof(float));
		if (TranslationsNum == 0)
		{
			Seq->GetTracks()[i].TranslationSamples.push_back(FVector(0.f, 0.f, 0.f));
		}
	}

	uint32 IndexTableNum;
	Fin.read((char*)&IndexTableNum, sizeof(int));
	Seq->GetTrackToJointIndexMapTable().resize(IndexTableNum);
	Fin.read((char*)Seq->GetTrackToJointIndexMapTable().data(), IndexTableNum * sizeof(int));

	Fin.close();
	
	return Seq;
}

shared_ptr<RHI::FTexture> FAssetManager::LoadTexture(const wstring& TexFileName)
{
	shared_ptr<FTexture> Tex = GDynamicRHI->CreateTexture(TexFileName);
	GDynamicRHI->CommitTextureAsView(Tex.get(), FResViewType::SRV_RVT);
	return Tex;
}

//void FAssetManager::InitMaterialShaderMap()
//{
//	shared_ptr<FMaterial> BasicShapeMaterial = make_shared<FMaterial>(1, 1, 0);
//	BasicShapeMaterial->SetShader(L"Resource\\Shader\\BasicShapeMaterial.hlsl");
//	BasicShapeMaterial->SetBlendMode(FBlendMode::OPAQUE_BM);
//	BaseMaterialMap.insert({ "BasicShapeMaterial", BasicShapeMaterial });
//
//	shared_ptr<FMaterial> BrickWallMaterial = make_shared<FMaterial>(2, 0, 2);
//	BrickWallMaterial->SetShader(L"Resource\\Shader\\BrickWallMaterial.hlsl");
//	BrickWallMaterial->SetBlendMode(FBlendMode::OPAQUE_BM);
//	BaseMaterialMap.insert({ "BrickWallMaterial", BrickWallMaterial });
//
//	shared_ptr<FMaterial> BloomMaterial = make_shared<FMaterial>(0, 1, 0);
//	BloomMaterial->SetShader(L"Resource\\Shader\\BloomMaterial.hlsl");
//	BloomMaterial->SetBlendMode(FBlendMode::OPAQUE_BM);
//	BaseMaterialMap.insert({ "BloomMaterial", BloomMaterial });
//
//	shared_ptr<FMaterial> GlassBrickMaterial = make_shared<FMaterial>(5, 1, 2);
//	GlassBrickMaterial->SetShader(L"Resource\\Shader\\GlassBrickMaterial.hlsl");
//	GlassBrickMaterial->SetBlendMode(FBlendMode::TRANSLUCENT_BM);
//	BaseMaterialMap.insert({ "GlassBrickMaterial", GlassBrickMaterial });
//
//	shared_ptr<FMaterial> GlassMaterial = make_shared<FMaterial>(5, 1, 0);
//	GlassMaterial->SetShader(L"Resource\\Shader\\GlassMaterial.hlsl");
//	GlassMaterial->SetBlendMode(FBlendMode::TRANSLUCENT_BM);
//	BaseMaterialMap.insert({ "GlassMaterial", GlassMaterial });
//
//	shared_ptr<FMaterial> M_MaterialSphere = make_shared<FMaterial>(0, 0, 0);
//	M_MaterialSphere->SetShader(L"Resource\\Shader\\M_MaterialSphere.hlsl");
//	M_MaterialSphere->SetBlendMode(FBlendMode::OPAQUE_BM);
//	BaseMaterialMap.insert({ "M_MaterialSphere", M_MaterialSphere });
//
//	shared_ptr<FMaterial> M_MaterialSphere_Plain = make_shared<FMaterial>(0, 0, 0);
//	M_MaterialSphere_Plain->SetShader(L"Resource\\Shader\\M_MaterialSphere_Plain.hlsl");
//	M_MaterialSphere_Plain->SetBlendMode(FBlendMode::OPAQUE_BM);
//	BaseMaterialMap.insert({ "M_MaterialSphere_Plain", M_MaterialSphere_Plain });
//}
