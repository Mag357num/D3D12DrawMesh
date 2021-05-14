#include "Scene.h"
#include "Engine.h"

void FScene::Tick(StepTimer& Timer)
{
	GetCurrentCamera().Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
}