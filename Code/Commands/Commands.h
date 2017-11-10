#pragma once

#include "StdAfx.h"
#include "Entities\EntityManager.h"

class Command
{
public:
	virtual void execute() = 0;
};


class SpawnPlant : public Command
{
public:
	//SpawnPlant(CEntityManagerEntity* entityMan, Vec3 pos);
	void execute() override;
protected:
	CEntityManagerEntity* entityManager;
	Vec3 spawnPos;
};