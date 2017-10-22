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
#include "GamePlugin.h"

using namespace std::chrono;
class CMyLightGeomEntity
	: public IEntityComponent
	, public IEntityPropertyGroup
{
public:

	//{DED9ED7E11D24147 - 926B0322E3B5725C}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CMyLightGeomEntity, "LightGeom", 0xDED9ED7E11D24147, 0x926B0322E3B5725C);

	void AddEventListener(IEntityEventListener* pListener);
	void RemoveEventListener(IEntityEventListener* pListener);

	virtual void Initialize() override;
	virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

	virtual const char* GetLabel() const override { return "LightGeom"; }
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	virtual	void ProcessEvent(SEntityEvent &event);

	virtual uint64 GetEventMask() const;
	void moveTo(Vec3 pos);
	bool lightSwitch();
	void changeLight(bool on);
protected:

	void Reset();

	IMaterial *pMat;
	IMaterial *pMatOff;
	string m_geometry = "objects/sphere.cgf";
	IHmdDevice* pDevice;
	float scale = 40.0f;
	Vec3 hmdPos = Vec3(0, 0, 0);
	IEntitySystem *m_pIEntitySystem;
	IEntity *m_pOffsetEntity;
	Quat offsetRotation = Quat(IDENTITY);
	bool lightOn = true;
	int testCounter = 0;
public:
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


class CMyLightGeomEntityRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CMyLightGeomEntity>("LightGeom", "Plant_Tools");

	}
	virtual void Unregister() override
	{

	}
};
static CMyLightGeomEntityRegistrator g_entityRegistrator;