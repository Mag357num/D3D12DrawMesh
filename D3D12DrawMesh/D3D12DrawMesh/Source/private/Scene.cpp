#include "Scene.h"
#include "Engine.h"
#include "Character.h"

void FScene::Tick(StepTimer& Timer)
{
	const float ElaTime = static_cast<float>(Timer.GetElapsedSeconds());

	// TODO: deprecated
	GetCurrentCamera()->Tick(static_cast<float>(ElaTime), FCameraMoveMode::AROUNDTARGET, FVector( 200.f, 200.f, 200.f), 300.f);

	//GetCurrentCharacter()->Tick(ElaTime);
	//GetStaticMeshActors()[5]->Tick_ActorRotate(ElaTime);
	//GetCurrentCamera()->Tick(static_cast<float>(ElaTime), FCameraMoveMode::AROUNDTARGET, GetCurrentCharacter()->GetSkeletalMeshComponent()->GetTransform().Translation, 300.f);
}