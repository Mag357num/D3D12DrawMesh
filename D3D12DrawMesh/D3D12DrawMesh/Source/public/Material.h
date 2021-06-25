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
	string MaterialName;
	EBlendMode BlendMode = EBlendMode::OPAQUE_BM;
	//FShadingMode ShadingMode; // unused
	//FMaterialDomain MaterialDomain; // unused

public:
	FMaterialInterface() = default;
	virtual ~FMaterialInterface() = default;

	void SetBlendMode(const EBlendMode& BM) { BlendMode = BM; }
	void SetName(const string& Name) { MaterialName = Name; }

	virtual const wstring& GetShader() const = 0;
	virtual FMaterialParam GetNumericParams() = 0;
	virtual vector<wstring> GetTextureParams() = 0;
	const EBlendMode& GetBlendMode() const { return BlendMode; }
	const string& GetName() { return MaterialName; }

	virtual void ChangeScalarParams(const uint32& Index, const float& S) = 0;
	virtual void ChangeVectorParams(const uint32& Index, const FVector4& V) = 0;
	virtual void ChangeTextureParams(const uint32& Index, const wstring& T) = 0;

};

class FMaterial : public FMaterialInterface
{
private:
	FMaterialParam NumericParams;
	vector<wstring> TextureParams;

	wstring ShaderFile; // i'd like to write vs, ps, other shader in one shader file

public:
	FMaterial() = default;
	//FMaterial(uint32 ScalarNum, uint32 VectorNum, uint32 TextureNum);
	~FMaterial() = default;

	shared_ptr<class FMaterialInstance> CreateInstance();

	void SetShader(const wstring& File) { ShaderFile = File; }
	virtual const wstring& GetShader() const override { return ShaderFile; }

	virtual void ChangeScalarParams(const uint32& Index, const float& S) override;
	virtual void ChangeVectorParams(const uint32& Index, const FVector4& V) override;
	virtual void ChangeTextureParams(const uint32& Index, const wstring& T) override;

	virtual FMaterialParam GetNumericParams() override { return NumericParams; }
	virtual vector<wstring> GetTextureParams() override { return TextureParams; }

	void ResizeScalarParams(uint32 Size) { NumericParams.ScalarParams.resize(Size); }
	void ResizeVectorParams(uint32 Size) { NumericParams.VectorParams.resize(Size); }
	void ResizeTextureParams(uint32 Size) { TextureParams.resize(Size); }
};

class FMaterialInstance : public FMaterialInterface
{
private:
	FMaterial* BaseMaterial;
	string BaseMaterialName;

	unordered_map<uint32, float> InstanceScalarParams;
	unordered_map<uint32, FVector4> InstanceVectorParams;
	unordered_map<uint32, wstring> InstanceTexParams;
public:
	FMaterialInstance(FMaterial* Mat) { BaseMaterial = Mat; BaseMaterialName = Mat->GetName(); }; // FMaterialInstance should be constructed with a FMaterial(as a template)
	~FMaterialInstance() = default;

	virtual void ChangeScalarParams(const uint32& Index, const float& S) override;
	virtual void ChangeVectorParams(const uint32& Index, const FVector4& V) override;
	virtual void ChangeTextureParams(const uint32& Index, const wstring& T) override;

	void SetBaseMaterial(FMaterial* Mat) { BaseMaterial = Mat; };

	virtual const wstring& GetShader() const override { return BaseMaterial->GetShader(); }
	virtual FMaterialParam GetNumericParams() override;
	virtual vector<wstring> GetTextureParams() override;
};