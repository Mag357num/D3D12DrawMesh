#pragma once
#include "stdafx.h"
#include "ActorComponent.h"
#include "Actor.h"
#include "StaticVertex.h"

class FStaticMeshLOD
{
private:
	// TODO: refactor to a FStaticVertex Array
	vector<FStaticVertex> Vertices;

	vector<uint32> Indices;

public:
	void ResizeVertices(uint32 Size) { Vertices.resize(Size); }
	void ResizeIndices(uint32 Size) { Indices.resize(Size); }
	const vector<FStaticVertex>& GetVertices2() const { return Vertices; }
	const vector<uint32>& GetIndices() const { return Indices; }
	//const uint32& GetVertexStride() const { return VertexStride; }
	//void SetVertexStride(const uint16& Stride) { VertexStride = Stride; }
	void SetIndices(const vector<uint32>& Param) { Indices = Param; }

	FStaticMeshLOD() = default;
	FStaticMeshLOD(const vector<FStaticVertex>& Vertices, const vector<uint32>& Indices)
		: Vertices(Vertices), Indices(Indices) {}
	~FStaticMeshLOD() = default;
};

class TStaticMesh
{
private:
	vector<FStaticMeshLOD> MeshLODs;
public:
	void SetMeshLODs(const vector<FStaticMeshLOD>& LODs) { MeshLODs = LODs; }
	const vector<FStaticMeshLOD>& GetMeshLODs() const { return MeshLODs; }

	TStaticMesh() = default;
	~TStaticMesh() = default;
};

class TStaticMeshComponent : public FActorComponent
{
private:
	shared_ptr<TStaticMesh> StaticMesh;
	wstring ShaderFileName;

public:
	void SetTransform(const FTransform& Trans) { Transform = Trans; }
	void SetShaderFileName(const wstring& Name) { ShaderFileName = Name; }
	void SetStaticMesh(shared_ptr<TStaticMesh> SM) { StaticMesh = SM; }

	TStaticMesh* GetStaticMesh() { return StaticMesh.get(); }
	const wstring& GetShaderFileName() const { return ShaderFileName; }

	TStaticMeshComponent() = default;
	~TStaticMeshComponent() = default;
};

class TStaticMeshActor : public AActor
{
public:
	void SetStaticMeshComponent(shared_ptr<TStaticMeshComponent> Com);
	TStaticMeshComponent* GetStaticMeshComponent();
};