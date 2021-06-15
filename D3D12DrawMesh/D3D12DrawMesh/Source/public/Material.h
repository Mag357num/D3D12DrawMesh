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
	virtual FMaterialParam GetNumericParams() = 0;
	virtual vector<wstring> GetTextureParams() = 0;

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
	shared_ptr<class FMaterialInstance> CreateInstance();

	void SetShader(const wstring& File) { ShaderFile = File; }
	virtual const wstring& GetShader() const override { return ShaderFile; }

	void AddFloatParam(const float& F) { NumericParams.FloatParams.push_back(F); }
	void AddVectorParam(const FVector& F) { NumericParams.VectorParams.push_back(PaddingToVec4(F)); }
	void AddTextureParam(wstring T) { TextureParams.push_back(T); }
	virtual FMaterialParam GetNumericParams() override { return NumericParams; }
	virtual vector<wstring> GetTextureParams() override { return TextureParams; }
};

class FMaterialInstance : public FMaterialInterface
{
private:
	FMaterial* OriginMaterial;

	unordered_map<uint32, float> InstanceFloatParams;
	unordered_map<uint32, FVector4> InstanceVectorParams;
	unordered_map<uint32, wstring> InstanceTexParams;
public:
	FMaterialInstance(FMaterial* Mat) { }; // FMaterialInstance should be constructed without a FMaterial(as a template)
	~FMaterialInstance() = default;

	void ChangeFloatParams(std::pair<uint32, float> pair) { InstanceFloatParams.insert(pair); }
	void ChangeVectorParams(std::pair<uint32, FVector4> pair) { InstanceVectorParams.insert(pair); }
	void ChangeTextureParams(std::pair<uint32, wstring> pair) { InstanceTexParams.insert(pair); }

	void SetOriginMaterial(FMaterial* Mat) { OriginMaterial = Mat; };
	virtual const wstring& GetShader() const override { return OriginMaterial->GetShader(); }
	virtual FMaterialParam GetNumericParams() override;
	virtual vector<wstring> GetTextureParams() override;
};