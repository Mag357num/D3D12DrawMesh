#pragma once
#include "stdafx.h"

class FStaticMeshLOD
{
private:
	uint32 VertexStride;
	vector<float> Vertices;
	vector<uint32> Indices;

public:
	FStaticMeshLOD() = default;
	~FStaticMeshLOD() = default;

	void ResizeVertices(uint32 Size) { Vertices.resize(Size); }
	void ResizeIndices(uint32 Size) { Indices.resize(Size); }
	const vector<float>& GetVertices() const { return Vertices; }
	const vector<uint32>& GetIndices() const { return Indices; }
	const uint32& GetVertexStride() const { return VertexStride; }
	void SetVertexStride(const uint32& Stride) { VertexStride = Stride; }
	void SetVertices(const vector<float>& Param) { Vertices = Param; }
	void SetIndices(const vector<uint32>& Param) { Indices = Param; }
};

class FStaticMeshActor
{
public:
	FStaticMeshActor() = default;
	~FStaticMeshActor() = default;

	FTransform Transform;
	vector<FStaticMeshLOD> MeshLODs;
	wstring ShaderFileName;
};