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
#include <ctime>
#include "GamePlugin.h"
#include "Entities/Helpers/ISimpleExtension.h"
#include <vector>
#include <map>
struct plant_piece
{
	IRenderNode* cyl = NULL;
	Matrix34 rot;
	Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 offset;
};
using namespace std::chrono;
class CPlantEntityComponent
	: public CGameObjectExtensionHelper<CPlantEntityComponent, ISimpleExtension>
	, public IEntityPropertyGroup
	, IActionListener
{
public:

	//static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode)
	//{
	//	if (auto* pEntity = gEnv->pEntitySystem->GetEntity(entityId))
	//	{
	//		auto* plantEntity = pEntity->GetComponent<CPlantEntityComponent>();
	//		if (IsPortActive(pActInfo, eInputPort_LightPosition)) // get light position
	//		{
	//			plantEntity->lightPosition = GetPortVec3(pActInfo, eInputPort_LightPosition);
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_LightIntensity)) // get light intensity
	//		{
	//			plantEntity->lightIntensity = GetPortFloat(pActInfo, eInputPort_LightIntensity);
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_Grow)) // check if growth is active
	//		{
	//			plantEntity->growActive = GetPortBool(pActInfo, eInputPort_Grow);
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_Reset)) // reset triggered from flow graph
	//		{
	//			plantEntity->resetGrowth();
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_TriggerGrow)) // growth triggered by flowgraph
	//		{
	//			plantEntity->updateGrowth();
	//		}
	//	}
	//}

	//{{81E16295AA3B456E-947EC994AD923E18}}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CPlantEntityComponent, "GPlantEntity", 0x81E16295AA3B456E, 0x947EC994AD923E18);
	virtual void OnAction(const ActionId &action, int activationMode, float value) override;

	//IFlowGraph* m_pFlowGraph;
	/*virtual void SetFlowGraph(IFlowGraph* pFlowGraph) override
	{
	if (m_pFlowGraph)
	m_pFlowGraph->Release();
	m_pFlowGraph = pFlowGraph;
	if (m_pFlowGraph)
	m_pFlowGraph->AddRef();
	};
	virtual IFlowGraph* GetFlowGraph() override {
	return m_pFlowGraph;
	};*/
	void AddEventListener(IEntityEventListener* pListener);
	void RemoveEventListener(IEntityEventListener* pListener);
	double CPlantEntityComponent::mMR(double d, int x);
	bool CPlantEntityComponent::cNEQ(Quat a, Quat b, int digis);
	virtual void Initialize() override;

	virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

	virtual const char* GetLabel() const override { return "GPlantEntity"; }
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	virtual uint64 GetEventMask() const;
	virtual	void ProcessEvent(SEntityEvent &event);
	void logVector(string s, Vec3 v)
	{
		string r = s;
		r.Append(" X: ");
		r.Append(std::to_string(v.x).c_str());
		r.Append(" Y: ");
		r.Append(std::to_string(v.y).c_str());
		r.Append(" Z: ");
		r.Append(std::to_string(v.z).c_str());
		CryLogAlways(r);
	}
	void updateGrowth();
	void resetGrowth();
	void growthSwitch(bool active);
	void growthSwitch();
	void setLight(IEntity* l);
	void setGrowthRate(int millis);
	void removeLight(IEntity* light);
	void updateLeadTarget();
	Quat rotatePosition(Vec3* target);
	void growSections();
	void Reset();
protected:

	std::vector<IEntity*> lights = std::vector<IEntity*>();
	// random growth things
	Vec3 lastSectionPos = Vec3(0, 0, .13f);
	Quat lastSectionRot = Quat(IDENTITY);

	Quat lastSectionRotation = Quat(IDENTITY);
	IActionMapManager *pActionMapManager;
	milliseconds lastGrowth;
	milliseconds lastGrowthSection;
	milliseconds growthRate = (milliseconds)100;
	int geometryCounter = 2; // counter for slots of geometry
	IMaterial *pMat;
	IMaterial *potMat;
	IMaterial *invisMat;
	float plantScale = .1f;
	float shrink = 1.0f;
	Vec3* lightPos = NULL; // set dynamically via FlowGraph
	float lightIntensity = 1.0f; // set dynamically via FlowGraph
	bool growActive = false;
	float sectionScale = .1f;
	std::map<int, plant_piece> plant_pieces;
	Vec3* leadTargetPos = NULL;
	// Handler for actionmap events that maps actions to callbacks
	TActionHandler<CPlantEntityComponent> m_actionHandler;
	string m_geometry = "objects/default/custom_plant_cyl.cgf";
	string m_potGeometry = "objects/pot/pot.cgf";
	string m_potProxy = "objects/plant_cylinder.cgf";
	bool proxySet = false;


public:
	enum EInputPorts
	{
		eInputPort_LightPosition = 0,
		eInputPort_LightIntensity,
		eInputPort_Grow,
		eInputPort_Reset,
		eInputPort_TriggerGrow
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

class CPlantEntityRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CPlantEntityComponent>("GBeanPlant", "GPlantEntities");

		//// Flow Node Defaults
		//CEntityFlowNodeFactory* pFlowNodeFactory = new CEntityFlowNodeFactory("entity:GBeanPlant");

		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<Vec3>("LightPosition", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<float>("LightIntensity", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<bool>("GrowActive", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<bool>("ResetGrowth", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<bool>("TriggerGrow", ""));
		//pFlowNodeFactory->m_activateCallback = CPlantEntityComponent::OnFlowgraphActivation;

		///*
		//placeholders right now, used for sending information back to flowgraph
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig_Void("OnHide"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig_Void("OnUnHide"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig<EntityId>("OnCollision"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig<string>("CollisionSurfaceName"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig<string>("OnGeometryChanged"));*/

		//pFlowNodeFactory->Close();
	}
	virtual void Unregister() override
	{

	}
};
static CPlantEntityRegistrator g_entityRegistratorPlant;