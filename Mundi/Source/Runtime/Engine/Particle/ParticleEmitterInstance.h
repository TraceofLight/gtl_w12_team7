#pragma once
#include "pch.h"
#define DECLARE_PARTICLE_PTR \
	/* 지금 활성화된 개수(ActivateParticles)가 곧 새로운 파티클이 들어갈 자리 */ \
	int32 CurrentIndex = ParticleIndices[ActivateParticles]; \
	\
	/* Stride를 곱해서 정확한 메모리 번지로 점프 */ \
	uint8* Ptr = ParticleData + (CurrentIndex * ParticleStride); \
	\
	/* FBaseParticle 타입으로 캐스팅해서 Particle이라는 이름의 참조 변수 생성 */\
	FBaseParticle& Particle = *((FBaseParticle*)Ptr); 

// Forward declarations
class UParticleEmitter;
class UParticleSystemComponent;
class UParticleLODLevel;
struct FParticleEventInstancePayload;

/**
 * Runtime instance of a particle emitter
 * Manages particle spawning, updating, and lifetime for a single emitter
 */
struct FParticleEmitterInstance
{
	/** Template emitter that this instance is based on */
	// �� �ν��Ͻ��� ����� �� ���赵 (���� ����)
	UParticleEmitter* SpriteTemplate;

	/** Owner particle system component */
	// ���� ���� ���(Location) �ִ��� �˷��� Component�� ������� ��.
	UParticleSystemComponent* Component;

	// ============== LOD ==============
	/** Current LOD level index being used */
	// ���� ��� ���� LOD �ܰ� �ε���
	int32 CurrentLODLevelIndex;

	/** Current LOD level being used */
	// ���� LOD �ܰ��� �������� (���⿡ ����� ��� ����Ʈ�� �������)
	// ex) ���� ī�޶�� �Ӵϱ� LOD 1�� �Ŵ���(��� 2���� ���)��� �۾��Ѵ�.
	UParticleLODLevel* CurrentLODLevel;

	// ============== �޸� ���� ==============
	// FParticleDataContainer ���� �����͵� ĳ��: ���ټӵ� ����ȭ

	/** Pointer to the particle data array */
	// ���� ��ƼŬ �����͵��� ����� �޸� ����� ���� �ּ�
	uint8* ParticleData;

	/** Pointer to the particle index array */
	// ����ִ� ��ƼŬ���� ��ȣ(Index)�� ���� �迭
	// ex) 3, 7, 9�� ��ƼŬ�� ��������ϱ� ��׸� ������Ʈ
	uint16* ParticleIndices;

	/** Pointer to the instance data array */
	// �ν��Ͻ��� ������ (��ƼŬ ���� ������ ����, ������ ��ü ���� �� ��)
	uint8* InstanceData;

	// ============== �޸� ���� ==============
	// uint8* �����Ϳ��� ��Ȯ�� ��ġ�� ã�� ���� ������

	/** The size of the Instance data array in bytes */
	// �ν��Ͻ� ������ ũ��
	int32 InstancePayloadSize;

	/** The offset to the particle data in bytes */
	// ��ƼŬ ������ ������ ��� ������(Payload)�� ���۵Ǵ� ������
	int32 PayloadOffset;

	/** The total size of a single particle in bytes */
	// �⺻ ��ƼŬ(FBaseParticle) �ϳ��� ũ�� (������)
	int32 ParticleSize;

	/** The stride between particles in the ParticleData array in bytes */
	// ��ƼŬ �ϳ��� �����ϴ� ��¥ �� ũ�� (�⺻ + ��� ������)
	// �̹��� �ν��Ͻ� ������ ��ƼŬ���� Stride�� ����
	// ex) �̹� ��ƼŬ�� �⺻ 50 ����Ʈ�� �÷� ��� 16����Ʈ �߰��ؼ� �� 66����Ʈ ����(Stride)
	// Data + (Index * ParticleStride)
	int32 ParticleStride;

	// ============== ���� ����(State) ==============
	/** The number of particles currently active in the emitter */
	// Ȱ��ȭ�� ��ƼŬ�� ����, Loop ����ȭ
	int32 ActiveParticles;

	/** Monotonically increasing counter for particle IDs */
	// ��ƼŬ ���� ID �ο��� ���� ī���� (��� ������ ��)
	uint32 ParticleCounter;

	/** The maximum number of active particles that can be held in the particle data array */
	// �޸� Ǯ���� ���� ������ �ִ� ��ƼŬ ����
	int32 MaxActiveParticles;

	/** The fraction of time left over from spawning (for sub-frame spawning accuracy) */
	// ���� ������ ������ ���� �ð� ��� ���� ����
	// �ʴ� 30 ������ �����Ѵٰ� ��������. �ٵ� �������� 60FPS (DeltaTime = 0.016s)
	// �̹� �����ӿ� �����ؾ� �� ���� = 30 x 0.016 = 0.48����
	// 0.48������ ������ �� �����Ƿ� �� ������ 0.48 ����
	// ���� �����ӿ��� 0.96 ���̶� ������ �� ��� �� ����
	// �ٴ��� �����ӿ� 1.44�� �Ǿ 1���� �����ϰ� 0.44�� ����� ���
	// �̰� �־�� ��ƼŬ�� �ε巴�� �̾����� ����
	float SpawnFraction;

	FParticleEmitterInstance()
		: SpriteTemplate(nullptr)
		, Component(nullptr)
		, CurrentLODLevelIndex(0)
		, CurrentLODLevel(nullptr)
		, ParticleData(nullptr)
		, ParticleIndices(nullptr)
		, InstanceData(nullptr)
		, InstancePayloadSize(0)
		, PayloadOffset(0)
		, ParticleSize(0)
		, ParticleStride(0)
		, ActiveParticles(0)
		, ParticleCounter(0)
		, MaxActiveParticles(0)
		, SpawnFraction(0.0f)
	{
	}

	~FParticleEmitterInstance()
	{
		// TODO: Implement cleanup logic
	}

	/**
	 * Spawns particles in the emitter
	 * @param Count - Number of particles to spawn
	 * @param StartTime - Starting time for the first particle
	 * @param Increment - Time increment between each particle spawn
	 * @param InitialLocation - Initial location for spawned particles
	 * @param InitialVelocity - Initial velocity for spawned particles
	 * @param EventPayload - Event payload data (optional)
	 */
	void SpawnParticles(
		int32 Count,
		float StartTime,
		float Increment,
		const FVector& InitialLocation,
		const FVector& InitialVelocity,
		FParticleEventInstancePayload* EventPayload = nullptr
	)
	{
		// ������ üũ
		if (!CurrentLODLevel || !ParticleData || !ParticleIndices)
		{
			return;
		}

		// ���� ����
		for (int32 i = 0; i < Count; i++)
		{
			// �� ���� �� �̻� ���� �� �ǵ���
			if (ActiveParticles >= MaxActiveParticles)
			{
				break;
			}

			// ��ũ�θ� ����ؼ� Particle ���� ����
			DECLARE_PARTICLE_PTR

			// �̹� ��ƼŬ�� ���� �ð� ���
			float SpawnTime = StartTime + (i * Increment);
			float Interp = 0.0f; // ������ (���������ӿ�)

			// PreSpawn: �⺻�� �ʱ�ȭ
			Particle.Location = InitialLocation;
			Particle.Velocity = InitialVelocity;
			Particle.BaseVelocity = InitialVelocity;
			Particle.RelativeTime = 0.0f;
			Particle.Lifetime = 1.0f; // �⺻ 1�� (��⿡�� ���)
			Particle.Rotation = 0.0f;
			Particle.RotationRate = 0.0f;
			Particle.Size = FVector::One();
			Particle.Color = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

			// TODO: Spawn ���� ���� (���߿� UParticleLODLevel ���� �� Ȱ��ȭ)
			// for (int32 ModuleIndex = 0; ModuleIndex < CurrentLODLevel->SpawnModules.Num(); ModuleIndex++)
			// {
			//     UParticleModule* Module = CurrentLODLevel->SpawnModules[ModuleIndex];
			//     if (Module && Module->bEnabled)
			//     {
			//         Module->Spawn(this, PayloadOffset, SpawnTime);
			//     }
			// }

			// PostSpawn: ���������� ���� �� ���
			// ������ �߰��� �¾�ٸ� �̵�������
			if (SpawnTime > 0.0f)
			{
				Particle.Location += Particle.Velocity * SpawnTime;
			}

			// Ȱ�� ��ƼŬ ���� ���� (�߿�!)
			ActiveParticles++;

			// ���� ID ����
			ParticleCounter++;
		}
	}

	/**
	 * Kills a particle at the specified index
	 * @param Index - Index of the particle to kill
	 */
	void KillParticle(int32 Index)
	{
		// ���� üũ
		if (Index < 0 || Index >= ActiveParticles)
		{
			return;
		}

		// [�ٽ� ���̵��] �迭�� ������ ��ƼŬ�� �ڸ��� �ٲٰ� ActiveParticles�� ����
		// ��: [0, 1, 2, 3, 4] ���� 2���� ���̸� -> [0, 1, 4, 3] �� �ǰ� ActiveParticles = 4
		// �̷��� �ϸ� �߰��� �� ������ �� ���� (�޸� ȿ��)

		if (Index < ActiveParticles - 1)
		{
			// ���� ��ƼŬ�� �ε����� ������ ��ƼŬ�� �ε����� ��ȯ
			uint16 Temp = ParticleIndices[Index];
			ParticleIndices[Index] = ParticleIndices[ActiveParticles - 1];
			ParticleIndices[ActiveParticles - 1] = Temp;
		}

		// Ȱ�� ��ƼŬ ���� ����
		ActiveParticles--;
	}

	/**
	 * Update all active particles
	 * @param DeltaTime - Time elapsed since last update
	 */
	void Tick(float DeltaTime)
	{
		if (!ParticleData || !ParticleIndices || ActiveParticles <= 0)
		{
			return;
		}

		// �������� ��ȸ (�߰��� ���̸� �ε����� ���̴ϱ�)
		for (int32 i = ActiveParticles - 1; i >= 0; i--)
		{
			// ��ƼŬ ������ ��������
			int32 ParticleIndex = ParticleIndices[i];
			uint8* ParticlePtr = ParticleData + (ParticleIndex * ParticleStride);
			FBaseParticle& Particle = *((FBaseParticle*)ParticlePtr);

			// ���� ������Ʈ
			Particle.RelativeTime += DeltaTime / Particle.Lifetime;

			// �׾����� üũ
			if (Particle.RelativeTime >= 1.0f)
			{
				KillParticle(i);
				continue;
			}

			// ���� ������Ʈ
			Particle.Location += Particle.Velocity * DeltaTime;
			Particle.Rotation += Particle.RotationRate * DeltaTime;

			// TODO: Update ���� ���� (���߿� UParticleLODLevel ���� �� Ȱ��ȭ)
			// for (UParticleModule* Module : CurrentLODLevel->UpdateModules)
			// {
			//     if (Module && Module->bEnabled)
			//     {
			//         Module->Update(this, PayloadOffset, DeltaTime);
			//     }
			// }
		}
	}

	/**
	 * Initialize the emitter instance
	 * @param InTemplate - Template emitter to use
	 * @param InComponent - Owner component
	 */
	void Init(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent)
	{
		SpriteTemplate = InTemplate;
		Component = InComponent;

		// TODO: ���߿� ���ø����� MaxParticles ������ �����ͼ� �޸� �Ҵ�
		// MaxActiveParticles = InTemplate->GetMaxParticleCount();
		// ParticleSize = sizeof(FBaseParticle);
		// ParticleStride = ParticleSize; // ��� ������ �⺻ ũ�⸸

		ActiveParticles = 0;
		ParticleCounter = 0;
		SpawnFraction = 0.0f;
	}
};
