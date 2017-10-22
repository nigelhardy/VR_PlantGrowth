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

class CPlantEntityCustom;

class CLightManagerEntity
	: public IEntityFlowGraphComponent
{
public:

	//{30CD4C05103040D9-BED41FCCB292ACFA}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CLightManagerEntity, "LightManager", 0x30CD4C05103040D9, 0xBED41FCCB292ACFA);
	static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode);
	void AddEventListener(IEntityEventListener* pListener);
	void RemoveEventListener(IEntityEventListener* pListener);

	void addLight(IEntity* light);
	void addPlant(IEntity* plant);
	void addRobot(IEntity* robot);

	void removeLight(IEntity* light);
	void lightSwitch(IEntity* light, bool on);
	void removePlant(IEntity* plant);
	void removeRobot(IEntity* robot);

	void resetGrowth();
	void grow(bool grow);
	void growSwitch();
	void removeAll();
	bool GetGlobalGrowth();

	IEntity* getOneLight();
	
	IFlowGraph* m_pFlowGraph;
	virtual void SetFlowGraph(IFlowGraph* pFlowGraph) override;
	virtual IFlowGraph* GetFlowGraph() override;
	virtual void Initialize() override;
	virtual	void ProcessEvent(SEntityEvent &event);
	void GetLightsInScene();
	void GetPlantsInScene();
	virtual uint64 GetEventMask() const;

protected:

	void Reset();

public:
	std::vector<IEntity*> lightEntities = std::vector<IEntity*>();
	std::vector<IEntity*> plantEntities = std::vector<IEntity*>();
	std::vector<IEntity*> robotEntities = std::vector<IEntity*>();
	bool globalGrowth = true;
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
};