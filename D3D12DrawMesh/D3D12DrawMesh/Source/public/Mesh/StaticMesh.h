#pragma once
#include "stdafx.h"
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
	vector<shared_ptr<FStaticMeshLOD>> MeshLODs;
public:
	FStaticMesh() = default;
	~FStaticMesh() = default;

	void SetMeshLODs(const vector<shared_ptr<FStaticMeshLOD>>& LODs) { MeshLODs = LODs; }
	const vector<shared_ptr<FStaticMeshLOD>>& GetMeshLODs() const { return MeshLODs; }
};

class FStaticMeshComponent : public FMeshComponent
{
private:
	shared_ptr<FStaticMesh> StaticMesh;

public:
	FStaticMeshComponent() { Type = EComponentType::STATICMESH_COMPONENT; };
	~FStaticMeshComponent() = default;

	void SetStaticMesh(shared_ptr<FStaticMesh> SM) { StaticMesh = SM; }
	FStaticMesh* GetStaticMesh() { return StaticMesh.get(); }
};

class AStaticMeshActor : public AActor
{
private:
	float AngularVelocity = 50.f;
	FStaticMeshComponent* StaticMeshComponent;

public:
	AStaticMeshActor(shared_ptr<FStaticMeshComponent> Sta) { RootComponent = Sta; StaticMeshComponent = Sta.get(); AddOwnedComponent(Sta); }

	void SetAnglarVel(const float& Ang) { AngularVelocity = Ang; }

	const float& GetAnglarVel() const { return AngularVelocity; }
	FStaticMeshComponent* GetStaticMeshComponent() { return StaticMeshComponent; }

	void Tick_ActorRotate(const float& ElapsedSeconds);
};