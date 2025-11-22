#include "pch.h"
#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModule.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleTypeDataBase.h"

UParticleEmitter::UParticleEmitter()
	: EmitterName("Emitter")
	, bDisabledLODsKeepEmitterAlive(false)
	, bCollapsed(false)
	, DetailMode(0)
	, EmitterRenderMode(EEmitterRenderMode::Normal)
	, EmitterEditorColor(1.0f, 1.0f, 1.0f, 1.0f)
	, InitialAllocationCount(0)
	, bIsSoloing(false)
	, PeakActiveParticles(0)
	, RequiredBytes(0)
	, ReqInstanceBytes(0)
	, bRequiresLoopNotification(false)
	, bAxisLockEnabled(false)
	, LockAxisFlags(EParticleAxisLock::None)
	, TypeDataModule(nullptr)
{
}

void UParticleEmitter::CacheEmitterModuleInfo()
{
	// 캐시 초기화
	PeakActiveParticles = 0;
	RequiredBytes = 0;
	ReqInstanceBytes = 0;
	bRequiresLoopNotification = false;
	bAxisLockEnabled = false;
	LockAxisFlags = EParticleAxisLock::None;
	TypeDataModule = nullptr;

	if (LODLevels.empty())
	{
		return;
	}

	// LOD 0을 기준으로 정보 캐싱
	UParticleLODLevel* LODLevel = LODLevels[0];
	if (!LODLevel)
	{
		return;
	}

	// 모듈 리스트 업데이트
	LODLevel->UpdateModuleLists();

	// TypeData 모듈 캐시
	TypeDataModule = LODLevel->TypeDataModule;

	// 최대 활성 파티클 수 계산
	PeakActiveParticles = LODLevel->CalculateMaxActiveParticleCount();

	// 필요 바이트 계산
	for (UParticleModule* Module : LODLevel->Modules)
	{
		if (Module)
		{
			RequiredBytes += Module->RequiredBytes(TypeDataModule);
			ReqInstanceBytes += Module->RequiredBytesPerInstance();
		}
	}

	// Required 모듈에서 추가 정보 캐싱
	if (LODLevel->RequiredModule)
	{
		// 루프 알림 필요 여부 (Duration이 0이 아닌 경우)
		if (LODLevel->RequiredModule->GetEmitterDurationValue() > 0.0f)
		{
			bRequiresLoopNotification = true;
		}

		// 축 고정 정보
		EParticleAxisLock AxisLock = LODLevel->RequiredModule->GetAxisLockOption();
		if (AxisLock != EParticleAxisLock::None)
		{
			bAxisLockEnabled = true;
			LockAxisFlags = AxisLock;
		}
	}

	// 모든 LOD 레벨의 PeakActiveParticles 계산
	for (int32 i = 1; i < static_cast<int32>(LODLevels.size()); ++i)
	{
		if (LODLevels[i])
		{
			LODLevels[i]->UpdateModuleLists();
			int32 LODPeak = LODLevels[i]->CalculateMaxActiveParticleCount();
			PeakActiveParticles = std::max(PeakActiveParticles, LODPeak);
		}
	}
}

UParticleLODLevel* UParticleEmitter::GetLODLevel(int32 LODIndex)
{
	if (LODIndex >= 0 && LODIndex < static_cast<int32>(LODLevels.size()))
	{
		return LODLevels[LODIndex];
	}
	return nullptr;
}

void UParticleEmitter::SetEmitterName(const FString& InName)
{
	EmitterName = InName;
}

bool UParticleEmitter::AutogenerateLowestLODLevel()
{
	// 최소 1개의 LOD 레벨이 필요
	if (LODLevels.empty())
	{
		return false;
	}

	// 이미 여러 LOD 레벨이 있으면 생성하지 않음
	if (LODLevels.size() > 1)
	{
		return false;
	}

	// LOD 0을 기반으로 새 LOD 레벨 생성
	UParticleLODLevel* SourceLOD = LODLevels[0];
	if (!SourceLOD)
	{
		return false;
	}

	// 새 LOD 레벨 생성 (간소화된 버전)
	UParticleLODLevel* NewLOD = new UParticleLODLevel();
	NewLOD->Level = 1;
	NewLOD->bEnabled = true;

	// 필수 모듈 복사 (같은 인스턴스 공유)
	NewLOD->RequiredModule = SourceLOD->RequiredModule;
	NewLOD->SpawnModule = SourceLOD->SpawnModule;
	NewLOD->TypeDataModule = SourceLOD->TypeDataModule;

	// 모듈 복사 (LOD 1에서 일부 모듈 비활성화 가능)
	for (UParticleModule* Module : SourceLOD->Modules)
	{
		if (Module)
		{
			NewLOD->Modules.Add(Module);
		}
	}

	NewLOD->SetLevelIndex(1);
	LODLevels.Add(NewLOD);

	return true;
}
