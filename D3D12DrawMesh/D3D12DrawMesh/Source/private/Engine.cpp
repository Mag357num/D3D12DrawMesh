#include "Engine.h"
#include "Camera.h"
#include "DynamicRHI.h"
#include "RenderThread.h"
#include "DeviceEventProcessor.h"
#include "Camera.h"
#include "Character.h"
#include "StaticMesh.h"
#include "SkeletalMesh.h"
#include "Light.h"
#include "Material.h"

using namespace Microsoft::WRL;
using RHI::GDynamicRHI;

FEngine* GEngine = nullptr;

FEngine::FEngine(uint32 width, uint32 height, std::wstring name) :
	ResoWidth(width),
	ResoHeight(height),
	Title(name),
	IsUseWarpDevice(false)
{
	GEngine = this;
	AspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

FEngine::~FEngine()
{
}

void FEngine::Init(void* WindowHandle)
{
	HWindow = WindowHandle;

	// RHI
	RHI::GDynamicRHI->CreateRHI();
	RHI::GDynamicRHI->RHIInit();

	// asset manager
	FAssetManager::CreateAssetManager();
	FDeviceEventProcessor::CreateEventProcessor();

	// create scene
	CurrentScene = CreateScene();

	// create or init scene actors
	{
		// create materials for actor component
		shared_ptr<FMaterial> Empty = make_shared<FMaterial>();
		shared_ptr<FMaterial> Translucent = make_shared<FMaterial>();
		{
			Empty->SetBlendMode(FBlendMode::OPAQUE_BM);
			Empty->SetShader(L"XXX.hlsl");

			Translucent->SetBlendMode(FBlendMode::TRANSLUCENT_BM);
			Translucent->SetShader(L"XXX.hlsl");
			Translucent->AddFloatParam(0.5f); // opacity
		}

		// static mesh actors
		vector<shared_ptr<AStaticMeshActor>> StaticMeshActors;
		FAssetManager::Get()->LoadStaticMeshActors(L"Resource\\Scene_.dat", StaticMeshActors);
		for (auto i : StaticMeshActors)
		{
			i->GetStaticMeshComponent()->SetMaterial(Empty);
			CurrentScene->AddStaticMeshActor(i);
		}

		// camera
		CurrentScene->SetCurrentCamera(make_shared<ACamera>(FVector(1000.f, 0.f, 300.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, -0.2f), 0.8f, static_cast<float>(ResoWidth), static_cast<float>(ResoHeight)));

		// light
		{
			// directional light
			shared_ptr<ADirectionalLight> DirectionalLight = make_shared<ADirectionalLight>(FVector(1000.f, 0.f, 1000.f), FVector(-1.f, 0.f, -1.f));
			DirectionalLight->SetOrthoParam(-1200.f, 1200.f, -1200.f, 1200.f, 1.0f, 3000.0f); // TODO: hard coding
			CurrentScene->SetDirectionalLight(DirectionalLight);

			// point light
			shared_ptr<APointLight> PointLight1 = make_shared<APointLight>(FVector(120, 380, 160));
			CurrentScene->AddPointLight(PointLight1);
		}

		// character
		shared_ptr<ACharacter> Cha = make_shared<ACharacter>();
		{
			shared_ptr<FSkeletalMeshComponent> SkeMeshCom = make_shared<FSkeletalMeshComponent>();
			{
				SkeMeshCom->SetSkeletalMesh(FAssetManager::Get()->LoadSkeletalMesh(L"Resource\\SkeletalMeshBinary_.dat"));
				SkeMeshCom->GetSkeletalMesh()->SetSkeleton(FAssetManager::Get()->LoadSkeleton(L"Resource\\SkeletonBinary_.dat"));
				SkeMeshCom->AddSequence(std::pair<string, shared_ptr<FAnimSequence>>("Run", FAssetManager::Get()->LoadAnimSequence(L"Resource\\SequenceRun_.dat")));
				SkeMeshCom->AddSequence(std::pair<string, shared_ptr<FAnimSequence>>("Idle", FAssetManager::Get()->LoadAnimSequence(L"Resource\\SequenceIdle_.dat")));
				SkeMeshCom->SetTransform(FTransform(FVector(1.f, 1.f, 1.f), FQuat(EulerToQuat(FEuler(0.f, 0.f, 0.f))), FVector(0.f, -700.f, 0.f)));
			}
			Cha->SetSkeletalMeshCom(SkeMeshCom);
		}
		CurrentScene->SetCurrentCharacter(Cha);
	}

	// thread
	FRenderThread::CreateRenderThread();
	FRenderThread::Get()->Start();
	FRenderThread::Get()->CreateFrameResource(CurrentScene);
}

void FEngine::Tick()
{
	// GAME tick
	Timer.Tick(NULL);
	CurrentScene->Tick(Timer); // all actors store in FScene for now
	FDeviceEventProcessor::Get()->Tick();

	FRenderThread::Get()->WaitForRenderer();
	FRenderThread::Get()->UpdateFrameRes(CurrentScene.get());
}

void FEngine::Destroy()
{
	FAssetManager::DestroyAssetManager();
	FDeviceEventProcessor::DestroyEventProcessor();
	FRenderThread::DestroyRenderThread();
	FDynamicRHI::DestroyRHI();
}

void FEngine::OnKeyDown(unsigned char Key)
{
	FDeviceEventProcessor::Get()->OnKeyDown(Key);
}

void FEngine::OnKeyUp(unsigned char Key)
{
	FDeviceEventProcessor::Get()->OnKeyUp(Key);
}

void FEngine::OnMouseMove(uint32 x, uint32 y)
{
	FDeviceEventProcessor::Get()->OnMouseMove(x, y);
}

void FEngine::OnButtonDown(uint32 x, uint32 y)
{
	FDeviceEventProcessor::Get()->OnButtonDown(x, y);
}

void FEngine::OnButtonUp()
{
	FDeviceEventProcessor::Get()->OnButtonUp();
}

void FEngine::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	std::wstring mMainWndCaption = L"d3d App";

	frameCnt++;

	// Compute averages over one second period.
	if ((Timer.GetTotalSeconds() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		wstring fpsStr = std::to_wstring(fps);
		wstring mspfStr = std::to_wstring(mspf);

		wstring windowText = mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText((HWND)HWindow, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
