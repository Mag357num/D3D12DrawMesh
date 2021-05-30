#include "Engine.h"
#include "Camera.h"
#include "DynamicRHI.h"
#include "RenderThread.h"

using namespace Microsoft::WRL;
using RHI::GDynamicRHI;
using RHI::FMesh_deprecated;
using RHI::FMeshRes;

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

	CurrentScene = CreateScene();

	// static mesh actors
	vector<AStaticMeshActor> StaticMeshActors;
	FAssetManager::Get()->LoadStaticMeshActors(L"Resource\\Scene_.dat", StaticMeshActors);
	for (auto i : StaticMeshActors)
	{
		CurrentScene->AddStaticMeshActor(i);
	}

	// camera
	CurrentScene->SetCurrentCamera(make_shared<ACamera>(FVector(1000.f, 0.f, 300.f), FVector(0.f, 0.f, 1.f) , FVector(0.f, 1.f, -0.2f), 0.8f, static_cast<float>(ResoWidth), static_cast<float>(ResoHeight)));

	// light
	CurrentScene->SetDirectionLight(FDirectionLight());

	// character
	shared_ptr<ACharacter> Cha = make_shared<ACharacter>();
	{
		shared_ptr<TSkeletalMeshComponent> SkeMeshCom = make_shared<TSkeletalMeshComponent>();
		{
			SkeMeshCom->SetSkeletalMesh( FAssetManager::Get()->LoadSkeletalMesh( L"Resource\\SkeletalMeshBinary_.dat" ) );
			SkeMeshCom->GetSkeletalMesh()->SetSkeleton( FAssetManager::Get()->LoadSkeleton( L"Resource\\SkeletonBinary_.dat" ) );
			SkeMeshCom->AddSequence( std::pair<string, shared_ptr<FAnimSequence>>( "Run", FAssetManager::Get()->LoadAnimSequence( L"Resource\\SequenceRun_.dat" ) ) );
			SkeMeshCom->AddSequence( std::pair<string, shared_ptr<FAnimSequence>>( "Idle", FAssetManager::Get()->LoadAnimSequence( L"Resource\\SequenceIdle_.dat" ) ) );
			SkeMeshCom->SetTransform( { { 1.f, 1.f, 1.f }, FQuat( EulerToQuat( FEuler( 0.f, 0.f, 0.f ) ) ), { 300.f, 200.f, 0.f } } );
		}
		Cha->SetSkeletalMeshCom( SkeMeshCom );
	}
	CurrentScene->SetCharacter(Cha);

	// thread
	FRenderThread::CreateRenderThread();
	FRenderThread::Get()->Start();
	FRenderThread::Get()->CreateFrameResource(CurrentScene);
}

void FEngine::Tick()
{
	FRenderThread::Get()->WaitForRenderThread();

	Timer.Tick(NULL);
	CurrentScene->Tick(Timer); // all actors store in FScene for now

	// TODO: remove FrameRes concept add into the tick layer, update static mesh's constant buffer when tick them
	FRenderThread::Get()->UpdateFrameRes(CurrentScene.get());
}

void FEngine::Destroy()
{
	FRenderThread::DestroyRenderThread();
}

void FEngine::OnKeyDown(unsigned char Key)
{
	CurrentScene->GetCurrentCamera()->OnKeyDown(Key);
	CurrentScene->GetCharacter()->OnKeyDown(Key);
}

void FEngine::OnKeyUp(unsigned char Key)
{
	CurrentScene->GetCurrentCamera()->OnKeyUp(Key);
	CurrentScene->GetCharacter()->OnKeyUp(Key);
}

void FEngine::OnMouseMove(uint32 x, uint32 y)
{
	CurrentScene->GetCurrentCamera()->OnMouseMove(x, y);
	CurrentScene->GetCharacter()->OnMouseMove(x, y);
}

void FEngine::OnButtonDown(uint32 x, uint32 y)
{
	CurrentScene->GetCurrentCamera()->OnButtonDown(x, y);
	CurrentScene->GetCharacter()->OnButtonDown(x, y);
}

void FEngine::OnButtonUp()
{
	CurrentScene->GetCurrentCamera()->OnButtonUp();
	CurrentScene->GetCharacter()->OnButtonUp();
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
