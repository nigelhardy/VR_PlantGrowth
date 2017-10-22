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

class CDummyEntity
	: public IEntityComponent
	, public IEntityPropertyGroup
{
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CDummyEntity, "Dummy", 0xBA8B086AA7BF486C, 0xAD0FEFAD7E84F591);

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
	virtual const char* GetLabel() const override { return "Dymmy"; };
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
};


