#include "StdAfx.h"
#include "ControllerEntity.h"
#include "GamePlugin.h"

class CControllerRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CControllerEntity>("VR_Controller", "VR_Objects");
	}
	virtual void Unregister() override
	{

	}

public:
	CControllerRegistrator() {}
	~CControllerRegistrator()
	{

	}
};

static CControllerRegistrator g_controllerRegistrator;

CRYREGISTER_CLASS(CControllerEntity)


void CControllerEntity::Initialize()
{
	pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/controller/vive_controller_ce.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
	GetEntity()->SetMaterial(pMat);
	Reset();
}

void CControllerEntity::ProcessEvent(SEntityEvent & event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_RESET:
	{
		switch (event.nParam[0])
		{
		case 0: // Game ends
			break;
		case 1: // Game starts
			Reset();
			break;
		default:
			break;
		}
	}
	case ENTITY_EVENT_UPDATE:
	{
		//SEntityUpdateContext* param = (SEntityUpdateContext*)event.nParam[0];
	}
	case ENTITY_EVENT_START_LEVEL:
	{
		if (!gEnv->IsEditor())
		{
			Reset();
		}
	}
	default:
		break;
	}
}

void CControllerEntity::SerializeProperties(Serialization::IArchive & archive)
{

	if (archive.isInput())
	{
		//Reset();
		//InitAI();
	}
}
void CControllerEntity::dummyFunction()
{
	CryLogAlways("DummyFunction");
}
void CControllerEntity::Reset()
{

	GetEntity()->SetScale(Vec3(scale, scale, scale));
	GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
	GetEntity()->Activate(true); // necessary for UPDATE event to be called
	GetEntity()->LoadGeometry(0, m_geometry); // create first cylinder as base
	SEntityPhysicalizeParams params;
	params.type = PE_RIGID;
	GetEntity()->Physicalize(params);

	//pe_player_dimensions playerDimensions;
	//// Prefer usage of a cylinder instead of capsule
	//playerDimensions.bUseCapsule = 0;
	//// Specify the size of our cylinder
	//playerDimensions.sizeCollider = Vec3(0.7f, 0.7f, 1.0f);
	//// Keep pivot at the player's feet (defined in player geometry) 
	//playerDimensions.heightPivot = 0.0f;
	//// Offset collider upwards
	//playerDimensions.heightCollider = 1.0f;
	//playerDimensions.groundContactEps = 0.004f;
	//physParams.pPlayerDimensions = &playerDimensions;


	//pe_player_dynamics playerDynamics;
	//playerDynamics.kAirControl = 0.0f;
	//playerDynamics.kAirResistance = 1.0;
	//playerDynamics.mass = physParams.mass;
	//physParams.pPlayerDynamics = &playerDynamics;

	//GetEntity()->Physicalize(physParams);
}