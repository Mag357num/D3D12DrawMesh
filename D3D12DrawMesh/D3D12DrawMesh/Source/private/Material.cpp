#include "Material.h"

FMaterialParam FMaterialInstance::GetNumericParams()
{
	FMaterialParam Param = OriginMaterial->GetNumericParams();
	for (unordered_map<uint32, float>::iterator iter = InstanceFloatParams.begin(); iter != InstanceFloatParams.end(); iter++)
	{
		Param.FloatParams[iter->first] = iter->second;
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


shared_ptr<FMaterialInstance> FMaterial::CreateInstance()
{
	shared_ptr<FMaterialInstance> Instance = make_shared<FMaterialInstance>(this);
	Instance->SetOriginMaterial(this);
	Instance->SetBlendMode(GetBlendMode());
	return Instance;
}