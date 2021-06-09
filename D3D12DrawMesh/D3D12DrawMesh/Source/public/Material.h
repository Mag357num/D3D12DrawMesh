#pragma once
#include "stdafx.h"
#include "RHIResource.h"

using namespace RHI;

class FMaterial
{
private:
	vector<float> FloatParam;
	vector<FVector> VectorParam;
	vector<shared_ptr<FTexture>> TextureParam;

	shared_ptr<FShader> VS;
	shared_ptr<FShader> PS;
};

class FMaterialInstance
{
private:
	shared_ptr<FMaterial> OriginMaterial;

	vector<float> FloatParam;
	vector<FVector> VectorParam;
	vector<shared_ptr<RHI::FTexture>> TextureParam;
};