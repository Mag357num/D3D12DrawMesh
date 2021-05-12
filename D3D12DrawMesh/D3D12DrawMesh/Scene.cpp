#include "Scene.h"
#include "Engine.h"

void FScene::UpdateMainCamera(class FEngine* Engine)
{
	StepTimer& Timer = Engine->GetTimer();
	Timer.Tick(NULL);
	GetCurrentCamera().Update(static_cast<float>(Timer.GetElapsedSeconds()));
}

void FScene::ManipulateCamera(unsigned char Key, uint32 X, uint32 Y, SceneCameraManipulateType Type)
{
	switch (Type)
	{
	case SceneCameraManipulateType::KEY_UP:
		CurrentCamera.OnKeyUp(Key);
		break;
	case SceneCameraManipulateType::KEY_DOWN:
		CurrentCamera.OnKeyDown(Key);
		break;
	case SceneCameraManipulateType::MOUSEMOVE:
		CurrentCamera.OnMouseMove(X, Y);
		break;
	case SceneCameraManipulateType::RIGHT_BUTTON_DOWN:
		CurrentCamera.OnRightButtonDown(X, Y);
		break;
	case SceneCameraManipulateType::RIGHT_BUTTON_UP:
		CurrentCamera.OnRightButtonUp();
		break;
	default:
		break;
	}
}
