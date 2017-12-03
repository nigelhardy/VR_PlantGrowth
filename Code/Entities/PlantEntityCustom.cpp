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
	pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/plant.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
	potMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/pot_mtl_multi.mtl");
	potMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(potMat);
	invisMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/invis.mtl");
	invisMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(invisMat);
	GetEntity()->SetMaterial(pMat);
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
	lastSectionRot = Quat(IDENTITY);
	pos = new Vec3[vertArraySize];
	tng = new Vec3[vertArraySize];
	uv = new Vec2[vertArraySize];
	ind = new vtx_idx[vertArraySize];
	faces = new SMeshFace[vertArraySize / 3];
	pStaticObject = gEnv->p3DEngine->CreateStatObj();

	branches.push_back(Branch(GetEntity(), Vec3(0, 0, .13f), lastSectionRot, pMat, 3));
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
		//// old way
		//milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		//if (lastGrowth + growthRate < now && growActive) // add another cylinder periodically
		//{
		//	lastGrowth = now;
		//	updateGrowth();	
		//}
		//else if (lastGrowthSection + growthRate < now && growActive) // add another cylinder periodically
		//{
		//	lastGrowthSection = now;
		//	growSections();
		//}


		if (currentTime - offsetTime > plantTime) // add another cylinder periodically
		{
			for (int i = 0; i < branches.size(); i++)
			{
				branches.at(i).updateGrowth(lights);
			}
			plantTime++;
		}
		else if (plantTime > currentTime - offsetTime)
		{
			for (int i = 0; i < branches.size(); i++)
			{
				branches.at(i).RemoveSection();
			}
			plantTime--;
		}
		break;
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
	for (int i = 0; i < branches.size(); i++)
	{
		int slot = branches.at(i).slot;
		GetEntity()->UnphysicalizeSlot(slot);
		GetEntity()->FreeSlot(slot);
	}
	branches.clear();
	branches.push_back(Branch(GetEntity(), Vec3(0, 0, .13f), lastSectionRot, pMat, 3));
}

CRYREGISTER_CLASS(CPlantEntityCustom)