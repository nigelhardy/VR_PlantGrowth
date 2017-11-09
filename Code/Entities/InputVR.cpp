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
using namespace std::chrono;

class CInputVREntity
	: public IEntityFlowGraphComponent
	, public IEntityPropertyGroup
{
public:
	static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode)
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
	virtual void SetFlowGraph(IFlowGraph* pFlowGraph) override
	{
		if (m_pFlowGraph)
			m_pFlowGraph->Release();
		m_pFlowGraph = pFlowGraph;
		if (m_pFlowGraph)
			m_pFlowGraph->AddRef();
	};
	virtual IFlowGraph* GetFlowGraph() override {
		return m_pFlowGraph;
	};
	// {{25D1A6E5290048E8-9A7202192918DFAE}}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CInputVREntity, "InputVR", 0x25D1A6E5290048E8, 0x9A7202192918DFAE);

	void AddEventListener(IEntityEventListener* pListener)
	{
		// Does not check uniquiness due to performance reasons.
		//m_listeners.push_back(pListener);
	}
	void RemoveEventListener(IEntityEventListener* pListener)
	{
		//stl::find_and_erase(m_listeners, pListener);
	}

	virtual void Initialize() override
	{
		//GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
		GetEntity()->Activate(true); // necessary for UPDATE event to be called
	}
	
	virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

	virtual const char* GetLabel() const override { return "InputVR"; }
	virtual void SerializeProperties(Serialization::IArchive& archive) override // give control in editor over some properties
	{
		archive(Serialization::ModelFilename(m_geometry), "Geometry", "Geometry");
		archive(leftController, "Left", "Left");
		if (archive.isInput())
		{
			Reset();
		}
	}
	virtual	void ProcessEvent(SEntityEvent &event)
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
			
			break;
		}
	}
	
	void getControllerInput()
	{
		if (pController == NULL)
		{
			return;
		}
		currentController = eHmdController_OpenVR_1;
		currentController = eHmdController_OpenVR_2;
		// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
		if (pController->IsConnected(currentController))
		{
			if (pDevice)
			{
				// Get the current tracking state
				HmdTrackingState stateHMD = pDevice->GetLocalTrackingState();
				hmdPos = stateHMD.pose.position;
			}
			bool touchMenu = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_ApplicationMenu);
			bool touchGrip = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Grip);
			bool touchTrig = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Trigger);
			bool touchTrack = pController->IsButtonPressed(currentController, eKI_Motion_OpenVR_TouchPadBtn);
			Vec2 trackpadVec = pController->GetThumbStickValue(currentController, eKI_Motion_OpenVR_TouchPad_X);

			if (touchMenu != touchMenuLast) {
				
			}
			if (touchGrip != touchGripLast) {

			}
			if (touchTrig != touchTrigLast) {

			}
			if (touchTrack != touchTrackLast) {

			}

			touchMenuLast = touchMenu;
			touchGripLast = touchGrip;
			touchTrigLast = touchTrig;
			touchTrackLast = touchTrack;

			//if (getOffsetEntity())
			//{
			//	if (movePlayerDest)
			//	{
			//		Matrix34 vr_cam = m_pOffsetEntity->GetWorldTM();
			//		vr_cam.SetTranslation(*movePlayerDest);
			//		m_pOffsetEntity->SetWorldTM(vr_cam);
			//		movePlayerDest = NULL;
			//	}
			//	CCamera cam = gEnv->pSystem->GetViewCamera();
			//	HmdTrackingState state = pController->GetLocalTrackingState(currentController);

			//	GetEntity()->SetRotation(state.pose.orientation);
			//	GetEntity()->SetPos(m_pOffsetEntity->GetPos() + state.pose.position);
			//}
		}
	}
	bool getOffsetEntity()
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

	virtual uint64 GetEventMask() const {
		return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_LEVEL) | BIT64(ENTITY_EVENT_RESET);
	}

protected:
	bool getEntityManager()
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
	void getVRDevices() {
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
	void Reset()
	{
		getVRDevices();

	}
	int spawnEntitySelector = 0;
	float lastSpawnEntityChange = 0.f;
	IMaterial *pMat;
	string m_geometry = "objects/controller/vive_controller_ce.cgf";
	const IHmdController* pController;
	IHmdDevice* pDevice;
	float scale = 40.0f;
	Vec3 hmdPos = Vec3(0, 0, 0);
	IEntitySystem *m_pIEntitySystem;
	IEntity *m_pOffsetEntity;
	CEntityManagerEntity *entityManager = NULL;
	bool leftController = true;
	Vec3 posArea;
	Vec3 nextPlayerPos = Vec3(0, 0, 0);
	size_t areaId = 0;
	IArea* a = NULL;
	EHmdController currentController;
	bool touchGripLast = false;
	bool touchTrigLast = false;
	bool touchTrackLast = false;
	bool touchMenuLast = false;

	Vec3* movePlayerDest = NULL;
	float lastResetTime = 0;
public:
	enum EInputPorts
	{
		eInputPort_itemHeld = 0,
		eInputPort_MoveHmd

	};

	enum EOutputPorts
	{
		// placeholders right now
		eOutputPort_MovePlayer = 0,
	};
};

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