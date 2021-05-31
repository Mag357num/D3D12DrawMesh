#pragma once
#include "stdafx.h"
#include "ActorComponent.h"
#include "Actor.h"
#include "StaticVertex.h"

struct TStaticMeshLOD
{
public:
	vector<TStaticVertex> Vertice;
	vector<uint32> Indice;
public:
	TStaticMeshLOD() = default;
	TStaticMeshLOD( const vector<TStaticVertex>& Vertice, const vector<uint32>& Indice ) : Vertice( Vertice ), Indice( Indice ) { }
	~TStaticMeshLOD() = default;
};

class TStaticMesh
{
private:
	vector<TStaticMeshLOD> MeshLODs;
public:
	TStaticMesh() = default;
	~TStaticMesh() = default;

	void SetMeshLODs(const vector<TStaticMeshLOD>& LODs) { MeshLODs = LODs; }
	const vector<TStaticMeshLOD>& GetMeshLODs() const { return MeshLODs; }
};

class TStaticMeshComponent : public TActorComponent
{
private:
	shared_ptr<TStaticMesh> StaticMesh;

public:
	TStaticMeshComponent() = default;
	~TStaticMeshComponent() = default;

	void SetStaticMesh(shared_ptr<TStaticMesh> SM) { StaticMesh = SM; }
	TStaticMesh* GetStaticMesh() { return StaticMesh.get(); }
};

class AStaticMeshActor : public AActor
{
public:
	void SetStaticMeshComponent(shared_ptr<TStaticMeshComponent> Com);
	TStaticMeshComponent* GetStaticMeshCom();
};