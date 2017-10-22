#include "StdAfx.h"
#include "LightManager.h"

using namespace std::chrono;

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
static CMyLightManagerEntityRegistrator g_entityRegistratorLight;

CRYREGISTER_CLASS(CLightManagerEntity)

void CLightManagerEntity::OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode)
{
	if (auto* pEntity = gEnv->pEntitySystem->GetEntity(entityId))
	{
		auto* thisEntity = pEntity->GetComponent<CLightManagerEntity>();
		if (IsPortActive(pActInfo, eInputPort_LightPosition))
		{
			EntityId tmp = GetPortEntityId(pActInfo, eInputPort_LightPosition);
			auto* pEntity = gEnv->pEntitySystem->GetEntity(tmp);
			if (thisEntity != NULL)
			{
				thisEntity->lightEntities.push_back(pEntity);
			}
		}
	}
		
}
void CLightManagerEntity::AddEventListener(IEntityEventListener* pListener)
{
	// Does not check uniquiness due to performance reasons.
	//m_listeners.push_back(pListener);
}
void CLightManagerEntity::RemoveEventListener(IEntityEventListener* pListener)
{
	//stl::find_and_erase(m_listeners, pListener);
}
void CLightManagerEntity::addLight(IEntity* light)
{
	if (light != NULL)
	{
		lightEntities.push_back(light);
		for (int i = 0; i < plantEntities.size(); i++)
		{
			plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->setLight(light);
		}
	}
	
}
void CLightManagerEntity::removeLight(IEntity* light)
{
	if (light != NULL)
	{
		for (int i = 0; i < lightEntities.size(); i++)
		{
			if (lightEntities.at(i)->GetId() == light->GetId())
			{
				lightEntities.erase(lightEntities.begin() + i);
				for (int i = 0; i < plantEntities.size(); i++)
				{
					plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->removeLight(light);
				}
				gEnv->pEntitySystem->RemoveEntity(light->GetId());
				
				return;
			}
		}
		gEnv->pEntitySystem->RemoveEntity(light->GetId());
	}

}
void CLightManagerEntity::lightSwitch(IEntity* light, bool on)
{
	if (light != NULL)
	{
		if (on)
		{
			addLight(light);
		}
		else
		{
			for (int i = 0; i < lightEntities.size(); i++)
			{
				if (lightEntities.at(i)->GetId() == light->GetId())
				{
					lightEntities.erase(lightEntities.begin() + i);
					for (int i = 0; i < plantEntities.size(); i++)
					{
						plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->removeLight(light);
					}
					return;
				}
			}
		}
	}

}
void CLightManagerEntity::addPlant(IEntity* plant)
{
	if (plant != NULL)
	{
		plantEntities.push_back(plant);
		CPlantEntityCustom *temp = plant->GetComponent<CPlantEntityCustom>();
		if (temp)
		{
			temp->growthSwitch(globalGrowth);
		}
		else
		{
			return;
		}
		if (lightEntities.size() > 0)
		{
			for (int i = 0; i < lightEntities.size(); i++)
			{
				plant->GetComponent<CPlantEntityCustom>()->setLight(lightEntities.at(i));
			}
		}
		
	}
}
void CLightManagerEntity::removePlant(IEntity* plant)
{
	if (plant != NULL)
	{
		for (int i = 0; i < plantEntities.size(); i++)
		{
			if (plantEntities.at(i)->GetId() == plant->GetId())
			{
				plantEntities.erase(plantEntities.begin() + i);
				gEnv->pEntitySystem->RemoveEntity(plant->GetId());
				return;
			}
		}
	}
}
void CLightManagerEntity::addRobot(IEntity* robot)
{
	if (robot != NULL)
	{
		robotEntities.push_back(robot);
	}
}
void CLightManagerEntity::removeRobot(IEntity* robot)
{
	if (robot != NULL)
	{
		for (int i = 0; i < robotEntities.size(); i++)
		{
			if (robotEntities.at(i)->GetId() == robot->GetId())
			{
				robotEntities.erase(robotEntities.begin() + i);
				gEnv->pEntitySystem->RemoveEntity(robot->GetId());
				return;
			}
		}
	}
}
void CLightManagerEntity::resetGrowth()
{
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->Reset();
	}
}
void CLightManagerEntity::grow(bool grow)
{
	globalGrowth = grow;
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->growthSwitch(grow);
	}
}
void CLightManagerEntity::growSwitch()
{
	globalGrowth = !globalGrowth;
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->growthSwitch(globalGrowth);
	}
}
void CLightManagerEntity::removeAll()
{
	for (int i = 0; i < plantEntities.size(); i++)
	{
		gEnv->pEntitySystem->RemoveEntity(plantEntities.at(i)->GetId());
	}
	for (int i = 0; i < lightEntities.size(); i++)
	{
		gEnv->pEntitySystem->RemoveEntity(lightEntities.at(i)->GetId());
	}
	for (int i = 0; i < robotEntities.size(); i++)
	{
		gEnv->pEntitySystem->RemoveEntity(robotEntities.at(i)->GetId());
	}
	lightEntities.clear();
	plantEntities.clear();
	robotEntities.clear();
}
IEntity* CLightManagerEntity::getOneLight()
{
	if (lightEntities.size() > 0)
	{
		return lightEntities.back();
	}
	else
	{
		return NULL;
	}
	
}
bool CLightManagerEntity::GetGlobalGrowth()
{
	return globalGrowth;
}
void CLightManagerEntity::SetFlowGraph(IFlowGraph* pFlowGraph)
{
	if (m_pFlowGraph)
		m_pFlowGraph->Release();
	m_pFlowGraph = pFlowGraph;
	if (m_pFlowGraph)
		m_pFlowGraph->AddRef();
};
IFlowGraph* CLightManagerEntity::GetFlowGraph() {
	return m_pFlowGraph;
};
void CLightManagerEntity::Initialize()
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
void CLightManagerEntity::ProcessEvent(SEntityEvent &event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_START_GAME:
		Reset();
		GetLightsInScene();
		GetPlantsInScene();
		break;
	case ENTITY_EVENT_RESET:
		Reset();
		break;
	case ENTITY_EVENT_UPDATE:
		/*if (lightEntities.size() > 0)
		{
			std::string tmp = std::to_string(lightEntities.back()->GetPos().x);
			string s = lightEntities.back()->GetName();
			s.append("  ");
			s.append(tmp.c_str());
			CryLogAlways(s);
			lightEntities.pop_back();
		}*/
		break;
	}
}
void CLightManagerEntity::GetLightsInScene()
{
	float constraintRadius = 10.0f;
	const Vec3 boxMin = GetEntity()->GetWorldPos() - Vec3(constraintRadius + 5.f);
	const Vec3 boxMax = GetEntity()->GetWorldPos() + Vec3(constraintRadius + 5.f); 
	IPhysicalEntity** nearbyEntities = 0;
	if (size_t entityCount = gEnv->pPhysicalWorld->GetEntitiesInBox(boxMin, boxMax, nearbyEntities, ent_all))
	{
		for (size_t i = 0; i < entityCount; ++i)
		{
			if (IEntity* nearbyEntity = gEnv->pEntitySystem->GetEntityFromPhysics(nearbyEntities[i]))
			{
				string s = nearbyEntity->GetName();
				if (!s.compare("lightGeom"))
				{
					CryLogAlways("Found a light already in scene");
					CryLogAlways(s);
					addLight(nearbyEntity);
				}
			}
		}
	}
}
void CLightManagerEntity::GetPlantsInScene()
{
	float constraintRadius = 10.0f;
	const Vec3 boxMin = GetEntity()->GetWorldPos() - Vec3(constraintRadius + 5.f);
	const Vec3 boxMax = GetEntity()->GetWorldPos() + Vec3(constraintRadius + 5.f);
	IPhysicalEntity** nearbyEntities = 0;
	if (size_t entityCount = gEnv->pPhysicalWorld->GetEntitiesInBox(boxMin, boxMax, nearbyEntities, ent_all))
	{
		for (size_t i = 0; i < entityCount; ++i)
		{
			if (IEntity* nearbyEntity = gEnv->pEntitySystem->GetEntityFromPhysics(nearbyEntities[i]))
			{
				string s = nearbyEntity->GetName();
				if (!s.compare("beanPlant"))
				{
					CryLogAlways("Found a plant already in scene");
					CryLogAlways(s);
					addPlant(nearbyEntity);
				}
			}
		}
	}
}
uint64 CLightManagerEntity::GetEventMask() const {
	return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_GAME) | BIT64(ENTITY_EVENT_RESET);
}


void CLightManagerEntity::Reset()
{
	lightEntities.clear();
	plantEntities.clear();
	robotEntities.clear();
}