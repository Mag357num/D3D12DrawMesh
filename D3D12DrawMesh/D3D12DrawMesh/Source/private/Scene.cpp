#include "Scene.h"
#include "Engine.h"

void TScene::Tick(StepTimer& Timer)
{
	GetCurrentCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCurrentCamera()->Update(static_cast<float>(Timer.GetElapsedSeconds()), FCameraMoveMode::AROUNDTARGET, GetCurrentCharacter()->GetSkeletalMeshCom()->GetTransform().Translation, 300.f);
}

FDirectionLight& TScene::GetDirectionLight()
{
	return DirectionLight;
}

vector<AStaticMeshActor>& TScene::GetStaticMeshActors()
{
	return StaticMeshActors;
}

void TScene::AddStaticMeshActor( AStaticMeshActor Actor )
{
	StaticMeshActors.push_back(Actor);
}
