#include "StdAfx.h"
#include "EntityManager.h"
#include "ControllerEntity.h"

using namespace std::chrono;

class CEntityManagerEntityRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CEntityManagerEntity>("EntityManager", "Plant_Tools");
		// Flow Node Defaults
		CEntityFlowNodeFactory* pFlowNodeFactory = new CEntityFlowNodeFactory("entity:EntityManager");

		pFlowNodeFactory->m_inputs.push_back(InputPortConfig<EntityId>("EntityManagerId", ""));
		pFlowNodeFactory->m_activateCallback = CEntityManagerEntity::OnFlowgraphActivation;
		pFlowNodeFactory->Close();
	}
	virtual void Unregister() override
	{

	}
};
static CEntityManagerEntityRegistrator g_entityRegistratorLight;

CRYREGISTER_CLASS(CEntityManagerEntity)

void CEntityManagerEntity::OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode)
{
	if (auto* pEntity = gEnv->pEntitySystem->GetEntity(entityId))
	{
		auto* thisEntity = pEntity->GetComponent<CEntityManagerEntity>();
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
void CEntityManagerEntity::AddEventListener(IEntityEventListener* pListener)
{
	// Does not check uniquiness due to performance reasons.
	//m_listeners.push_back(pListener);
}
void CEntityManagerEntity::RemoveEventListener(IEntityEventListener* pListener)
{
	//stl::find_and_erase(m_listeners, pListener);
}
void CEntityManagerEntity::addLight(IEntity* light)
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
IEntity* CEntityManagerEntity::spawnEntity(char* name, char* className, float size, Vec3 pos) {
	SEntitySpawnParams spawnParams;
	spawnParams.sName = name;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);
	spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;
	spawnParams.vScale = Vec3(size, size, size);
	IEntity* entity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	//GetEntity()->AttachChild(entity);
	Matrix34 locMat = entity->GetLocalTM();
	locMat.SetTranslation(pos);
	entity->SetLocalTM(locMat);
	//entity->Hide(true);
	return entity;
}
void CEntityManagerEntity::spawnPlant(Vec3 pos) {
	IEntity* newPlant = spawnEntity("plantEntity", "GBeanPlantCustom", 1.0f, pos);
	if (newPlant)
	{
		CPlantEntityCustom* plantCustom = newPlant->GetComponent<CPlantEntityCustom>();
		if (plantCustom)
		{
			plantCustom->growthSwitch(true);
			plantCustom->SetTime(currentTime, currentTime);
			addPlant(newPlant);
		}
	}
}
void CEntityManagerEntity::spawnRobot(Vec3 pos) {
	IEntity* newRobot = spawnEntity("robotEntity", "Robot", 10.0f, pos);
	if (newRobot)
	{
		addRobot(newRobot);
	}
}
void CEntityManagerEntity::spawnLight(Vec3 pos) {
	IEntity* newLight = spawnEntity("lightEntity", "LightGeom", .5f, pos);
	if (newLight)
	{
		addLight(newLight);
	}
}
void CEntityManagerEntity::spawnSelectedEntity(Vec3 pos) {
	switch (currentSpawnEntity)
	{
	case plant:
		spawnPlant(pos);
		break;
	case light:
		spawnLight(pos);
		break;
	case robot:
		spawnRobot(pos);
		break;
	default:
		break;
	}
}
int CEntityManagerEntity::getSelectedEntity()
{
	return currentSpawnEntity;
}
void CEntityManagerEntity::removeLight(IEntity* light)
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
void CEntityManagerEntity::changeSelectedEntity(int diff)
{
	// keep it in range
	currentSpawnEntity = (currentSpawnEntity + diff) % entityCount;
	if (currentSpawnEntity < 0)
	{
		currentSpawnEntity += entityCount;
	}
}
void CEntityManagerEntity::lightSwitch(IEntity* light, bool on)
{
	if (light != NULL)
	{
		if (on)
		{
			addLight(light);
			for (int i = 0; i < lightOffEntities.size(); i++)
			{
				if (lightOffEntities.at(i)->GetId() == light->GetId())
				{
					lightOffEntities.erase(lightOffEntities.begin() + i);
				}
			}
		}
		else
		{
			for (int i = 0; i < lightEntities.size(); i++)
			{
				if (lightEntities.at(i)->GetId() == light->GetId())
				{
					lightEntities.erase(lightEntities.begin() + i);
					lightOffEntities.push_back(light);
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
void CEntityManagerEntity::addPlant(IEntity* plant)
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
void CEntityManagerEntity::removePlant(IEntity* plant)
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
void CEntityManagerEntity::addRobot(IEntity* robot)
{
	if (robot != NULL)
	{
		robotEntities.push_back(robot);
	}
}
void CEntityManagerEntity::removeRobot(IEntity* robot)
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
void CEntityManagerEntity::resetGrowth()
{
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->Reset();
	}
}
void CEntityManagerEntity::backOneGrowth()
{
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->RemoveSection();
	}
}
void CEntityManagerEntity::grow(bool grow)
{
	globalGrowth = grow;
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->growthSwitch(grow);
	}
}
void CEntityManagerEntity::growSwitch()
{
	globalGrowth = !globalGrowth;
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->growthSwitch(globalGrowth);
	}
}
void CEntityManagerEntity::removeAll()
{

	for (int i = 0; i < allEntities.size(); i++)
	{
		for (int j = 0; j < allEntities.at(i)->size(); j++)
		{
			gEnv->pEntitySystem->RemoveEntity(allEntities.at(i)->at(j)->GetId());
		}
	}

	//for (int i = 0; i < plantEntities.size(); i++)
	//{
	//	gEnv->pEntitySystem->RemoveEntity(plantEntities.at(i)->GetId());
	//}
	//for (int i = 0; i < lightEntities.size(); i++)
	//{
	//	gEnv->pEntitySystem->RemoveEntity(lightEntities.at(i)->GetId());
	//}
	//for (int i = 0; i < lightOffEntities.size(); i++)
	//{
	//	gEnv->pEntitySystem->RemoveEntity(lightOffEntities.at(i)->GetId());
	//}
	//for (int i = 0; i < robotEntities.size(); i++)
	//{
	//	gEnv->pEntitySystem->RemoveEntity(robotEntities.at(i)->GetId());
	//}

	plantEntities.clear();
	lightEntities.clear();
	lightOffEntities.clear();
	robotEntities.clear();
	
}
IEntity* CEntityManagerEntity::getOneLight()
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
bool CEntityManagerEntity::GetGlobalGrowth()
{
	return globalGrowth;
}
void CEntityManagerEntity::attachClosestEntity(IEntity* controller)
{
	IEntity* closest = NULL;
	switch(currentSpawnEntity)
	{
	case plant:
		closest = getClosestEntity("plantEntity", controller->GetWorldPos());
		break;
	case light:
		closest = getClosestEntity("lightEntity", controller->GetWorldPos());
		break;
	case robot:
		closest = getClosestEntity("robotEntity", controller->GetWorldPos());
		break;
	default:
		break;
	}
	
	if (closest)
	{
		Vec3 diffVec = closest->GetWorldPos() - controller->GetWorldPos();
		Quat diffRot = controller->GetRotation();
		controller->GetComponent<CControllerEntity>()->AttachObject(closest, diffVec, diffRot);
		
		// 40 because scale of controller is 40
		// Should somehow avoid setting scale of controller, load geometry scaled instead
		//diffVec.SetLength(diffVec.GetLength() / 40.0f);
		//closest->SetPosRotScale(diffVec, Quat(IDENTITY), closest->GetScale() / 40.0f);
		
		//closest->SetPos(diffVec);
		//closest->SetScale(closest->GetScale() / 40.0f);
		//closest->SetRotation(childRotation);
	}
}
void CEntityManagerEntity::switchClosestLight(IEntity * controller)
{
	IEntity* closest = NULL;
	closest = getClosestEntity("lightEntity", controller->GetWorldPos());

	if (closest)
	{
		lightSwitch(closest, closest->GetComponent<CMyLightGeomEntity>()->lightSwitch());
	}
}
void physicallizeEntity(IEntity* entity)
{
	// do this so it can be grabbed again
	SEntityPhysicalizeParams params;
	params.type = PE_RIGID;
	entity->Physicalize(params);
}
void CEntityManagerEntity::detachEntities(IEntity* controller)
{
	controller->GetComponent<CControllerEntity>()->DetachObjects();
}
IEntity* CEntityManagerEntity::getClosestEntity(string name, Vec3 pos)
{
	float constraintRadius = .2f;
	const Vec3 boxMin = pos - Vec3(constraintRadius + 0.05f);
	const Vec3 boxMax = pos + Vec3(constraintRadius + 0.05f);
	IPhysicalEntity** nearbyEntities = 0;
	IEntity* closest = NULL;
	float* smallestDist = NULL;
	if (size_t entityCount = gEnv->pPhysicalWorld->GetEntitiesInBox(boxMin, boxMax, nearbyEntities, ent_all))
	{
		for (size_t i = 0; i < entityCount; ++i)
		{
			if (IEntity* nearbyEntity = gEnv->pEntitySystem->GetEntityFromPhysics(nearbyEntities[i]))
			{
				string s = nearbyEntity->GetName();
				CryLogAlways(s);
				if (!s.compare(name))
				{
					CryLogAlways(name);
					float dist = pos.GetDistance(nearbyEntity->GetWorldPos());
					if (!smallestDist)
					{
						smallestDist = new float(dist);
						closest = nearbyEntity;
					}
					else if (dist < *smallestDist)
					{
						*smallestDist = dist;
						closest = nearbyEntity;
					}
				}
			}
		}
		return closest;
	}
}

void CEntityManagerEntity::SetFlowGraph(IFlowGraph* pFlowGraph)
{
	if (m_pFlowGraph)
		m_pFlowGraph->Release();
	m_pFlowGraph = pFlowGraph;
	if (m_pFlowGraph)
		m_pFlowGraph->AddRef();
};
IFlowGraph* CEntityManagerEntity::GetFlowGraph() {
	return m_pFlowGraph;
};
void CEntityManagerEntity::Initialize()
{
	GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
	GetEntity()->Activate(true); // necessary for UPDATE event to be called
	allEntities.push_back(&plantEntities);
	allEntities.push_back(&lightEntities);
	allEntities.push_back(&lightOffEntities);
	allEntities.push_back(&robotEntities);
	
	/*
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Light");

	spawnParams.vPosition = GetEntity()->GetPos();
	spawnParams.qRotation = GetEntity()->GetRotation();

	spawnParams.vScale = Vec3(1, 1, 1);

	gEnv->pEntitySystem->SpawnEntity(spawnParams);*/

	// Spawn the entity, bullet is propelled in CBullet based on the rotation and position here

}
void CEntityManagerEntity::UpdateTime(int time)
{
	for (int i = 0; i < plantEntities.size(); i++)
	{
		plantEntities.at(i)->GetComponent<CPlantEntityCustom>()->SetTime(time);
	}
	// add other entities that need it
}
void CEntityManagerEntity::UpdateRelativeTime(int time)
{
	currentTime += time;
	UpdateTime(currentTime);
}
void CEntityManagerEntity::ProcessEvent(SEntityEvent &event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_START_GAME:
		Reset();
		GetEntitiesInScene("plantEntity");
		GetEntitiesInScene("lightEntity");
		GetEntitiesInScene("robotEntity");
		break;
	case ENTITY_EVENT_RESET:
		Reset();
		break;
	case ENTITY_EVENT_UPDATE:
		milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		if (lastGrowth + growthRate < now && globalGrowth) // add another cylinder periodically
		{
			currentTime++;
			UpdateTime(currentTime);
			lastGrowth = now;
		}
		
		break;
	}
}
void CEntityManagerEntity::GetEntitiesInScene(string entityName)
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
				if (!s.compare(entityName))
				{
					CryLogAlways("Found a " + entityName + " already in scene");
					CryLogAlways(s);
					
					if (!entityName.compare("plantEntity"))
					{
						addPlant(nearbyEntity);
					}
					else if (!entityName.compare("lightEntity"))
					{
						addLight(nearbyEntity);
					}
					else if (!entityName.compare("robotEntity"))
					{
						addRobot(nearbyEntity);
					}
					
				}
			}
		}
	}
}

uint64 CEntityManagerEntity::GetEventMask() const {
	return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_GAME) | BIT64(ENTITY_EVENT_RESET);
}


void CEntityManagerEntity::Reset()
{
	lightEntities.clear();
	plantEntities.clear();
	robotEntities.clear();
}