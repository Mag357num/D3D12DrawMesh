#pragma once
#include "stdafx.h"
#include "RHIResource.h"

using namespace RHI;

struct FMaterialParam // declare this for writeing constant buffer more convenient
{
	vector<float> ScalarParams;
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
	virtual FMaterialParam GetNumericParams() = 0;
	virtual vector<wstring> GetTextureParams() = 0;

	virtual void ChangeScalarParams(uint32 Index, float S) = 0;
	virtual void ChangeVectorParams(uint32 Index, FVector4 V) = 0;
	virtual void ChangeTextureParams(uint32 Index, wstring T) = 0;

	void SetBlendMode(const FBlendMode& BM) { BlendMode = BM; }
	const FBlendMode& GetBlendMode() const { return BlendMode; }
};

class FMaterial : public FMaterialInterface
{
private:
	FMaterialParam NumericParams;
	vector<wstring> TextureParams;

	wstring ShaderFile; // i'd like to write vs, ps, other shader in one shader file

public:
	FMaterial(uint32 ScalarNum, uint32 VectorNum, uint32 TextureNum);
	~FMaterial() = default;

	shared_ptr<class FMaterialInstance> CreateInstance();

	void SetShader(const wstring& File) { ShaderFile = File; }
	virtual const wstring& GetShader() const override { return ShaderFile; }

	virtual void ChangeScalarParams(uint32 Index, float S) override;
	virtual void ChangeVectorParams(uint32 Index, FVector4 V) override;
	virtual void ChangeTextureParams(uint32 Index, wstring T) override;

	virtual FMaterialParam GetNumericParams() override { return NumericParams; }
	virtual vector<wstring> GetTextureParams() override { return TextureParams; }

	void ResizeScalarParams(uint32 Size) { NumericParams.ScalarParams.resize(Size); }
	void ResizeVectorParams(uint32 Size) { NumericParams.VectorParams.resize(Size); }
	void ResizeTextureParams(uint32 Size) { TextureParams.resize(Size); }
};

class FMaterialInstance : public FMaterialInterface
{
private:
	FMaterial* OriginMaterial;

	unordered_map<uint32, float> InstanceScalarParams;
	unordered_map<uint32, FVector4> InstanceVectorParams;
	unordered_map<uint32, wstring> InstanceTexParams;
public:
	FMaterialInstance(FMaterial* Mat) { }; // FMaterialInstance should be constructed without a FMaterial(as a template)
	~FMaterialInstance() = default;

	virtual void ChangeScalarParams(uint32 Index, float S) override;
	virtual void ChangeVectorParams(uint32 Index, FVector4 V) override;
	virtual void ChangeTextureParams(uint32 Index, wstring T) override;

	void SetOriginMaterial(FMaterial* Mat) { OriginMaterial = Mat; };
	virtual const wstring& GetShader() const override { return OriginMaterial->GetShader(); }
	virtual FMaterialParam GetNumericParams() override;
	virtual vector<wstring> GetTextureParams() override;
};