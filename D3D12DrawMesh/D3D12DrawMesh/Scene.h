#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "SimpleCamera.h"
#include "StaticMesh.h"
#include "Light.h"
#include "Character.h"

enum class SceneCameraManipulateType
{
	KEY_UP = 0,
	KEY_DOWN = 1,
	MOUSEMOVE = 2,
	RIGHT_BUTTON_DOWN = 3,
	RIGHT_BUTTON_UP = 4,
};

class FScene
{
private:
	FCamera CurrentCamera; // TODO: change to std::vector<FCamera> Cameras
	FDirectionLight DirectionLight; // TODO: extend to a array of lights
	vector<FStaticMeshComponent> ComponentArray;
	FCharacter Character;

public:
	void UpdateMainCamera(class FEngine* Engine);
	void ManipulateCamera(unsigned char Key, uint32 X, uint32 Y, SceneCameraManipulateType Type);

	void SetCurrentCamera(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float AspectRatio) { return CurrentCamera.Init(PositionParam, UpDir, LookAt, Fov, AspectRatio); }
	void SetDirectionLight(const FDirectionLight& Light) { DirectionLight = Light; }
	void SetComponentArray(const vector<FStaticMeshComponent>& Coms) { ComponentArray = Coms; }

	FCamera& GetCurrentCamera() { return CurrentCamera; }
	const FDirectionLight& GetDirectionLight() const { return DirectionLight; }
	const vector<FStaticMeshComponent>& GetComponentArray() const { return ComponentArray; }
};