#include "Scene.h"
#include "Engine.h"

void FScene::Tick(StepTimer& Timer)
{
	GetCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCurrentCamera().Update_AroundTarget(static_cast<float>(Timer.GetElapsedSeconds()), GetCharacter()->GetLocation(), 300.f);
}