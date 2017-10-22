#include "StdAfx.h"
#include "Entities\LightManager.h"

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

using namespace std::chrono;
class CLightManagerEntity
	: public IEntityFlowGraphComponent
{
public:

	//{30CD4C05103040D9-BED41FCCB292ACFA}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CLightManagerEntity, "LightManager", 0x30CD4C05103040D9, 0xBED41FCCB292ACFA);
	static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode)
	{
		if (auto* pEntity = gEnv->pEntitySystem->GetEntity(entityId))
		{
			auto* thisEntity = pEntity->GetComponent<CLightManagerEntity>();
			if (IsPortActive(pActInfo, eInputPort_LightPosition))
			{
				EntityId tmp = GetPortEntityId(pActInfo, eInputPort_LightPosition);
				auto* pEntity = gEnv->pEntitySystem->GetEntity(tmp);
				thisEntity->lightEntities.push_back(pEntity);
			}
		}
		
	}
	void AddEventListener(IEntityEventListener* pListener)
	{
		// Does not check uniquiness due to performance reasons.
		//m_listeners.push_back(pListener);
	}
	void RemoveEventListener(IEntityEventListener* pListener)
	{
		//stl::find_and_erase(m_listeners, pListener);
	}
	void addLight(IEntity* light)
	{
		lightEntities.push_back(light);
		std::string tmp = std::to_string(light->GetPos().x);
		string s = light->GetName();
		s.append("  ");
		s.append(tmp.c_str());
		CryLogAlways(s);
	}
	IFlowGraph* m_pFlowGraph;
	virtual void SetFlowGraph(IFlowGraph* pFlowGraph) override
	{
		if (m_pFlowGraph)
			m_pFlowGraph->Release();
		m_pFlowGraph = pFlowGraph;
		if (m_pFlowGraph)
			m_pFlowGraph->AddRef();
	};
	virtual IFlowGraph* GetFlowGraph() override {
		return m_pFlowGraph;
	};
	virtual void Initialize() override
	{
		
		GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
		GetEntity()->Activate(true); // necessary for UPDATE event to be called
		
		/*
		SEntitySpawnParams spawnParams;
		spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Light");

		spawnParams.vPosition = GetEntity()->GetPos();
		spawnParams.qRotation = GetEntity()->GetRotation();

		spawnParams.vScale = Vec3(1, 1, 1);

		gEnv->pEntitySystem->SpawnEntity(spawnParams);*/

		// Spawn the entity, bullet is propelled in CBullet based on the rotation and position here

	}
	virtual	void ProcessEvent(SEntityEvent &event)
	{
		switch (event.event)
		{
		case ENTITY_EVENT_START_LEVEL:
			Reset();
			break;
		case ENTITY_EVENT_RESET:
			Reset();
			break;
		case ENTITY_EVENT_UPDATE:
			if (lightEntities.size() > 0)
			{
				std::string tmp = std::to_string(lightEntities.back()->GetPos().x);
				string s = lightEntities.back()->GetName();
				s.append("  ");
				s.append(tmp.c_str());
				CryLogAlways(s);
				lightEntities.pop_back();
			}
			break;
		}
	}

	virtual uint64 GetEventMask() const {
		return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_LEVEL) | BIT64(ENTITY_EVENT_RESET);
	}

protected:

	void Reset()
	{

	}

public:
	std::vector<EntityId> lightEntityIds;
	std::vector<IEntity*> lightEntities;
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

CRYREGISTER_CLASS(CLightManagerEntity)

struct IEntityRegistrator
{
	IEntityRegistrator()
	{
		if (g_pFirst == nullptr)
		{
			g_pFirst = this;
			g_pLast = this;
		}
		else
		{
			g_pLast->m_pNext = this;
			g_pLast = g_pLast->m_pNext;
		}
	}

	virtual void Register() = 0;
	virtual void Unregister() = 0;

public:
	IEntityRegistrator *m_pNext;

	static IEntityRegistrator *g_pFirst;
	static IEntityRegistrator *g_pLast;
};
class CMyLightManagerEntityRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CLightManagerEntity>("LightManager", "Plant_Tools");
		// Flow Node Defaults
		CEntityFlowNodeFactory* pFlowNodeFactory = new CEntityFlowNodeFactory("entity:LightManager");

		pFlowNodeFactory->m_inputs.push_back(InputPortConfig<EntityId>("LightEntityId", ""));
		pFlowNodeFactory->m_activateCallback = CLightManagerEntity::OnFlowgraphActivation;
		pFlowNodeFactory->Close();
	}
	virtual void Unregister() override
	{

	}
};
static CMyLightManagerEntityRegistrator g_entityRegistrator;