#include "Scene.h"
#include "Engine.h"

void FScene::Tick(StepTimer& Timer)
{
	GetCurrentCamera().Tick(static_cast<float>(Timer.GetElapsedSeconds()));
	GetCharacter()->Tick(static_cast<float>(Timer.GetElapsedSeconds()));
}

shared_ptr<ACharacter> FScene::CreateCharacter(const std::wstring& SkeletalMeshFileName)
{
	shared_ptr<ACharacter> Character = make_shared<ACharacter>();
	shared_ptr<FSkeletalMeshComponent> SkeMeshCom = make_shared<FSkeletalMeshComponent>();
	shared_ptr<FSkeletalMesh> SkeMesh = ;


	Character->SetSkeletalMeshCom(SkeMeshCom);
	return Character;
}
