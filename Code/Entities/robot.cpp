#include "StdAfx.h"
#include "robot.h"
#include "GamePlugin.h"

class CRobotRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CRobotEntity>("Robot", "VR_Objects");
	}
	virtual void Unregister() override
	{

	}

public:
	CRobotRegistrator() {}
	~CRobotRegistrator()
	{

	}
};

static CRobotRegistrator g_testRegistrator;

CRYREGISTER_CLASS(CRobotEntity)


void CRobotEntity::Initialize()
{
	pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("robot/blackandredfloatingrobot.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
	pMatInvis = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/transport_invis.mtl");
	pMatInvis = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMatInvis);
	Reset();
}

void CRobotEntity::ProcessEvent(SEntityEvent & event)
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

void CRobotEntity::SerializeProperties(Serialization::IArchive & archive)
{
	
	if (archive.isInput())
	{ 
		//Reset();
		//InitAI();
	}
}
void CRobotEntity::dummyFunction()
{
	CryLogAlways("DummyFunction");
}
void CRobotEntity::Reset()
{
	GetEntity()->LoadGeometry(0, "Assets/robot/BlackAndRedFloatingRobot.cgf");
	GetEntity()->SetMaterial(pMat);

	SEntityPhysicalizeParams physParams;
	physParams.type = PE_RIGID;
	physParams.nSlot = 0;
	physParams.mass = 10.0f;
	
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

	GetEntity()->Physicalize(physParams);
}