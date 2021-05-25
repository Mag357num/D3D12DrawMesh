#include "Scene.h"
#include "Engine.h"

void FScene::Tick(StepTimer& Timer)
{
	GetCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCurrentCamera().Update(static_cast<float>(Timer.GetElapsedSeconds()), FCameraMoveMode::AROUNDTARGET, GetCharacter()->GetLocation(), 300.f);
}