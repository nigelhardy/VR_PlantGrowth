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
#include <CrySystem\VR\IHMDDevice.h>
#include <CrySystem\VR\IHMDManager.h>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include "GamePlugin.h"
#include <vector>
#include "EntityManager.h"
#include "CryParticleSystem\IParticles.h"
#include "LightGeom.h"
#include "robot.h"
#include <limits>
#include "InputVR.h"

using namespace std::chrono;

void CInputVREntity::OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode)
{
	if (auto* pEntity = gEnv->pEntitySystem->GetEntity(entityId))
	{
		if (IsPortActive(pActInfo, eInputPort_itemHeld))
		{
			//auto* controllerEntity = pEntity->GetComponent<CControllerREntity>();
			//controllerEntity->getClosestEntity();
		}
	}
}

IFlowGraph* m_pFlowGraph;
void CInputVREntity::SetFlowGraph(IFlowGraph* pFlowGraph)
{
	if (m_pFlowGraph)
		m_pFlowGraph->Release();
	m_pFlowGraph = pFlowGraph;
	if (m_pFlowGraph)
		m_pFlowGraph->AddRef();
};
IFlowGraph* CInputVREntity::GetFlowGraph() {
	return m_pFlowGraph;
};

void CInputVREntity::AddEventListener(IEntityEventListener* pListener)
{
	// Does not check uniquiness due to performance reasons.
	//m_listeners.push_back(pListener);
}
void CInputVREntity::RemoveEventListener(IEntityEventListener* pListener)
{
	//stl::find_and_erase(m_listeners, pListener);
}
IEntity* CInputVREntity::spawnController(char* name) {
	SEntitySpawnParams spawnParams;
	spawnParams.sName = name;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("VR_Controller");
	spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;
	//spawnParams.vScale = Vec3(size, size, size);
	IEntity* entity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	return entity;
}
void CInputVREntity::Initialize()
{
	//GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
	GetEntity()->Activate(true); // necessary for UPDATE event to be called
	controllerEntity1 = spawnController("controller1");

	if (controllerEntity1) 
	{
		controllerEntity1->GetComponent<CControllerEntity>()->setMode(plantMode);
	}
	controllerEntity2 = spawnController("controller2");
	getMovementWand();
	getMovementPlane();
	getPlayerMarker();
}

void CInputVREntity::SerializeProperties(Serialization::IArchive& archive) // give control in editor over some properties
{
	//archive(Serialization::ModelFilename(m_geometry), "Geometry", "Geometry");
	//archive(leftController, "Left", "Left");
	if (archive.isInput())
	{
		Reset();
	}
}
void CInputVREntity::ProcessEvent(SEntityEvent &event)
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
		posArea = GetEntity()->GetPos();
		getControllerInput();
		processCommands();
		break;
	}
}
void CInputVREntity::processCommands()
{
	if (commands.size() > 0)
	{
		commands.back()->execute();
		commands.pop_back();
	}
}
void CInputVREntity::getControllerInput()
{
	if (pController == NULL || !getEntityManager())
	{
		return;
	}
	updateControllersLocation();
	currentController = eHmdController_OpenVR_1;
	// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
	if (pController->IsConnected(eHmdController_OpenVR_1))
	{
		bool touchMenu1 = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_ApplicationMenu);
		bool touchGrip1 = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Grip);
		bool touchTrig1 = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Trigger);
		bool touchTrack1 = pController->IsButtonPressed(currentController, eKI_Motion_OpenVR_TouchPadBtn);
		Vec2 trackpadVec = pController->GetThumbStickValue(currentController, eKI_Motion_OpenVR_TouchPad_X);

		if (touchMenu1 != touchMenuLast1) {
			if (touchMenu1) {
				// spawn selected entity
				//commands.push_back(new SpawnEntityCommand(entityManager, controllerPos1));
				commands.insert(commands.begin(), new SpawnEntityCommand(entityManager, controllerPos1));
			}
		}
		if (touchGrip1 != touchGripLast1) {
			if (touchGrip1)
			{
				commands.insert(commands.begin(), new DetachEntitiesCommand(entityManager, controllerEntity1));
				commands.insert(commands.begin(), new RemoveAllEntitiesCommand(entityManager));
			}
		}
		if (touchTrig1 != touchTrigLast1) {
			if (touchTrig1)
			{
				commands.insert(commands.begin(), new AttachClosestEntityCommand(entityManager, controllerEntity1, "plantEntity"));
			}
			else if (!touchTrig1)
			{
				commands.insert(commands.begin(), new DetachEntitiesCommand(entityManager, controllerEntity1));
			}
		}
		if (touchTrack1 != touchTrackLast1) {
			if (touchTrack1)
			{
				if (trackpadVec.y >= 0.0f)
				{
					// up on d-pad
					commands.insert(commands.begin(), new ChangeEntityCommand(entityManager, 1, controllerEntity1));
				}
				else
				{
					// down on d-pad
					commands.insert(commands.begin(), new ChangeEntityCommand(entityManager, -1, controllerEntity1));
				}
			}
			
		}

		touchMenuLast1 = touchMenu1;
		touchGripLast1 = touchGrip1;
		touchTrigLast1 = touchTrig1;
		touchTrackLast1 = touchTrack1;
	}
	currentController = eHmdController_OpenVR_2;
	if (pController->IsConnected(currentController))
	{
		bool touchMenu2 = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_ApplicationMenu);
		bool touchGrip2 = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Grip);
		bool touchTrig2 = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Trigger);
		bool touchTrack2 = pController->IsButtonPressed(currentController, eKI_Motion_OpenVR_TouchPadBtn);
		Vec2 trackpadVec = pController->GetThumbStickValue(currentController, eKI_Motion_OpenVR_TouchPad_X);

		if (touchMenu2 != touchMenuLast2) {
			if (touchMenu2) {
				
			}
		}
		if (touchGrip2 != touchGripLast2) {
			if (touchGrip2)
			{

			}
		}
		if (touchTrig2 != touchTrigLast2) {
			if (touchTrig2)
			{

			}
			else if (!touchTrig2)
			{

			}
		}
		// this is a bit ugly, this could be done better
		if (touchTrack2)
		{
			if (getPlayerMarker())
			{
				commands.push_back(new MovePlayerMarkerCommand(controllerEntity2, playerMarker, true));
			}
		}
		if (touchTrack2 != touchTrackLast2) {
			if (touchTrack2)
			{
				if (getMovementPlane() && getMovementWand())
				{
					commands.push_back(new ShowMovementPlaneCommand(movementPlane, movementWand, true));
				}
			}
			else if (!touchTrack2)
			{
				commands.push_back(new MovePlayerCommand(controllerEntity2, playerMarker, &movePlayerDest, hmdPos));
				if (getMovementPlane() && getMovementWand())
				{
					commands.push_back(new ShowMovementPlaneCommand(movementPlane, movementWand, false));
				}
				if (getPlayerMarker())
				{
					commands.push_back(new MovePlayerMarkerCommand(controllerEntity2, playerMarker, false));
				}
			}

		}

		touchMenuLast2 = touchMenu2;
		touchGripLast2 = touchGrip2;
		touchTrigLast2 = touchTrig2;
		touchTrackLast2 = touchTrack2;
	}
}
bool CInputVREntity::getMovementPlane() // make player movement plane vis and invis
{
	if (!movementPlane)
	{
		movementPlane = gEnv->pEntitySystem->FindEntityByName("MovementPlane");
		if (movementPlane)
		{
			// 32.69 puts it just above floor of building
			movementPlane->SetPos(Vec3(movementPlane->GetPos().x, movementPlane->GetPos().y, 32.6951));
			movementPlane->Hide(true);
			return true;
		}
	}
	else
	{
		return true;
	}
	return false;
}
bool CInputVREntity::getMovementWand() // make player movement plane vis and invis
{
	if (movementWand)
	{
		return true;
	}
	else if (!movementWand)
	{
		movementWand = gEnv->pEntitySystem->FindEntityByName("Wand");
		if (movementWand)
		{
			controllerEntity2->AttachChild(movementWand);
			movementWand->SetLocalTM(IDENTITY);
		}
	}
}
bool CInputVREntity::getPlayerMarker() // make player movement plane vis and invis
{
	if (playerMarker)
	{
		return true;
	}
	else if (!playerMarker)
	{
		playerMarker = gEnv->pEntitySystem->FindEntityByName("PlayerMarker");
		if (playerMarker)
		{
			playerMarker->Hide(true);
			return true;
		}
	}
}
void CInputVREntity::updateControllersLocation() {

	if(getOffsetEntity())
	{

		if (movePlayerDest)
		{
			Matrix34 vr_cam = m_pOffsetEntity->GetWorldTM();
			vr_cam.SetTranslation(*movePlayerDest);
			m_pOffsetEntity->SetWorldTM(vr_cam);
			movePlayerDest = NULL;
		}
		if (pController->IsConnected(eHmdController_OpenVR_1)) 
		{
			//CCamera cam = gEnv->pSystem->GetViewCamera();
			HmdTrackingState state = pController->GetLocalTrackingState(eHmdController_OpenVR_1);
			controllerPos1 = m_pOffsetEntity->GetPos() + state.pose.position;
			controllerRot1 = state.pose.orientation;
			if (controllerEntity1)
			{
				controllerEntity1->SetPos(controllerPos1);
				controllerEntity1->SetRotation(controllerRot1);
			}
			
		}
		if (pController->IsConnected(eHmdController_OpenVR_2))
		{
			//CCamera cam = gEnv->pSystem->GetViewCamera();
			HmdTrackingState state = pController->GetLocalTrackingState(eHmdController_OpenVR_2);
			controllerPos2 = m_pOffsetEntity->GetPos() + state.pose.position;
			controllerRot2 = state.pose.orientation;
			if (controllerEntity2)
			{
				controllerEntity2->SetPos(controllerPos2);
				controllerEntity2->SetRotation(controllerRot2);
			}
		}
		
	}
}
bool CInputVREntity::getOffsetEntity()
{
	if (m_pOffsetEntity)
	{
		return true;
	}
	else
	{
		m_pOffsetEntity = m_pIEntitySystem->FindEntityByName("VR_Cam");
		if (m_pOffsetEntity)
		{
			return true;
		}
	}
	return false;
}
bool CInputVREntity::getEntityManager()
{
	if (!entityManager)
	{
		auto* pEntity = gEnv->pEntitySystem->FindEntityByName("EntityManager");
		if (pEntity != NULL)
		{
			entityManager = pEntity->GetComponent<CEntityManagerEntity>();
			if (entityManager)
			{
				return true;
			}
		}
	}
	else
	{
		return true;
	}
	return false;
}
void CInputVREntity::getVRDevices() {
	if (IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager()) // Check, if the HMD Manager exists
	{
		if (pDevice = pHmdManager->GetHmdDevice()) // Check, if a valid HMD device is connected
		{
			if (pDevice->GetClass() == EHmdClass::eHmdClass_OpenVR) // Check, if the connected device is an OpenVR device (HTC Vive)
			{
				// The IHmdController is handling all VR controllers associated with the current VR device.
				pController = pDevice->GetController();
				m_pIEntitySystem = gEnv->pSystem->GetIEntitySystem();
				m_pOffsetEntity = m_pIEntitySystem->FindEntityByName("VR_Cam");

			}
		}
	}
}
void CInputVREntity::Reset()
{
	getVRDevices();
}

CRYREGISTER_CLASS(CInputVREntity)

class CInputVREntityRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CInputVREntity>("InputVR", "VR_Objects");
		// Flow Node Defaults
		CEntityFlowNodeFactory* pFlowNodeFactory = new CEntityFlowNodeFactory("entity:InputVR");

		pFlowNodeFactory->m_inputs.push_back(InputPortConfig<int>("AreaEntityID", ""));
		pFlowNodeFactory->m_activateCallback = CInputVREntity::OnFlowgraphActivation;
		pFlowNodeFactory->Close();
	}
	virtual void Unregister() override
	{

	}
};
static CInputVREntityRegistrator g_entityRegistratorInputVR;