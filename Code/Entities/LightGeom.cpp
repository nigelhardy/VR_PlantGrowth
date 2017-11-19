#include "StdAfx.h"
//#include <CryEntitySystem\IEntityComponent.h>
//#include <CryEntitySystem\IEntitySystem.h>
//#include <CryEntitySystem\IEntityClass.h>
//#include <CrySerialization\Decorators\Resources.h>
//#include "Helpers\ISimpleExtension.h"
//#include <CryEntitySystem\IEntity.h>
//#include <chrono>
//#include "Helpers/EntityFlowNode.h"
//#include <CrySerialization/Enum.h>
//#include <string>
//#define _USE_MATH_DEFINES
//#include <math.h>
#include "LightGeom.h"

using namespace std::chrono;

void CMyLightGeomEntity::AddEventListener(IEntityEventListener* pListener)
{
	// Does not check uniquiness due to performance reasons.
	//m_listeners.push_back(pListener);
}
void CMyLightGeomEntity::RemoveEventListener(IEntityEventListener* pListener)
{
	//stl::find_and_erase(m_listeners, pListener);
}

void CMyLightGeomEntity::Initialize()
{
	Reset();
	//pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/light_visual.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/light_visual.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
	pMatOff = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/material_default.mtl");
	pMatOff = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMatOff);
	GetEntity()->SetMaterial(pMat);
	//GetEntity()->SetScale(Vec3(scale, scale, scale));
	GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
	GetEntity()->Activate(true); // necessary for UPDATE event to be called
	GetEntity()->LoadGeometry(0, m_geometry); // create first cylinder as base
	SEntityPhysicalizeParams params;
	//params.type = PE_RIGID;
	params.type = PE_STATIC;
	params.nSlot = 0;
	params.mass = 1.0f;
	GetEntity()->Physicalize(params);


	SEntitySpawnParams spawnParamsL;
	spawnParamsL.sName = "light";
	spawnParamsL.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Light");
	spawnParamsL.nFlags = ENTITY_FLAG_CLIENT_ONLY;
	IEntity *pSpawnedEntityL = gEnv->pEntitySystem->SpawnEntity(spawnParamsL);
	SChildAttachParams childParams;
	GetEntity()->AttachChild(pSpawnedEntityL, childParams);
	pSpawnedEntityL->SetLocalTM(Matrix34::CreateIdentity());

	/*SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Light");

	spawnParams.vPosition = GetEntity()->GetPos();
	spawnParams.qRotation = GetEntity()->GetRotation();

	spawnParams.vScale = Vec3(1, 1, 1);
		
	gEnv->pEntitySystem->SpawnEntity(spawnParams);*/

	// Spawn the entity, bullet is propelled in CBullet based on the rotation and position here
		
}
//virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

//const char* CMyLightGeomEntity::GetLabel() const override { return "LightGeom"; }

void CMyLightGeomEntity::changeLight(bool on)
{
	IEntity* child = GetEntity()->GetChild(0);
	if (on)
	{
		lightOn = on;
		if (child)
		{
			child->Hide(false);
		}
		GetEntity()->SetMaterial(pMat);
	}
	else
	{
		if (child)
		{
			child->Hide(true);
		}
		GetEntity()->SetMaterial(pMatOff);
	}
}
bool CMyLightGeomEntity::lightSwitch()
{
	lightOn = !lightOn;
	IEntity* child = GetEntity()->GetChild(0);
	if (lightOn)
	{
		if (child)
		{
			child->Hide(false);
		}
		GetEntity()->SetMaterial(pMat);
	}
	else
	{
		if (child)
		{
			child->Hide(true);
		}
		GetEntity()->SetMaterial(pMatOff);
	}
	return lightOn;
}
void CMyLightGeomEntity::SerializeProperties(Serialization::IArchive& archive) // give control in editor over some properties
{
	archive(Serialization::ModelFilename(m_geometry), "Geometry", "Geometry");
	if (archive.isInput())
	{
		Reset();
	}
}
void CMyLightGeomEntity::ProcessEvent(SEntityEvent &event)
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
		break;
	}
}
void CMyLightGeomEntity::moveTo(Vec3 pos)
{
	GetEntity()->SetPos(pos);
}
uint64 CMyLightGeomEntity::GetEventMask() const {
	return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_LEVEL) | BIT64(ENTITY_EVENT_RESET);
}

void CMyLightGeomEntity::Reset()
{


}

CRYREGISTER_CLASS(CMyLightGeomEntity)