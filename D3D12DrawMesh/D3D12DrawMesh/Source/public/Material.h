#pragma once
#include "stdafx.h"
#include "RHIResource.h"

using namespace RHI;

struct FMaterialParam // declare this for writeing constant buffer more convenient
{
	vector<float> FloatParams;
	vector<FVector4> VectorParams;
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
	virtual vector<shared_ptr<FTexture>>& GetTextureParams() = 0;


	void SetBlendMode(const FBlendMode& BM) { BlendMode = BM; }
	const FBlendMode& GetBlendMode() const { return BlendMode; }
};

class FMaterial : public FMaterialInterface
{
private:
	FMaterialParam NumericParams;
	vector<shared_ptr<FTexture>> TextureParams;

	wstring ShaderFile; // i'd like to write vs, ps, other shader in one shader file

public:
	shared_ptr<class FMaterialInstance> CreateInstance();

	void SetShader(const wstring& File) { ShaderFile = File; }
	virtual const wstring& GetShader() const override { return ShaderFile; }

	void AddFloatParam(const float& F) { NumericParams.FloatParams.push_back(F); }
	void AddVectorParam(const FVector& F) { NumericParams.VectorParams.push_back(PaddingToVec4(F)); }
	void AddTexture(shared_ptr<FTexture> T) { TextureParams.push_back(T); }
	virtual FMaterialParam& GetNumericParams() override { return NumericParams; }
	virtual vector<shared_ptr<FTexture>>& GetTextureParams() override { return TextureParams; }
};

class FMaterialInstance : public FMaterialInterface
{
private:
	FMaterial* OriginMaterial;

	FMaterialParam NumericParams;
	vector<shared_ptr<RHI::FTexture>> TextureParams;
public:
	FMaterialInstance() = default; // TODO: make FMaterialInstance cant be create public, only be create by material
	~FMaterialInstance() = default;

	void SetNumericParam(FMaterialParam P) { NumericParams = P; }
	void SetOriginMaterial(FMaterial* Mat) { OriginMaterial = Mat; };
	virtual const wstring& GetShader() const override;
	virtual FMaterialParam& GetNumericParams() override { return NumericParams; }
	virtual vector<shared_ptr<FTexture>>& GetTextureParams() override { return TextureParams; }
};