#pragma once
#include "StdAfx.h"
#include <CryEntitySystem\IEntityComponent.h>
#include <CryEntitySystem\IEntitySystem.h>
#include <CryEntitySystem\IEntityClass.h>
#include <CrySerialization\Decorators\Resources.h>
#include "Helpers\ISimpleExtension.h"
#include <CryEntitySystem\IEntity.h>
#include <chrono>
#include "Helpers/EntityFlowNode.h"
#include <CrySerialization/Enum.h>
#include <CrySystem\VR\IHMDDevice.h>
#include <CrySystem\VR\IHMDManager.h>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include "GamePlugin.h"
#include <vector>
#include "EntityManager.h"
#include "CryParticleSystem\IParticles.h"
#include "LightGeom.h"
#include "robot.h"
#include <limits>
#include <vector>
#include "ControllerEntity.h"

class Command
{
public:
	virtual void execute() = 0;
	virtual ~Command() {};
};

class SpawnEntityCommand : public Command
{
public:
	SpawnEntityCommand(CEntityManagerEntity* entityMan, Vec3 pos)
	{
		spawnPos = pos;
		entityManager = entityMan;
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->spawnSelectedEntity(spawnPos);
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	Vec3 spawnPos;
};

class RemoveAllEntitiesCommand : public Command
{
public:
	RemoveAllEntitiesCommand(CEntityManagerEntity* entityMan)
	{
		entityManager = entityMan;
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->removeAll();
		}
	};
protected:
	CEntityManagerEntity* entityManager;
};

class AttachClosestEntityCommand : public Command
{
public:
	AttachClosestEntityCommand(CEntityManagerEntity* entityMan, IEntity* cont, string objectName)
	{
		controller = cont;
		entityManager = entityMan;
		name = objectName;
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->attachClosestEntity(controller);
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	IEntity* controller;
	string name;
};


class SwitchClosestLight : public Command
{
public:
	SwitchClosestLight(CEntityManagerEntity* entityMan, IEntity* cont)
	{
		controller = cont;
		entityManager = entityMan;
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->switchClosestLight(controller);
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	IEntity* controller;
};

class DetachEntitiesCommand : public Command
{
public:
	DetachEntitiesCommand(CEntityManagerEntity* entityMan, IEntity* controller)
	{
		entityManager = entityMan;
		cont = controller;
	}
	void execute() override
	{
		if (entityManager)
		{
			cont->GetComponent<CControllerEntity>()->DetachObjects();
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	IEntity* cont;
};

class RemoveGrowth : public Command
{
public:
	RemoveGrowth(CEntityManagerEntity* entityMan)
	{
		entityManager = entityMan;
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->backOneGrowth();
		}
	};
protected:
	CEntityManagerEntity* entityManager;
};

class GrowthSwitch : public Command
{
public:
	GrowthSwitch(CEntityManagerEntity* entityMan, bool grow)
	{
		entityManager = entityMan;
		growStatus = grow;
		switchGrowth = false;
	}
	GrowthSwitch(CEntityManagerEntity* entityMan)
	{
		entityManager = entityMan;
	}
	void execute() override
	{
		if (entityManager && switchGrowth)
		{
			entityManager->growSwitch();
		}
		else if (entityManager)
		{
			entityManager->grow(growStatus);
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	bool growStatus;
	bool switchGrowth = true;
};

class ChangeTime : public Command
{
public:
	ChangeTime(CEntityManagerEntity* entityMan, int time)
	{
		entityManager = entityMan;
		setTime = time;
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->UpdateRelativeTime(setTime);
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	int setTime;
};

class ShowMovementPlaneCommand : public Command
{
public:
	ShowMovementPlaneCommand(IEntity* movementPlane, IEntity* movementWand, bool showPlane)
	{
		plane = movementPlane;
		wand = movementWand;
		// true will hide it, so reverse logic
		hide = !showPlane;
	}
	void execute() override
	{
		if (plane)
		{
			plane->Hide(hide);
		}
		if (wand)
		{
			wand->Hide(hide);
		}
	};
protected:
	IEntity* plane;
	IEntity* wand;
	bool hide;
};

class MovePlayerMarkerCommand : public Command
{
public:
	MovePlayerMarkerCommand(IEntity* controller, IEntity* playerMarker, bool showPlane)
	{
		marker = playerMarker;
		cont = controller;
		// true will hide it, so reverse logic
		hide = !showPlane;
	}
	void execute() override
	{
		if (cont && !hide)
		{
			ray_hit rayHit;
			Vec3 origin = cont->GetWorldPos();
			Vec3 dir = cont->GetForwardDir();
			int numHits = gEnv->pPhysicalWorld->RayWorldIntersection(origin, dir * 10.0f, ent_static, rwi_stop_at_pierceable, &rayHit, 1);
			if (numHits > 0)
			{
				if (rayHit.surface_idx == 103)
				{
					if (marker)
					{
						Matrix34 ind = marker->GetWorldTM();
						ind.SetTranslation(rayHit.pt + Vec3(-.3f, -.3f, 0.0f));
						marker->SetWorldTM(ind);
						marker->Hide(hide);
					}
				}
				else
				{
					// didn't hit movement plane, so hide player marker
					marker->Hide(true);
				}
			}
			
		}
		else if (hide && marker) {
			marker->Hide(hide);
		}
	};
protected:
	IEntity* marker;
	IEntity* cont;
	bool hide;
};

class MovePlayerCommand : public Command
{
public:
	MovePlayerCommand(IEntity* controller, IEntity* playerMarker, Vec3** movePlayerDest, Vec3 hmdPos)
	{
		cont = controller;
		pm = playerMarker;
		playerDest = movePlayerDest;
		hmdPosVec = hmdPos;
	}
	void execute() override
	{
		if (cont)
		{
			ray_hit rayHit;
			Vec3 origin = cont->GetWorldPos();
			Vec3 dir = cont->GetForwardDir();
			int numHits = gEnv->pPhysicalWorld->RayWorldIntersection(origin, dir * 10.0f, ent_static, rwi_stop_at_pierceable, &rayHit, 1);
			if (numHits > 0)
			{
				if (rayHit.surface_idx == 103)
				{
					if (pm)
					{
						Matrix34 ind = pm->GetWorldTM();
						ind.SetTranslation(rayHit.pt + Vec3(-.3f, -.3f, 0.0f));
						pm->SetWorldTM(ind);
					}
					
					Vec3* playerPos = new Vec3(Vec3(rayHit.pt - Vec3(hmdPosVec.x, hmdPosVec.y, 0.0f)));
					*playerDest = playerPos;
					
				}
			}
		}
	};
protected:
	IEntity* cont;
	IEntity* pm;
	Vec3** playerDest;
	Vec3 hmdPosVec;
};

class ChangeEntityCommand : public Command
{
public:
	ChangeEntityCommand(CEntityManagerEntity* entityMan, int diff, IEntity* controller)
	{
		entityManager = entityMan;
		difference = diff;
		cont = controller;
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->changeSelectedEntity(difference);
			// set controller color/mode
			cont->GetComponent<CControllerEntity>()->setMode(entityManager->getSelectedEntity());
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	IEntity* cont;
	int difference;
};


using namespace std::chrono;

class CInputVREntity
	: public IEntityFlowGraphComponent
	, public IEntityPropertyGroup
{
public:
	static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode);

	IFlowGraph* m_pFlowGraph;
	virtual void SetFlowGraph(IFlowGraph* pFlowGraph) override;
	virtual IFlowGraph* GetFlowGraph() override;
	// {{25D1A6E5290048E8-9A7202192918DFAE}}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CInputVREntity, "InputVR", 0x25D1A6E5290048E8, 0x9A7202192918DFAE);

	void AddEventListener(IEntityEventListener* pListener);
	void RemoveEventListener(IEntityEventListener* pListener);

	virtual void Initialize() override;

	virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

	virtual const char* GetLabel() const override { return "InputVR"; }
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	virtual	void ProcessEvent(SEntityEvent &event);

	void getControllerInput();
	bool getOffsetEntity();
	IEntity* spawnController(char* name);
	void updateControllersLocation();
	void processCommands();
	bool getMovementPlane();
	bool getMovementWand();
	bool getPlayerMarker();


	virtual uint64 GetEventMask() const {
		return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_LEVEL) | BIT64(ENTITY_EVENT_RESET);
	}

protected:
	bool getEntityManager();
	void getVRDevices();
	void Reset();

	int spawnEntitySelector = 0;
	float lastSpawnEntityChange = 0.f;

	string m_geometry = "objects/controller/vive_controller_ce.cgf";
	const IHmdController* pController;
	IHmdDevice* pDevice;
	float scale = 40.0f;
	Vec3 hmdPos = Vec3(0, 0, 0);
	IEntitySystem *m_pIEntitySystem;
	IEntity *m_pOffsetEntity;
	CEntityManagerEntity *entityManager = NULL;
	bool leftController = true;
	Vec3 posArea;
	Vec3 nextPlayerPos = Vec3(0, 0, 0);
	size_t areaId = 0;
	IArea* a = NULL;
	EHmdController currentController;
	bool touchGripLast1 = false;
	bool touchTrigLast1 = false;
	bool touchTrackLast1 = false;
	bool touchMenuLast1 = false;

	bool touchGripLast2 = false;
	bool touchTrigLast2 = false;
	bool touchTrackLast2 = false;
	bool touchMenuLast2 = false;

	milliseconds touchGripLastHeld2 = (milliseconds)0;


	Vec3* movePlayerDest = NULL;
	float lastResetTime = 0;

	IEntity* controllerEntity1 = NULL;
	Vec3 controllerPos1 = Vec3(0);
	Quat controllerRot1 = Quat(IDENTITY);

	IEntity* controllerEntity2 = NULL;
	Vec3 controllerPos2 = Vec3(0);
	Quat controllerRot2 = Quat(IDENTITY);

	std::vector< Command* > commands;

	IEntity* playerMarker = NULL;
	IEntity* movementPlane = NULL;
	IEntity* movementWand = NULL;
public:
	enum EInputPorts
	{
		eInputPort_itemHeld = 0,
		eInputPort_MoveHmd

	};

	enum EOutputPorts
	{
		// placeholders right now
		eOutputPort_MovePlayer = 0,
	};
};