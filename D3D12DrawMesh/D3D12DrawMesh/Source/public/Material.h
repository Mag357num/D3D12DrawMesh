#pragma once
#include "stdafx.h"
#include "RHIResource.h"

using namespace RHI;

struct FMaterialParam // declare this for writeing constant buffer more convenient
{
	vector<float> FloatParams;
	vector<FVector> VectorParams;
};

class FMaterialInterface
{
private:
	FBlendMode BlendMode = FBlendMode::OPAQUE_BM;
	FShadingMode ShadingMode; // unused
	FMaterialDomain MaterialDomain; // unused
public:
	FMaterialInterface() = default;
	virtual ~FMaterialInterface() = default;

	virtual const wstring& GetShader() const = 0;
	virtual FMaterialParam& GetNumericParams() = 0;


	void SetBlendMode(const FBlendMode& BM) { BlendMode = BM; }
	const FBlendMode& GetBlendMode() const { return BlendMode; }
};

class FMaterial : public FMaterialInterface
{
private:
	FMaterialParam NumericParam;
	vector<shared_ptr<FTexture>> TextureParams;

	wstring ShaderFile; // i'd like to write vs, ps, other shader in one shader file

public:
	void SetShader(const wstring& File) { ShaderFile = File; }
	virtual const wstring& GetShader() const override { return ShaderFile; }

	void AddFloatParam(const float& F) { NumericParam.FloatParams.push_back(F); }
	void AddVectorParam(const FVector& F) { NumericParam.VectorParams.push_back(F); }
	void AddTexture(shared_ptr<FTexture> T) { TextureParams.push_back(T); }
	virtual FMaterialParam& GetNumericParams() override { return NumericParam; }
	vector<shared_ptr<FTexture>>& GetTextureParams() { return TextureParams; }
};

class FMaterialInstance : public FMaterialInterface
{
private:
	shared_ptr<FMaterial> OriginMaterial;

	FMaterialParam NumericParam;
	vector<shared_ptr<RHI::FTexture>> TextureParam;
};