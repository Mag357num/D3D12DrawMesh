//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run(FEngine* pEngine, HINSTANCE hInstance, int nCmdShow)
{
	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"DXSampleClass";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(pEngine->GetWidth()), static_cast<LONG>(pEngine->GetHeight()) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		pEngine->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, // We have no parent window.
		nullptr, // We aren't using menus.
		hInstance,
		pEngine);

	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.
	pEngine->Init((void*)m_hwnd);

	ShowWindow(m_hwnd, nCmdShow);

	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pEngine->Destroy();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	FEngine* pEngine = reinterpret_cast<FEngine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
		{
			// Save the DXSample* passed in to CreateWindow.
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;

	case WM_KEYDOWN:
		if (pEngine)
		{
			pEngine->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_KEYUP:
		if (pEngine)
		{
			pEngine->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_RBUTTONDOWN:
		if (pEngine)
		{
			UINT x = LOWORD(lParam);
			UINT y = HIWORD(lParam);
			pEngine->OnButtonDown(x, y);
		}
		return 0;

	case WM_RBUTTONUP:
		if (pEngine)
		{
			pEngine->OnButtonUp();
		}
		return 0;

	case WM_LBUTTONDOWN:
		if (pEngine)
		{
			UINT x = LOWORD(lParam);
			UINT y = HIWORD(lParam);
			pEngine->OnButtonDown(x, y);
		}
		return 0;

	case WM_LBUTTONUP:
		if (pEngine)
		{
			pEngine->OnButtonUp(); // same with right button
		}
		return 0;

	case WM_MOUSEMOVE:
		if (pEngine)
		{
			UINT x = LOWORD(lParam);
			UINT y = HIWORD(lParam);
			pEngine->OnMouseMove(x, y);
		}
		return 0;

	case WM_PAINT:
		if (pEngine)
		{
			pEngine->CalculateFrameStats();
			pEngine->Tick();
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}
