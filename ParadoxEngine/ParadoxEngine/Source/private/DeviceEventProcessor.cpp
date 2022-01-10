#include "DeviceEventProcessor.h"

shared_ptr<FDeviceEventProcessor> FDeviceEventProcessor::GDEventProcessor = nullptr;

void FDeviceEventProcessor::CreateEventProcessor()
{
	assert(GDEventProcessor == nullptr);
	GDEventProcessor = make_shared<FDeviceEventProcessor>();
}

void FDeviceEventProcessor::DestroyEventProcessor()
{
	assert(GDEventProcessor != nullptr);
	GDEventProcessor = nullptr;
}

void FDeviceEventProcessor::Tick()
{
	MouseMove_BottonDown_FirstPosition = MouseMove_BottonDown_CurrentPosition;
}

void FDeviceEventProcessor::OnKeyDown(const unsigned char& key)
{
	switch (key)
	{
	case 'W':
		Keys.w = true;
		break;
	case 'A':
		Keys.a = true;
		break;
	case 'S':
		Keys.s = true;
		break;
	case 'D':
		Keys.d = true;
		break;
	case 'Q':
		Keys.q = true;
		break;
	case 'E':
		Keys.e = true;
		break;
	case VK_LEFT:
		Keys.left = true;
		break;
	case VK_RIGHT:
		Keys.right = true;
		break;
	case VK_UP:
		Keys.up = true;
		break;
	case VK_DOWN:
		Keys.down = true;
		break;
	case VK_ESCAPE:
		//Reset();
		break;
	}
}

void FDeviceEventProcessor::OnKeyUp(const unsigned char& key)
{
	switch (key)
	{
	case 'W':
		Keys.w = false;
		break;
	case 'A':
		Keys.a = false;
		break;
	case 'S':
		Keys.s = false;
		break;
	case 'D':
		Keys.d = false;
		break;
	case 'Q':
		Keys.q = false;
		break;
	case 'E':
		Keys.e = false;
		break;
	case VK_LEFT:
		Keys.left = false;
		break;
	case VK_RIGHT:
		Keys.right = false;
		break;
	case VK_UP:
		Keys.up = false;
		break;
	case VK_DOWN:
		Keys.down = false;
		break;
	}
}

void FDeviceEventProcessor::OnButtonDown(const uint32& x, const uint32& y)
{
	bIsMouseDown = true;

	MouseMove_BottonDown_FirstPosition = { static_cast<float>(x), static_cast<float>(y) };
	MouseMove_BottonDown_CurrentPosition = MouseMove_BottonDown_FirstPosition;

}

void FDeviceEventProcessor::OnButtonUp()
{
	bIsMouseDown = false;
}

void FDeviceEventProcessor::OnMouseMove(const uint32& x, const uint32& y)
{
	if (bIsMouseDown)
	{
		MouseMove_BottonDown_CurrentPosition = { static_cast<float>(x), static_cast<float>(y) };
	}
}

FVector2 FDeviceEventProcessor::GetDeltaMouseMove_BottonDown()
{
	FVector2 Result = MouseMove_BottonDown_CurrentPosition - MouseMove_BottonDown_FirstPosition;
	return Result;
}