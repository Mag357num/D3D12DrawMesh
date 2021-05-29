#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"
#include "Key.h"

class ACharacter : public AActor
{
private:
	KeysPressed Keys = {};
	float MoveSpeed = 300.f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.005f;
	bool IsMouseDown = false;

	FVector2 MouseDown_CurrentPosition;
	FVector2 MouseDown_FirstPosition;
	FVector2 MouseMove_FirstPosition;
	FVector2 MouseMove_CurrentPosition;

public:
	void SetSkeletalMeshCom( shared_ptr<TSkeletalMeshComponent> SkeMesh ) { Components.push_back( SkeMesh ); }
	void Tick( const float& ElapsedSeconds );

	TSkeletalMeshComponent* GetSkeletalMeshCom() { return Components[0]->As<TSkeletalMeshComponent>(); }
	FVector& GetLocation() { return GetSkeletalMeshCom()->GetTransform().Translation; }
	FVector GetLook();

	void OnKeyDown( const unsigned char& key );
	void OnKeyUp( const unsigned char& key );
	void OnButtonDown( const uint32& x, const uint32& y );
	void OnButtonUp();
	void OnMouseMove( const uint32& x, const uint32& y );
	void SetCurrentAnim(string Key);


	ACharacter() = default;
	~ACharacter() = default;

};

