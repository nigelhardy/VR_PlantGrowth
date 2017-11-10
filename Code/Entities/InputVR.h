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

enum controllerCommands
{
	menu1Down = 0,
	menu1Up,
	trigger1Down,
	trigger1up

};

class Command
{
public:
	virtual void execute() = 0;
	virtual ~Command() {};
};

class CommandQueue
{
	CommandQueue();
	void execute()
	{

	}
	void newCommand(int command)
	{
		if (menu1Down)
		{
			CryLogAlways("Menu 1 Down");
		}
	}
};



class SpawnPlant : public Command
{
public:
	SpawnPlant(CEntityManagerEntity* entityMan, Vec3 pos) 
	{
		spawnPos = pos;
		if (entityMan)
		{
			entityManager = entityMan;
		}
	}
	void execute() override
	{
		if (entityManager)
		{
			entityManager->spawnPlant(spawnPos);
		}
	};
protected:
	CEntityManagerEntity* entityManager;
	Vec3 spawnPos;
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

	virtual uint64 GetEventMask() const {
		return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_LEVEL) | BIT64(ENTITY_EVENT_RESET);
	}

protected:
	bool getEntityManager();
	void getVRDevices();
	void Reset();

	int spawnEntitySelector = 0;
	float lastSpawnEntityChange = 0.f;
	IMaterial *pMat;
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
	bool touchGripLast = false;
	bool touchTrigLast = false;
	bool touchTrackLast = false;
	bool touchMenuLast = false;

	Vec3* movePlayerDest = NULL;
	float lastResetTime = 0;

	IEntity* controllerEntity1 = NULL;
	Vec3 controllerPos1 = Vec3(0);
	Quat controllerRot1 = Quat(IDENTITY);

	IEntity* controllerEntity2 = NULL;
	Vec3 controllerPos2 = Vec3(0);
	Quat controllerRot2 = Quat(IDENTITY);

	std::vector< Command* > commands;

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