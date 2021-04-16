#pragma once
#include "stdafx.h"

struct FTransform
{
	FVector Translation;
	FVector Rotator;
	FVector Scale;
};

class FMeshLOD
{
public:
	FMeshLOD() = default;
	~FMeshLOD() = default;

	vector<float>& GetVertices() { return Vertices; }
	vector<uint32>& GetIndices() { return Indices; }
	uint32& GetVertexStride() { return VertexStride; }

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

	uint32 MeshActorIndex;
	FTransform Transform;
	vector<FMeshLOD> MeshLODs;
};