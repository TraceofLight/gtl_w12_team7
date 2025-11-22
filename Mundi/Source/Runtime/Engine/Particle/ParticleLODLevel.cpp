#include "pch.h"
#include "ParticleLODLevel.h"
#include "ParticleModule.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleTypeDataBase.h"

UParticleLODLevel::UParticleLODLevel()
	: Level(0)
	, bEnabled(true)
	, RequiredModule(nullptr)
	, TypeDataModule(nullptr)
	, SpawnModule(nullptr)
	, PeakActiveParticles(0)
{
}

void UParticleLODLevel::UpdateModuleLists()
{
	// 캐시 초기화
	SpawnModules.clear();
	UpdateModules.clear();

	// 모듈들을 순회하며 분류
	for (UParticleModule* Module : Modules)
	{
		if (!Module)
		{
			continue;
		}

		// 이 LOD 레벨에서 유효한지 확인
		if (!Module->IsValidForLODLevel(Level))
		{
			continue;
		}

		// Spawn 모듈 캐시
		if (Module->IsSpawnModule())
		{
			SpawnModules.Add(Module);
		}

		// Update 모듈 캐시
		if (Module->IsUpdateModule())
		{
			UpdateModules.Add(Module);
		}
	}
}

int32 UParticleLODLevel::CalculateMaxActiveParticleCount()
{
	if (!RequiredModule || !SpawnModule)
	{
		return 0;
	}

	// 기본 계산: 생성률 * 수명
	float Duration = RequiredModule->GetEmitterDurationValue();
	float SpawnRate = SpawnModule->Rate.Max;
	float Lifetime = 1.0f;  // 기본 수명

	// Lifetime 모듈에서 최대 수명 찾기
	for (UParticleModule* Module : Modules)
	{
		// UParticleModuleLifetime 체크 (간단히 이름으로)
		if (Module && Module->GetClass()->Name &&
			strcmp(Module->GetClass()->Name, "UParticleModuleLifetime") == 0)
		{
			// Lifetime 모듈의 최대값 사용
			// 실제로는 캐스팅해서 값을 가져와야 함
			Lifetime = 5.0f;  // 임시 값
			break;
		}
	}

	// 버스트 파티클 수 추가
	int32 BurstTotal = 0;
	for (const FParticleBurst& Burst : SpawnModule->BurstList)
	{
		BurstTotal += Burst.Count;
	}

	// 최대 파티클 수 계산
	int32 MaxCount = static_cast<int32>(SpawnRate * Lifetime) + BurstTotal;

	// 안전 마진 추가 (20%)
	MaxCount = static_cast<int32>(static_cast<float>(MaxCount) * 1.2f);

	// 최소값 보장 및 MaxDrawCount 제한
	MaxCount = std::max(10, MaxCount);

	int32 MaxDrawCount = RequiredModule->GetMaxDrawCount();
	if (MaxDrawCount > 0)
	{
		MaxCount = std::min(MaxDrawCount, MaxCount);
	}

	PeakActiveParticles = MaxCount;
	return MaxCount;
}

int32 UParticleLODLevel::GetModuleIndex(UParticleModule* InModule)
{
	for (int32 i = 0; i < static_cast<int32>(Modules.size()); ++i)
	{
		if (Modules[i] == InModule)
		{
			return i;
		}
	}
	return -1;
}

UParticleModule* UParticleLODLevel::GetModuleAtIndex(int32 InIndex)
{
	if (InIndex >= 0 && InIndex < static_cast<int32>(Modules.size()))
	{
		return Modules[InIndex];
	}
	return nullptr;
}

void UParticleLODLevel::SetLevelIndex(int32 InLevelIndex)
{
	Level = InLevelIndex;

	// 모든 모듈의 LOD 유효성 업데이트
	if (RequiredModule)
	{
		RequiredModule->SetLODValidity(Level, true);
	}

	if (SpawnModule)
	{
		SpawnModule->SetLODValidity(Level, true);
	}

	if (TypeDataModule)
	{
		TypeDataModule->SetLODValidity(Level, true);
	}

	for (UParticleModule* Module : Modules)
	{
		if (Module)
		{
			Module->SetLODValidity(Level, true);
		}
	}
}

bool UParticleLODLevel::IsModuleEditable(UParticleModule* InModule)
{
	if (!InModule)
	{
		return false;
	}

	// 이 LOD 레벨에서 유효한지 확인
	return InModule->IsValidForLODLevel(Level);
}
