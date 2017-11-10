#pragma once
#include "StdAfx.h"

#include "GamePlugin.h"

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>
#include <CryEntitySystem/IEntitySystem.h>

#include <CryAISystem/IAISystem.h>
#include <CryAISystem/IMovementSystem.h>
#include <CryAISystem/MovementRequest.h>
#include <IActorSystem.h>

//#include "Player/ISimpleActor.h"

class CControllerEntity
	: public IEntityComponent
	, public IEntityPropertyGroup
{
	// {55C03A1070A5448E-991CF4BA2D66F8C6}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CControllerEntity, "VR_Controller", 0x55C03A1070A5448E, 0x991CF4BA2D66F8C6);

	class CPathObstacles
		: public IPathObstacles
	{
		// IPathObstacles
		virtual bool IsPathIntersectingObstacles(const NavigationMeshID meshID, const Vec3& start, const Vec3& end, float radius) const { return false; }
		virtual bool IsPointInsideObstacles(const Vec3& position) const { return false; }
		virtual bool IsLineSegmentIntersectingObstaclesOrCloseToThem(const Lineseg& linesegToTest, float maxDistanceToConsiderClose) const override { return false; }
		// ~IPathObstacles
	};

public:
	// IEntityComponent
	virtual void Initialize() override;
	virtual	void ProcessEvent(SEntityEvent &event) override;
	virtual uint64 GetEventMask() const override { return (BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_RESET) | BIT64(ENTITY_EVENT_START_LEVEL)); }
	// IEntityComponent

	// IEntityPropertyGroup
	virtual const char* GetLabel() const override { return "VR_Controller"; };
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	// IEntityPropertyGroup
	void dummyFunction();


protected:

protected:
	void Reset();

	string geometry = "";
	float mass = 10;


	bool m_bOnGround;
	Vec3 m_groundNormal;

	float scale = 40.0f;
	IMaterial *pMat;
	string m_geometry = "objects/controller/vive_controller_ce.cgf";
};


