#pragma once
#include "stdafx.h"
#include "RHIResource.h"

using namespace RHI;

class FMaterialInterface
{
private:
	FBlendMode BlendMode = FBlendMode::OPAQUE_BM;
	FShadingMode ShadingMode; // unused
	FMaterialDomain MaterialDomain; // unused
public:
	FMaterialInterface() = default;
	virtual ~FMaterialInterface() = default;

	void SetBlendMode(const FBlendMode& BM) { BlendMode = BM; }
	const FBlendMode& GetBlendMode() const { return BlendMode; }
};

class FMaterial : public FMaterialInterface
{
private:
	vector<float> FloatParams;
	vector<FVector> VectorParams;
	vector<shared_ptr<FTexture>> TextureParams;

	wstring ShaderFile; // i'd like to write vs, ps, other shader in one shader file

public:
	void SetShader(const wstring& File) { ShaderFile = File; }
	const wstring& GetShader() const { return ShaderFile; }

	void AddFloatParam(const float& F) { FloatParams.push_back(F); }
	void AddVectorParam(const FVector& F) { VectorParams.push_back(F); }
	void AddTexture(shared_ptr<FTexture> T) { TextureParams.push_back(T); }
	vector<float>& GetFloatParams() { return FloatParams; }
	vector<FVector>& GetVectorParams() { return VectorParams; }
	vector<shared_ptr<FTexture>>& GetTextureParams() { return TextureParams; }
};

class FMaterialInstance : public FMaterialInterface
{
private:
	shared_ptr<FMaterial> OriginMaterial;

	vector<float> FloatParam;
	vector<FVector> VectorParam;
	vector<shared_ptr<RHI::FTexture>> TextureParam;
};