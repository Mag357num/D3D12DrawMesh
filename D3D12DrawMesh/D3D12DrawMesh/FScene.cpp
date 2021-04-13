#include "FScene.h"
#include "Engine.h"

void FScene::UpdateMainCamera(class FEngine* Engine)
{
	StepTimer& Timer = Engine->GetTimer();
	Timer.Tick(NULL);
	SceneCamera.Update(static_cast<float>(Timer.GetElapsedSeconds()));
}