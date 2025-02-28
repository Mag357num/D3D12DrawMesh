#include "Material.h"

void FMaterialInstance::ChangeScalarParams(uint32 Index, float S)
{
	if (OriginMaterial->GetNumericParams().ScalarParams.size() < Index + 1)
	{
		OriginMaterial->ResizeScalarParams(Index + 1);
	}
	std::pair<uint32, float> pair(Index, S); InstanceScalarParams.insert(pair);
}

void FMaterialInstance::ChangeVectorParams(uint32 Index, FVector4 V)
{
	if (OriginMaterial->GetNumericParams().VectorParams.size() < Index + 1)
	{
		OriginMaterial->ResizeVectorParams(Index + 1);
	}
	std::pair<uint32, FVector4> pair(Index, V); InstanceVectorParams.insert(pair);
}

void FMaterialInstance::ChangeTextureParams(uint32 Index, wstring T)
{
	if (OriginMaterial->GetTextureParams().size() < Index + 1)
	{
		OriginMaterial->ResizeTextureParams(Index + 1);
	}
	std::pair<uint32, wstring> pair(Index, T); InstanceTexParams.insert(pair);
}

FMaterialParam FMaterialInstance::GetNumericParams()
{
	FMaterialParam Param = OriginMaterial->GetNumericParams();
	for (unordered_map<uint32, float>::iterator iter = InstanceScalarParams.begin(); iter != InstanceScalarParams.end(); iter++)
	{
		Param.ScalarParams[iter->first] = iter->second;
	}
	for (unordered_map<uint32, FVector4>::iterator iter = InstanceVectorParams.begin(); iter != InstanceVectorParams.end(); iter++)
	{
		Param.VectorParams[iter->first] = iter->second;
	}
	return Param;
}

vector<wstring> FMaterialInstance::GetTextureParams()
{
	vector<wstring> Texs = OriginMaterial->GetTextureParams();
	for (unordered_map<uint32, wstring>::iterator iter = InstanceTexParams.begin(); iter != InstanceTexParams.end(); iter++)
	{
		Texs[iter->first] = iter->second;
	}
	return Texs;
}


FMaterial::FMaterial(uint32 ScalarNum, uint32 VectorNum, uint32 TextureNum)
{
	NumericParams.ScalarParams.resize(ScalarNum);
	NumericParams.VectorParams.resize(VectorNum);
	TextureParams.resize(TextureNum);
}

shared_ptr<FMaterialInstance> FMaterial::CreateInstance()
{
	shared_ptr<FMaterialInstance> Instance = make_shared<FMaterialInstance>(this);
	Instance->SetOriginMaterial(this);
	Instance->SetBlendMode(GetBlendMode());
	return Instance;
}

void FMaterial::ChangeScalarParams(uint32 Index, float S)
{
	if (NumericParams.ScalarParams.size() < Index + 1)
	{
		NumericParams.ScalarParams.resize(Index + 1);
	}
	NumericParams.ScalarParams[Index] = S;
}

void FMaterial::ChangeVectorParams(uint32 Index, FVector4 V)
{
	if (NumericParams.VectorParams.size() < Index + 1)
	{
		NumericParams.VectorParams.resize(Index + 1);
	}
	NumericParams.VectorParams[Index] = V;
}

void FMaterial::ChangeTextureParams(uint32 Index, wstring T)
{
	if (TextureParams.size() < Index + 1)
	{
		TextureParams.resize(Index + 1);
	}
	TextureParams[Index] = T;
}
