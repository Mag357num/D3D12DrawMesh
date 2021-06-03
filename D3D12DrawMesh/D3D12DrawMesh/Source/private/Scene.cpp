#include "Scene.h"
#include "Engine.h"

void FScene::Tick(StepTimer& Timer)
{
	GetCurrentCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCurrentCamera()->Tick(static_cast<float>(Timer.GetElapsedSeconds()), FCameraMoveMode::AROUNDTARGET, GetCurrentCharacter()->GetSkeletalMeshCom()->GetTransform().Translation, 300.f);
}

ADirectionLight* FScene::GetDirectionLight()
{
	return DirectionLight.get();
}

vector<shared_ptr<AStaticMeshActor>>& FScene::GetStaticMeshActors()
{
	return StaticMeshActors;
}

void FScene::AddStaticMeshActor(shared_ptr<AStaticMeshActor> Actor )
{
	StaticMeshActors.push_back(Actor);
}
