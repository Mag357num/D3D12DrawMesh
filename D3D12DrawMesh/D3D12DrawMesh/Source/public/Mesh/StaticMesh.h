#pragma once
#include "stdafx.h"
#include "ActorComponent.h"
#include "Actor.h"
#include "StaticVertex.h"

struct FStaticMeshLOD
{
public:
	vector<FStaticVertex> Vertice;
	vector<uint32> Indice;
public:
	FStaticMeshLOD() = default;
	FStaticMeshLOD( const vector<FStaticVertex>& Vertice, const vector<uint32>& Indice ) : Vertice( Vertice ), Indice( Indice ) { }
	~FStaticMeshLOD() = default;
};

class FStaticMesh
{
private:
	vector<FStaticMeshLOD> MeshLODs;
public:
	FStaticMesh() = default;
	~FStaticMesh() = default;

	void SetMeshLODs(const vector<FStaticMeshLOD>& LODs) { MeshLODs = LODs; }
	const vector<FStaticMeshLOD>& GetMeshLODs() const { return MeshLODs; }
};

class FStaticMeshComponent : public FActorComponent
{
private:
	shared_ptr<FStaticMesh> StaticMesh;

public:
	FStaticMeshComponent() = default;
	~FStaticMeshComponent() = default;

	void SetStaticMesh(shared_ptr<FStaticMesh> SM) { StaticMesh = SM; }
	FStaticMesh* GetStaticMesh() { return StaticMesh.get(); }
};

class AStaticMeshActor : public AActor
{
private:
	float AngularVelocity = 50.f;

	bool Dirty = true;

public:
	void SetStaticMeshComponent(shared_ptr<FStaticMeshComponent> Com);
	FStaticMeshComponent* GetStaticMeshCom();

	const float& GetAnglarVel() const { return AngularVelocity; }
	void SetAnglarVel(const float& Ang) { AngularVelocity = Ang; }

	const bool& IsDirty() const { return Dirty; }
	void SetDirty(const bool& Dirty) { this->Dirty = Dirty; }

	void Tick(const float& ElapsedSeconds);
};