#pragma once
#include "stdafx.h"

struct FTransform
{
	FVector Translation;
	FVector Rotation;
	FVector Scale;
};

class FMeshLOD
{
public:
	FMeshLOD() = default;
	~FMeshLOD() = default;

	void ResizeVertices(uint32 Size) { Vertices.resize(Size); }
	void ResizeIndices(uint32 Size) { Indices.resize(Size); }
	const vector<float>& GetVertices() const { return Vertices; }
	const vector<uint32>& GetIndices() const { return Indices; }
	const uint32& GetVertexStride() const { return VertexStride; }
	void SetVertexStride(const uint32& Stride) { VertexStride = Stride; }
	void SetVertices(const vector<float>& Param) { Vertices = Param; }
	void SetIndices(const vector<uint32>& Param) { Indices = Param; }

public:
	uint32 VertexStride;
	vector<float> Vertices;
	vector<uint32> Indices;
};

class FMeshActor
{
public:
	FMeshActor() = default;
	~FMeshActor() = default;

	FTransform Transform;
	vector<FMeshLOD> MeshLODs;
};