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

class CRobotEntity
	: public IEntityComponent
	, public IEntityPropertyGroup
{
	// {1BF7A14A410F42DB-B8F88198503FB03D}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CRobotEntity, "Robot", 0x1BF7A14A410F42DB, 0xB8F88198503FB03D);

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
	virtual const char* GetLabel() const override { return "Robot"; };
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	// IEntityPropertyGroup
	void dummyFunction();
	

protected:

protected:
	void Reset();

	string geometry = "";
	float mass = 10;

	IMaterial* pMat;
	IMaterial* pMatInvis;
	bool m_bOnGround;
	Vec3 m_groundNormal;
};


