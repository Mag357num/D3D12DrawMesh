#include "Scene.h"
#include "Engine.h"
#include "Character.h"

void FScene::Tick(StepTimer& Timer)
{
	const float ElaTime = static_cast<float>(Timer.GetElapsedSeconds());

	GetCurrentCharacter()->Tick(ElaTime);
	GetStaticMeshActors()[5]->Tick(ElaTime);
	GetCurrentCamera()->Tick(static_cast<float>(ElaTime), FCameraMoveMode::AROUNDTARGET, GetCurrentCharacter()->GetSkeletalMeshCom()->GetTransform().Translation, 300.f);
}