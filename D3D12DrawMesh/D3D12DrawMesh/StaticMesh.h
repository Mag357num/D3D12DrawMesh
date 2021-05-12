#pragma once
#include "stdafx.h"

class FStaticMeshLOD
{
private:
	uint32 VertexStride;
	vector<float> Vertices;
	vector<uint32> Indices;

public:
	void ResizeVertices(uint32 Size) { Vertices.resize(Size); }
	void ResizeIndices(uint32 Size) { Indices.resize(Size); }
	const vector<float>& GetVertices() const { return Vertices; }
	const vector<uint32>& GetIndices() const { return Indices; }
	const uint32& GetVertexStride() const { return VertexStride; }
	void SetVertexStride(const uint16& Stride) { VertexStride = Stride; }
	void SetVertices(const vector<float>& Param) { Vertices = Param; }
	void SetIndices(const vector<uint32>& Param) { Indices = Param; }

	FStaticMeshLOD() = default;
	FStaticMeshLOD(const uint16& Stride, const vector<float>& Vertices, const vector<uint32>& Indices)
		: VertexStride(Stride), Vertices(Vertices), Indices(Indices) {}
	~FStaticMeshLOD() = default;
};

class FStaticMesh
{
private:
	vector<FStaticMeshLOD> MeshLODs;
public:
	void ResizeLOD(const uint32& Size) { MeshLODs.resize(Size); }
	void SetMeshLODs(const vector<FStaticMeshLOD>& LODs) { MeshLODs = LODs; }
	const vector<FStaticMeshLOD>& GetMeshLODs() const { return MeshLODs; }

	FStaticMesh() = default;
	~FStaticMesh() = default;
};

class FStaticMeshComponent
{
private:
	FStaticMesh StaticMesh;
	FTransform Transform;
	wstring ShaderFileName;

public:
	void ResizeLOD(const uint32& Size) { StaticMesh.ResizeLOD(Size); }

	void SetTransform(const FTransform& Trans) { Transform = Trans; }
	void SetShaderFileName(const wstring& Name) { ShaderFileName = Name; }
	void SetMeshLODs(const vector<FStaticMeshLOD>& LODs) { StaticMesh.SetMeshLODs(LODs); }
	void SetStaticMesh(const FStaticMesh& SM) { StaticMesh = SM; }

	const FTransform& GetTransform() const { return Transform; }
	const FStaticMesh& GetStaticMesh() const { return StaticMesh; }
	const wstring& GetShaderFileName() const { return ShaderFileName; }
	const vector<FStaticMeshLOD>& GetMeshLODs() const { return StaticMesh.GetMeshLODs(); }

	FStaticMeshComponent() = default;
	~FStaticMeshComponent() = default;
};