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
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include "Entities/PlantEntityCustom.h"
#include "GamePlugin.h"
#include "LightGeom.h"
enum modes
{
	plantMode = 0,
	lightMode,
	robotMode,
};

class CPlantEntityCustom;

class CEntityManagerEntity
	: public IEntityFlowGraphComponent
{
public:

	//{30CD4C05103040D9-BED41FCCB292ACFA}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CEntityManagerEntity, "EntityManager", 0x30CD4C05103040D9, 0xBED41FCCB292ACFA);
	static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode);
	void AddEventListener(IEntityEventListener* pListener);
	void RemoveEventListener(IEntityEventListener* pListener);

	void addLight(IEntity* light);
	void addPlant(IEntity* plant);
	void addRobot(IEntity* robot);


	void spawnPlant(Vec3 pos);
	void spawnRobot(Vec3 pos);
	void spawnLight(Vec3 pos);
	IEntity* spawnEntity(char* name, char* className, float size, Vec3 pos);
	void spawnSelectedEntity(Vec3 pos);

	void removeLight(IEntity* light);
	void lightSwitch(IEntity* light, bool on);
	void removePlant(IEntity* plant);
	void removeRobot(IEntity* robot);

	void resetGrowth();
	void backOneGrowth();
	void grow(bool grow);
	void growSwitch();
	void removeAll();
	bool GetGlobalGrowth();

	void attachClosestEntity(IEntity* controller);

	void switchClosestLight(IEntity* controller);

	void detachEntities(IEntity* controller);
	IEntity* getClosestEntity(string name, Vec3 pos);
	void changeSelectedEntity(int diff);
	int getSelectedEntity();
	void UpdateTime(int time);
	void UpdateRelativeTime(int time);

	IEntity* getOneLight();
	
	IFlowGraph* m_pFlowGraph;
	virtual void SetFlowGraph(IFlowGraph* pFlowGraph) override;
	virtual IFlowGraph* GetFlowGraph() override;
	virtual void Initialize() override;
	virtual	void ProcessEvent(SEntityEvent &event);
	void GetEntitiesInScene(string entityName);
	virtual uint64 GetEventMask() const;

protected:

	void Reset();
	int currentSpawnEntity = plant;

public:
	std::vector<std::vector<IEntity*>*> allEntities;
	std::vector<IEntity*> lightEntities;
	std::vector<IEntity*> lightOffEntities;
	std::vector<IEntity*> plantEntities;
	std::vector<IEntity*> robotEntities;
	bool globalGrowth = true;
	int currentTime = 0;
	milliseconds lastGrowth;
	milliseconds growthRate = (milliseconds)100;

	enum EInputPorts
	{
		eInputPort_LightPosition = 0,
	};

	enum EOutputPorts
	{
		// placeholders right now
		eOutputPort_OnHide = 0,
		eOutputPort_OnUnHide,
		eOutputPort_OnCollision,
		eOutputPort_CollisionSurfaceName,
		eOutputPort_OnGeometryChanged
	};
	enum entityType
	{
		plant = 0,
		light,
		robot,
		entityCount = 3
	};

};