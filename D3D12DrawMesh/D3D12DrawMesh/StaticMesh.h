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

class UStaticMesh
{
private:
	vector<FStaticMeshLOD> MeshLODs;
public:
	void ResizeLOD(const uint32& Size) { MeshLODs.resize(Size); }
	void SetMeshLODs(const vector<FStaticMeshLOD>& LODs) { MeshLODs = LODs; }
	const vector<FStaticMeshLOD>& GetMeshLODs() const { return MeshLODs; }

	UStaticMesh() = default;
	~UStaticMesh() = default;
};

class FStaticMeshComponent : public FActorComponent
{
private:
	UStaticMesh StaticMesh; // TODO: change to a smart ptr to divide the UStaticMesh with FStaticMeshComponent
	wstring ShaderFileName;

public:
	void ResizeLOD(const uint32& Size) { StaticMesh.ResizeLOD(Size); }

	void SetTransform(const FTransform& Trans) { Transform = Trans; }
	void SetShaderFileName(const wstring& Name) { ShaderFileName = Name; }
	void SetMeshLODs(const vector<FStaticMeshLOD>& LODs) { StaticMesh.SetMeshLODs(LODs); }
	void SetStaticMesh(const UStaticMesh& SM) { StaticMesh = SM; }

	const FTransform& GetTransform() const { return Transform; }
	UStaticMesh& GetStaticMesh() { return StaticMesh; }
	const wstring& GetShaderFileName() const { return ShaderFileName; }

	FStaticMeshComponent() = default;
	~FStaticMeshComponent() = default;
};

class AStaticMeshActor : public AActor
{
	// contain a static mesh component
};