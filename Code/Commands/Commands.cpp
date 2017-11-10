#include "Commands.h"
#include "StdAfx.h"

SpawnPlant::SpawnPlant(CEntityManagerEntity* entityMan, Vec3 pos)
{
	spawnPos = pos;
	if (entityMan)
	{
		entityManager = entityMan;
	}
}
void SpawnPlant::execute() {
	if (entityManager)
	{
		entityManager->spawnPlant(spawnPos);
	}
}