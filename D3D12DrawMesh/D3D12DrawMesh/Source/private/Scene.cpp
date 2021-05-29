#include "Scene.h"
#include "Engine.h"

void TScene::Tick(StepTimer& Timer)
{
	GetCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCurrentCamera()->Update(static_cast<float>(Timer.GetElapsedSeconds()), FCameraMoveMode::AROUNDTARGET, GetCharacter()->GetLocation(), 300.f);
}

const FDirectionLight& TScene::GetDirectionLight() const
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
