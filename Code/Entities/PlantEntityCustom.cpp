#include "StdAfx.h"
#include "PlantEntityCustom.h"
#include <math.h>

void CPlantEntityCustom::OnAction(const ActionId &action, int activationMode, float value)
{
	// This function is called when inputs trigger action map events
	// The handler below maps the event (see 'action') to a callback, further below in this file.
	m_actionHandler.Dispatch(this, GetEntityId(), action, activationMode, value);
}	

void CPlantEntityCustom::AddEventListener(IEntityEventListener* pListener)
{
	// Does not check uniquiness due to performance reasons.
	//m_listeners.push_back(pListener);
}
void CPlantEntityCustom::RemoveEventListener(IEntityEventListener* pListener)
{
	//stl::find_and_erase(m_listeners, pListener);
}

void CPlantEntityCustom::Initialize()
{		
	lastGrowth = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	potMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/pot_mtl_multi.mtl");
	potMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(potMat);
	invisMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/invis.mtl");
	invisMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(invisMat);
	GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
	GetEntity()->Activate(true); // necessary for UPDATE event to be called

	GetEntity()->LoadGeometry(0, m_potGeometry);
	GetEntity()->SetSlotMaterial(0, potMat);
	GetEntity()->LoadGeometry(1, m_potProxy);
	GetEntity()->SetSlotMaterial(1, invisMat);

	SEntityPhysicalizeParams params;
	params.type = PE_RIGID;
	GetEntity()->Physicalize(params);
	srand(time(NULL));
	branches.push_back(GrowBranch(GetEntity()->GetPos() + Vec3(0, 0, .13f), GetEntity()->GetRotation(), NULL));
}
CBranchEntity* CPlantEntityCustom::GrowBranch(Vec3 pos, Quat rot, CBranchEntity* cbChild)
{
	SEntitySpawnParams spawnParams;
	spawnParams.sName = "Branch_";
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Branch");
	spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;
	spawnParams.vScale = Vec3(1.0f);
	IEntity* entity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	GetEntity()->AttachChild(entity);
	entity->SetLocalTM(Matrix34::CreateIdentity());
	CBranchEntity* cb = entity->GetComponent<CBranchEntity>();
	cb->SetParent(GetEntity());
	cb->SetRoot(pos, rot);
	if (cbChild)
	{
		cbChild->addChildBranch(cb);
	}
	return cb;
}
CBranchEntity* CPlantEntityCustom::GrowLeaf(Vec3 pos, Quat rot, CBranchEntity* cbChild)
{
	SEntitySpawnParams spawnParams;
	spawnParams.sName = "Leaf_";
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Branch");
	spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;
	spawnParams.vPosition = pos;
	spawnParams.qRotation = rot * Quat::CreateRotationY((rand() % 360) * 3.14 / 180.0f);
	spawnParams.vScale = Vec3(.5f);
	IEntity* entity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	GetEntity()->AttachChild(entity);
	CBranchEntity* cb = entity->GetComponent<CBranchEntity>();
	cb->JustLeaf();
	return cb;
}
void CPlantEntityCustom::SerializeProperties(Serialization::IArchive& archive)  // give control in editor over some properties
{
	archive(Serialization::ModelFilename(m_geometry), "Geometry", "Geometry");
	if (archive.isInput())
	{
		Reset();
	}
}
uint64 CPlantEntityCustom::GetEventMask() const {
	return BIT64(ENTITY_EVENT_START_GAME) | BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_RESET) | BIT64(ENTITY_EVENT_RENDER_VISIBILITY_CHANGE);
}
void CPlantEntityCustom::ProcessEvent(SEntityEvent &event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_START_GAME:
		Reset();
		CryLogAlways("Start Game");
		break;
	case ENTITY_EVENT_RENDER_VISIBILITY_CHANGE:
		break;
	case ENTITY_EVENT_RESET:
		if (event.nParam[0] == 1)
		{
			CryLogAlways("Reset1");
			growActive = true;
		}
		else
		{
			CryLogAlways("Reset0");
			growActive = false;
		}
		Reset();
		break;
	case ENTITY_EVENT_UPDATE:

		if (currentTime - offsetTime > plantTime) // add another cylinder periodically
		{
			for (int i = 0; i < branches.size(); i++)
			{
				branches.at(i)->updateGrowth(lights);
				if (!branches.at(i)->end && !branches.at(i)->hasChild())
				{
					Vec3 branch_start_pos = branches.at(i)->GetEnd();
					Quat last_section_rot = branches.at(i)->plant_sections.at(branches.at(i)->plant_sections.size() - 2).rot;
					int ps_leaf = rand() % branches.at(i)->plant_sections.size();
					int r = rand() % 100;
					float angle = 55.0f;
					
					Quat rand_rot = Quat::CreateRotationY((rand() % 360) * 3.14 / 180.0f);
					if (r >= 0 && r <= 85)
					{
						Quat branch_start_rot = last_section_rot  * Quat::CreateRotationX(0 * 3.14 / 180.0f) * rand_rot;
						branches.push_back(GrowBranch(branch_start_pos, branch_start_rot, branches.at(i)));
						if (rand() % 4 == 0)
						{
							GrowLeaf(branches.at(i)->plant_sections.at(ps_leaf).pos, Quat(IDENTITY), NULL);
						}
						break;
					}
					else if (r < 90)
					{
						Quat branch_start_rot = last_section_rot  * Quat::CreateRotationX(angle * 3.14 / 180.0f) * rand_rot;
						Quat branch_start_rot_B = last_section_rot  * Quat::CreateRotationX(-angle * 3.14 / 180.0f) * rand_rot;

						branches.push_back(GrowBranch(branch_start_pos, branch_start_rot, branches.at(i)));
						branches.push_back(GrowBranch(branch_start_pos, branch_start_rot_B, branches.at(i)));
						break;
					}
					else if (r < 95)
					{
						Quat branch_start_rot = last_section_rot  * Quat::CreateRotationX(angle * 3.14 / 180.0f) * rand_rot;
						Quat branch_start_rot_B = last_section_rot  * Quat::CreateRotationX(0 * 3.14 / 180.0f) * rand_rot;

						branches.push_back(GrowBranch(branch_start_pos, branch_start_rot, branches.at(i)));
						branches.push_back(GrowBranch(branch_start_pos, branch_start_rot_B, branches.at(i)));
						break;

					}
					else
					{
						Quat branch_start_rot = last_section_rot  * Quat::CreateRotationX(0 * 3.14 / 180.0f) * rand_rot;
						Quat branch_start_rot_B = last_section_rot  * Quat::CreateRotationX(-angle * 3.14 / 180.0f) * rand_rot;

						branches.push_back(GrowBranch(branch_start_pos, branch_start_rot, branches.at(i)));
						branches.push_back(GrowBranch(branch_start_pos, branch_start_rot_B, branches.at(i)));
						GrowLeaf(branches.at(i)->plant_sections.at(ps_leaf).pos, Quat(IDENTITY), NULL);
						break;
					}
				}
			}
			plantTime++;
		}
		//else if (plantTime > currentTime - offsetTime)
		//{
		//	for (int i = 0; i < branches.size(); i++)
		//	{
		//		//branches.at(i).RemoveSection();
		//	}
		//	plantTime--;
		//}
		//break;
	}
}
void CPlantEntityCustom::growthSwitch(bool active)
{
	growActive = active;
}
void CPlantEntityCustom::growthSwitch()
{
	growActive = !growActive;
}
void CPlantEntityCustom::SetTime(int time)
{
	currentTime = time;
}
void CPlantEntityCustom::SetTime(int time, int offsetTimeSet)
{
	currentTime = time;
	offsetTime = offsetTimeSet;
}
void CPlantEntityCustom::setLight(IEntity* l)
{
	if (l)
	{
		lights.push_back(l);
	}
}
void CPlantEntityCustom::setGrowthRate(int millis)
{
	growthRate = (milliseconds)millis;
}
void CPlantEntityCustom::removeLight(IEntity* light)
{
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights.at(i)->GetId() == light->GetId())
		{
			lights.erase(lights.begin() + i);
			break;
		}
	}
}
void CPlantEntityCustom::Reset()
{
	//for (int i = 0; i < branches.size(); i++)
	//{
	//	gEnv->pEntitySystem->RemoveEntity(branches.at(i)->GetBranchEntity()->GetId());
	//}
	//branches.clear();
	//branches.push_back(GrowBranch(GetEntity()->GetPos() + Vec3(0, 0, .13f), GetEntity()->GetRotation(), NULL));
}

CRYREGISTER_CLASS(CPlantEntityCustom)