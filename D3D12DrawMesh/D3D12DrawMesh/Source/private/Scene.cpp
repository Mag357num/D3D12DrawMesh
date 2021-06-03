#include "Scene.h"
#include "Engine.h"

void FScene::Tick(StepTimer& Timer)
{
	const float ElaTime = static_cast<float>(Timer.GetElapsedSeconds());

	GetCurrentCharacter()->Tick(ElaTime);
	GetStaticMeshActors()[6]->Tick(ElaTime);
	GetCurrentCamera()->Tick(static_cast<float>(ElaTime), FCameraMoveMode::AROUNDTARGET, GetCurrentCharacter()->GetSkeletalMeshCom()->GetTransform().Translation, 300.f);
}

void FScene::SetDirectionLight(shared_ptr<ADirectionLight> Light)
{
	DirectionLight = Light;
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
