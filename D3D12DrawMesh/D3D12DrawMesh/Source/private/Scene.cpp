#include "Scene.h"
#include "Engine.h"

void FScene::Tick(StepTimer& Timer)
{
	GetCurrentCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCurrentCamera()->Update(static_cast<float>(Timer.GetElapsedSeconds()), FCameraMoveMode::AROUNDTARGET, GetCurrentCharacter()->GetSkeletalMeshCom()->GetTransform().Translation, 300.f);
}

FDirectionLight& FScene::GetDirectionLight()
{
	return DirectionLight;
}

vector<AStaticMeshActor>& FScene::GetStaticMeshActors()
{
	return StaticMeshActors;
}

void FScene::AddStaticMeshActor( AStaticMeshActor Actor )
{
	StaticMeshActors.push_back(Actor);
}
