#include "Material.h"

const wstring& FMaterialInstance::GetShader() const
{
	return OriginMaterial->GetShader();
}

shared_ptr<FMaterialInstance> FMaterial::CreateInstance()
{
	shared_ptr<FMaterialInstance> Instance = make_shared<FMaterialInstance>(this);
	Instance->SetNumericParam(NumericParams);
	Instance->SetOriginMaterial(this);
	Instance->SetBlendMode(GetBlendMode());
	return Instance;
}