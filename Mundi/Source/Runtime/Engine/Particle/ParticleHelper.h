#pragma once
#include "Particle.h"

/*-----------------------------------------------------------------------------
	Particle Sorting Helper
-----------------------------------------------------------------------------*/
struct FParticleOrder
{
	int32 ParticleIndex;

	union
	{
		float Z;
		uint32 C;
	};

	FParticleOrder(int32 InParticleIndex, float InZ) :
		ParticleIndex(InParticleIndex),
		Z(InZ)
	{
	}

	FParticleOrder(int32 InParticleIndex, uint32 InC) :
		ParticleIndex(InParticleIndex),
		C(InC)
	{
	}
};

#define DECLARE_PARTICLE(Name,Address)		\
	FBaseParticle& Name = *((FBaseParticle*) (Address));
