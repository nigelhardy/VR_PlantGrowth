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
{
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CDummyEntity, "Dummy", 0xBA8B086AA7BF486C, 0xAD0FEFAD7E84F591);

public:
	// IEntityComponent
	virtual void Initialize() override;
	virtual	void ProcessEvent(SEntityEvent &event) override;
	virtual uint64 GetEventMask() const override { return (BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_RESET) | BIT64(ENTITY_EVENT_START_LEVEL)); }

	void cry();

	


protected:
	

protected:
	void Reset();


	string geometry = "";
	float mass = 10;

	NavigationAgentTypeID m_navigationAgentTypeId;
	MovementActorCallbacks m_callbacks;
	MovementRequestID m_movementRequestId;
	uint32 m_pathFinderRequestId;
	std::shared_ptr<IPathFollower> m_pPathFollower;
	Movement::PathfinderState m_state;
	INavPath *m_pFoundPath;
	AgentMovementAbility m_movementAbility;
	Vec3 m_requestedTargetBodyDirection;


	bool m_bOnGround;
	Vec3 m_groundNormal;
};