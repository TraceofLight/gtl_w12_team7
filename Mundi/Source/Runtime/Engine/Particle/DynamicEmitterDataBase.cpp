#include "pch.h"
#include "DynamicEmitterDataBase.h"

#include "ParticleTypes.h"
#include "VertexData.h"
#include "ParticleHelper.h"
#include "SceneView.h"

void FDynamicSpriteEmitterDataBase::SortSpriteParticles(EParticleSortMode SortMode, bool bLocalSpace,
	int32 ParticleCount, const uint8* ParticleData, int32 ParticleStride, const uint16* ParticleIndices,
	const FSceneView* View, const FMatrix& LocalToWorld, FParticleOrder* ParticleOrder) const
{

	if (SortMode == EParticleSortMode::ViewProjDepth)
	{
		for (int32 ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
			float InZ;
			if (bLocalSpace)
			{
				InZ = View->GetViewProjectionMatrix().TransformPositionVector4(LocalToWorld.TransformPosition(Particle.Location)).W;
			}
			else
			{
				InZ = View->GetViewProjectionMatrix().TransformPositionVector4(Particle.Location).W;
			}
			ParticleOrder[ParticleIndex].ParticleIndex = ParticleIndex;

			ParticleOrder[ParticleIndex].Z = InZ;
		}
		std::sort(ParticleOrder, ParticleOrder + ParticleCount, [](const FParticleOrder& A, const FParticleOrder& B)
		{
			return A.Z > B.Z;
		}); // 내림차순 정렬
	}
	else if (SortMode == EParticleSortMode::DistanceToView)
	{
		for (int32 ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
			float InZ;
			FVector Position;
			if (bLocalSpace)
			{
				Position = LocalToWorld.TransformPosition(Particle.Location);
			}
			else
			{
				Position = Particle.Location;
			}
			InZ = (View->ViewLocation - Position).SizeSquared();
			ParticleOrder[ParticleIndex].ParticleIndex = ParticleIndex;
			ParticleOrder[ParticleIndex].Z = InZ;
		}
		std::sort(ParticleOrder, ParticleOrder + ParticleCount, [](const FParticleOrder& A, const FParticleOrder& B)
		{
			return A.Z > B.Z;
		}); // 내림차순 정렬
	}
	else if (SortMode == EParticleSortMode::Age_OldestFirst)
	{
		for (int32 ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
			ParticleOrder[ParticleIndex].ParticleIndex = ParticleIndex;
			ParticleOrder[ParticleIndex].C = Particle.Flags & STATE_CounterMask;
		}
		std::sort(ParticleOrder, ParticleOrder + ParticleCount, [](const FParticleOrder& A, const FParticleOrder& B)
		{
			return A.C > B.C;
		}); // 내림차순 정렬
	}
	else if (SortMode == EParticleSortMode::Age_NewestFirst)
	{
		for (int32 ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
			ParticleOrder[ParticleIndex].ParticleIndex = ParticleIndex;
			ParticleOrder[ParticleIndex].C = (~Particle.Flags) & STATE_CounterMask;
		}
		std::sort(ParticleOrder, ParticleOrder + ParticleCount, [](const FParticleOrder& A, const FParticleOrder& B)
		{
			return A.C > B.C;
		}); // 내림차순 정렬
	}
}

int32 FDynamicSpriteEmitterData::GetDynamicVertexStride() const
{
	return sizeof(FParticleSpriteVertex);
}

int32 FDynamicMeshEmitterData::GetDynamicVertexStride() const
{
	return sizeof(FMeshParticleInstanceVertex);
}
