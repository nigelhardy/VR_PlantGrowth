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

class CControllerREntity
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
	// {A3A6A79453444275-BA8222F506311A79}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CControllerREntity, "ControllerR", 0xA3A6A79453444275, 0xBA8222F506311A79);

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
		Reset();
		pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/controller/vive_controller_ce.mtl");
		pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
		GetEntity()->SetMaterial(pMat);
		GetEntity()->SetScale(Vec3(scale, scale, scale));
		GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
		GetEntity()->Activate(true); // necessary for UPDATE event to be called
		GetEntity()->LoadGeometry(0, m_geometry); // create first cylinder as base
		SEntityPhysicalizeParams params;
		params.type = PE_RIGID;
		GetEntity()->Physicalize(params);
		spawnPlaceholders();
	}
	IEntity* spawnEntity(char* name, char* className, float size) {
		SEntitySpawnParams spawnParams;
		spawnParams.sName = name;
		spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);
		spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;
		spawnParams.vScale = Vec3(size, size, size);
		IEntity* entity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
		GetEntity()->AttachChild(entity);
		Matrix34 locMat = entity->GetLocalTM();
		locMat.SetTranslation(Vec3(0, .005f, 0));
		entity->SetLocalTM(locMat);
		entity->Hide(true);
		return entity;
	}
	void spawnPlaceholders()
	{
		plantPlaceholder = spawnEntity("plantPlaceHolder", "GBeanPlantCustom", .01f);
		plantPlaceholder->GetComponent<CPlantEntityCustom>()->growthSwitch(false);
		lightPlaceholder = spawnEntity("lightPlaceHolder", "LightGeom", .005f);
		robotPlaceholder = spawnEntity("robotPlaceholder", "Robot", .2f);
	}
	virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

	virtual const char* GetLabel() const override { return "ControllerR"; }
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
			moveEntity(posArea);
			getControllerInput();
			
			break;
		}
	}
	void moveEntity(Vec3 p)
	{
		if (holdingPlant.size() > 0 || holdingLight.size() > 0 || holdingRobot.size() > 0)
		{
			Vec3 posDif = lastHoldArea - p;
			lastHoldArea = GetEntity()->GetPos();
			for (int i = 0; i < holdingPlant.size(); i++)
			{
				holdingPlant.at(i)->GetComponent<CPlantEntityCustom>()->growthSwitch(false);
				holdingPlant.at(i)->GetComponent<CPlantEntityCustom>()->Reset();
				holdingPlant.at(i)->SetPos(holdingPlant.at(i)->GetPos() - posDif);
				/*Matrix34 world = holdingPlant.at(i)->GetWorldTM();
				world.SetTranslation(holdingPlant.at(i)->GetPos() - posDif);
				holdingPlant.at(i)->SetWorldTM(world);*/
				/*gEnv->pEntitySystem->RemoveEntity(holdingPlant.at(i)->GetId());
				holdingPlant.erase(holdingPlant.begin() + i);*/
				/*CMyLightGeomEntity* tmp = holdingPlant.at(i)->GetComponent<CMyLightGeomEntity>();
				tmp->moveTo(GetEntity()->GetPos());*/
			}
			for (int i = 0; i < holdingLight.size(); i++)
			{
				holdingLight.at(i)->SetPos(holdingLight.at(i)->GetPos() - posDif);
			}
			for (int i = 0; i < holdingRobot.size(); i++)
			{
				holdingRobot.at(i)->SetPos(holdingRobot.at(i)->GetPos() - posDif);
			}
		}
	}
	void releaseObject()
	{
		// remove entity from holding vector
		for (int i = 0; i < holdingPlant.size(); i++)
		{
			if (entityManager)
			{
				holdingPlant.at(i)->GetComponent<CPlantEntityCustom>()->growthSwitch(entityManager->GetGlobalGrowth());
			}

		}
		holdingPlant.clear();
		holdingLight.clear();
		holdingRobot.clear();
	}
	void grabObject()
	{
		// put entity in holding vector
		IEntity* tmpP = getClosestEntity("beanPlant");
		IEntity* tmpL = getClosestEntity("lightGeom");
		IEntity* tmpR = getClosestEntity("robot");
		if (tmpP) // check for plant
		{
			holdingPlant.push_back(tmpP);
		}
		if (tmpL) // check for light
		{
			if (tmpP && (tmpL->GetPos() - GetEntity()->GetPos()).GetLength() > (tmpP->GetPos() - GetEntity()->GetPos()).GetLength()) // see if light is closer
			{
				// plant is closer, don't add light
			}
			else
			{
				holdingPlant.clear();
				holdingLight.push_back(tmpL);
			}
		}
		if (tmpR) // check for robot
		{
			if (tmpP && (tmpR->GetPos() - GetEntity()->GetPos()).GetLength() > (tmpP->GetPos() - GetEntity()->GetPos()).GetLength())
			{
				// plant is closer, don't add robot
			}
			else if (tmpL && (tmpR->GetPos() - GetEntity()->GetPos()).GetLength() > (tmpL->GetPos() - GetEntity()->GetPos()).GetLength())
			{
				// light is closer, don't add robot
			}
			else
			{
				holdingPlant.clear();
				holdingLight.clear();
				holdingRobot.push_back(tmpR);
			}
		}
	}
	void removeAllObjects()
	{
		if (holdingPlant.size() > 0 || holdingLight.size() > 0 || holdingRobot.size() > 0)
		{
			if (entityManager)
			{
				for (int i = 0; i < holdingPlant.size(); i++)
				{
					entityManager->removePlant(holdingPlant.at(i));
					//holdingPlant.erase(holdingPlant.begin() + i);
				}
				for (int i = 0; i < holdingLight.size(); i++)
				{
					entityManager->removeLight(holdingLight.at(i));
					//holdingLight.erase(holdingLight.begin() + i);
				}
				for (int i = 0; i < holdingRobot.size(); i++)
				{
					entityManager->removeRobot(holdingRobot.at(i));
					//holdingRobot.erase(holdingRobot.begin() + i);
				}
				holdingPlant.clear();
				holdingLight.clear();
				holdingRobot.clear();
			}
			else
			{
				CryLogAlways("LightManager not found.");
			}

		}
	}
	void switchHeldLight()
	{
		for (int i = 0; i < holdingLight.size(); i++) // switch light on and off
		{
			CMyLightGeomEntity* lGeom = holdingLight.at(i)->GetComponent<CMyLightGeomEntity>();
			if (lGeom)
			{
				if (lGeom->lightSwitch())
				{
					entityManager->lightSwitch(holdingLight.at(i), true);
				}
				else
				{
					entityManager->lightSwitch(holdingLight.at(i), false);
				}
			}
		}
	}
	void spawnObject()
	{
		lastSpawnEntityChange = std::numeric_limits<float>::min();
		switch (spawnEntitySelector)
		{
		case 0:
			spawnPlant();
			break;
		case 1:
			spawnNewLight();
			break;
		case 2:
			spawnRobot();
			break;
		}
	}
	void getControllerInput()
	{
		if (pController == NULL)
		{
			return;
		}
		if (leftController)
		{
			currentController = eHmdController_OpenVR_1;
		}
		else
		{
			currentController = eHmdController_OpenVR_2;
		}
		// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
		if (pController->IsConnected(currentController))
		{
			if (pDevice)
			{
				// Get the current tracking state
				HmdTrackingState stateHMD = pDevice->GetLocalTrackingState();
				hmdPos = stateHMD.pose.position;
			}
			if (!leftController)
			{
				// MENU BUTTON
				bool buttonMenu = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_ApplicationMenu);
				bool touchGrip = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Grip);
				bool touchTrig = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Trigger);
				bool touchTrack = pController->IsButtonPressed(currentController, eKI_Motion_OpenVR_TouchPadBtn);
				Vec2 trackpadVec = pController->GetThumbStickValue(currentController, eKI_Motion_OpenVR_TouchPad_X);

				if (buttonMenu && buttonMenu != buttonMenuLast && !touchTrig) // press menu button
				{
					spawnObject();
				}
				else if (buttonMenu != buttonMenuLast) // release menu button
				{

				}
				else if (buttonMenu && touchGrip) // menu button and grip
				{
					removeAllObjects();
				}

				
				
				if (touchTrig && touchTrig != touchTrigLast && !buttonMenu) // pull trigger
				{
					grabObject();
				}
				else if (!touchTrig && touchTrig != touchTrigLast) // release trigger
				{
					releaseObject();
				}
				else if (touchTrig && touchGrip) // Trigger and grip
				{
					removeAllObjects();
				}
				else if (touchTrig && buttonMenu && buttonMenu != buttonMenuLast) // trigger and menu
				{
					switchHeldLight();
				}
				
				if (touchTrack && touchTrack != touchTrackLast) // change object to spawn
				{
					if (trackpadVec.y > 0.0f)
					{
						switch (spawnEntitySelector)
						{
						case 0:
							plantPlaceholder->Hide(true);
							break;
						case 1:
							lightPlaceholder->Hide(true);
							break;
						case 2:
							robotPlaceholder->Hide(true);
							break;
						}
						spawnEntitySelector = (spawnEntitySelector + 1) % 3;
						// show or switch entity being shown
						switch (spawnEntitySelector)
						{
						case 0:
							plantPlaceholder->Hide(false);
							break;
						case 1:
							lightPlaceholder->Hide(false);
							break;
						case 2:
							robotPlaceholder->Hide(false);
							break;
						}
					}
					if (trackpadVec.y <= 0.0f)
					{
						switch (spawnEntitySelector)
						{
						case 0:
							plantPlaceholder->Hide(true);
							break;
						case 1:
							lightPlaceholder->Hide(true);
							break;
						case 2:
							robotPlaceholder->Hide(true);
							break;
						}
						spawnEntitySelector = (spawnEntitySelector - 1);
						if (spawnEntitySelector < 0)
						{
							spawnEntitySelector += 3;
						}
						// show or switch entity being shown
						switch (spawnEntitySelector)
						{
						case 0:
							plantPlaceholder->Hide(false);
							break;
						case 1:
							lightPlaceholder->Hide(false);
							break;
						case 2:
							robotPlaceholder->Hide(false);
							break;
						}
					}
					lastSpawnEntityChange = gEnv->pTimer->GetCurrTime();
				}
				else if (touchTrackLast != touchTrack)
				{
					
				}
				if (gEnv->pTimer->GetCurrTime() - 1.0f > lastSpawnEntityChange) // hide object after a second
				{
					lastResetTime = std::numeric_limits<float>::max();
					// hide selector entity
					switch (spawnEntitySelector)
					{
					case 0:
						plantPlaceholder->Hide(true);
						break;
					case 1:
						lightPlaceholder->Hide(true);
						break;
					case 2:
						robotPlaceholder->Hide(true);
						break;
					}

				}
				buttonMenuLast = buttonMenu;
				touchTrigLast = touchTrig;
				touchTrackLast = touchTrack;
				touchGripLast = touchGrip;
			}
			else // right controller
			{
				bool buttonMenu = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_ApplicationMenu);
				bool touchGrip = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Grip);
				bool touchTrig = pController->IsButtonTouched(currentController, eKI_Motion_OpenVR_Trigger);
				bool touchTrack = pController->IsButtonPressed(currentController, eKI_Motion_OpenVR_TouchPadBtn);
				if (touchGrip && touchGrip != touchGripLast)
				{
					
				}
				else if (touchGrip)
				{
					
				}

				if (touchTrack)
				{
					
					foundSpot = movePlayerRayCast(touchTrack);
					if (!wandVis)
					{
						showHidePlane(true);
						showHideWand(true);
						wandVis = true;
					}

				}
				else if (touchTrackLast != touchTrack)
				{
					showHideWand(false);
					showHidePlane(false);
					wandVis = false;
					if (foundSpot)
					{
						movePlayerDest = new Vec3(nextPlayerPos);
					}

					if (playerMarker)
					{
						Matrix34 ind = playerMarker->GetWorldTM();
						ind.SetTranslation(Vec3(0, 0, 0));
						playerMarker->SetWorldTM(ind);
					}
				}
				if (touchTrig && touchTrig != touchTrigLast)
				{
					if (entityManager)
					{
						entityManager->growSwitch();
					}
					else if (getLightManager())
					{
						entityManager->growSwitch();
					}
					lastResetTime = gEnv->pTimer->GetCurrTime();
				}
				else if (touchTrig) // holding trigger
				{
					if (gEnv->pTimer->GetCurrTime() - 1.0f > lastResetTime)
					{
						if (getLightManager())
						{
							if (buttonMenu) // menu button is held too
							{
								entityManager->removeAll();
							}
							else
							{
								entityManager->resetGrowth();
								entityManager->grow(true);
							}

						}
						lastResetTime = std::numeric_limits<float>::max();
					}
				}
				buttonMenuLast = buttonMenu;
				touchGripLast = touchGrip;
				touchTrigLast = touchTrig;
				touchTrackLast = touchTrack;
			}
			if (getOffsetEntity())
			{
				if (movePlayerDest)
				{
					Matrix34 vr_cam = m_pOffsetEntity->GetWorldTM();
					vr_cam.SetTranslation(*movePlayerDest);
					m_pOffsetEntity->SetWorldTM(vr_cam);
					movePlayerDest = NULL;
				}
				CCamera cam = gEnv->pSystem->GetViewCamera();
				HmdTrackingState state = pController->GetLocalTrackingState(currentController);

				GetEntity()->SetRotation(state.pose.orientation);
				GetEntity()->SetPos(m_pOffsetEntity->GetPos() + state.pose.position);
			}
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
	void showHidePlane(bool show) // make player movement plane vis and invis
	{
		if (!movementPlane)
		{
			movementPlane = gEnv->pEntitySystem->FindEntityByName("MovementPlane");
			movementPlane->SetPos(Vec3(movementPlane->GetPos().x, movementPlane->GetPos().y, 32.6951));
			if (movementPlane)
			{
				movementPlane->Hide(!show);
			}
		}
		else
		{
			movementPlane->Hide(!show);
		}
	}
	void showHideWand(bool show) // make particle movement wand vis and invis
	{
		if (wand)
		{
			wand->Hide(!show);
		}
		else if (!wand)
		{
			wand = m_pIEntitySystem->FindEntityByName("Wand");
			if (wand)
			{
				GetEntity()->AttachChild(wand);
				wand->SetLocalTM(IDENTITY);
				wand->Hide(!show);
			}
		}
	}
	void logVector(string s, Vec3 v)
	{
		string r = s;
		r.Append(" X: ");
		r.Append(std::to_string(v.x).c_str());
		r.Append(" Y: ");
		r.Append(std::to_string(v.y).c_str());
		r.Append(" Z: ");
		r.Append(std::to_string(v.z).c_str());
		CryLogAlways(r);
	}
	bool movePlayerRayCast(bool button)
	{
		ray_hit rayHit;

		Vec3 origin = GetEntity()->GetWorldPos();
		Vec3 dir = GetEntity()->GetForwardDir();
		int numHits = gEnv->pPhysicalWorld->RayWorldIntersection(origin, dir * 10.0f, ent_static, rwi_stop_at_pierceable, &rayHit, 1);
		if (numHits > 0)
		{
			if (rayHit.surface_idx == 103)
			{
				if (playerMarker)
				{
					Matrix34 ind = playerMarker->GetWorldTM();
					ind.SetTranslation(rayHit.pt + Vec3(-.3f, -.3f, 0.0f));
					playerMarker->SetWorldTM(ind);
				}
				else
				{
					playerMarker = gEnv->pEntitySystem->FindEntityByName("PlayerMarker");
					if (playerMarker)
					{
						Matrix34 ind = playerMarker->GetWorldTM();
						ind.SetTranslation(rayHit.pt + Vec3(-.3f, -.3f,0.0f));
						playerMarker->SetWorldTM(ind);
					}
				}
				nextPlayerPos = Vec3(rayHit.pt - Vec3(hmdPos.x, hmdPos.y, 0.0f));
				/*int type = rayHit.pCollider->GetiForeignData();
				if (type == PHYS_FOREIGN_ID_STATIC)
				{
					IRenderNode* pRenderNode = (IRenderNode*)rayHit.pCollider->GetForeignData(PHYS_FOREIGN_ID_STATIC);

					if (pRenderNode)
					{
						pRenderNode->SetMaterial(highlightMovementArea);
						CryLogAlways("Hit: %s, ipart: %d, partid: %d, surafce_idx: %d, iNode: %d, \n",
							pRenderNode->GetName(), rayHit.ipart, rayHit.partid, rayHit.surface_idx, rayHit.iNode);
					}
				}*/
				return true;
			}
			else
			{
				if (playerMarker)
				{
					Matrix34 ind = playerMarker->GetWorldTM();
					ind.SetTranslation(Vec3(0, 0, 0));
					playerMarker->SetWorldTM(ind);
				}
				else
				{
					playerMarker = gEnv->pEntitySystem->FindEntityByName("PlayerMarker");
					if (playerMarker)
					{
						Matrix34 ind = playerMarker->GetWorldTM();
						ind.SetTranslation(Vec3(0, 0, 0));
						playerMarker->SetWorldTM(ind);
					}
				}
				
			}
		}
		return false;
		
	}
	SEntitySpawnParams createSpawnParams(char* name, char* className, Vec3 scale) 
	{
		SEntitySpawnParams spawnParams;
		spawnParams.sName = name;
		spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);
		spawnParams.vScale = scale;
		spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;
		return spawnParams;
	}
	IEntity* spawnBasicEntity(SEntitySpawnParams sp) 
	{
		
		IEntity *pSpawnedEntity = gEnv->pEntitySystem->SpawnEntity(sp);
		pSpawnedEntity->SetPos(GetEntity()->GetPos() + GetEntity()->GetForwardDir() * .25f);
		pSpawnedEntity->SetRotation(IDENTITY);
		return pSpawnedEntity;
	}
	void spawnPlant()
	{
		if (getLightManager())
		{
			SEntitySpawnParams sp = createSpawnParams("beanPlant", "GBeanPlantCustom", Vec3(1.0f));
			entityManager->addPlant(spawnBasicEntity(sp));
		}
	}
	void spawnRobot()
	{
		if (getLightManager())
		{
			SEntitySpawnParams sp = createSpawnParams("robot", "Robot", Vec3(10.f));
			entityManager->addRobot(spawnBasicEntity(sp));
		}
	}

	void spawnNewLight()
	{
		if (getLightManager())
		{
			SEntitySpawnParams sp = createSpawnParams("lightGeom", "LightGeom", Vec3(.2f));
			IEntity* pSpawnedEntity = spawnBasicEntity(sp);
			SEntitySpawnParams spL = createSpawnParams("light", "Light", Vec3(1.0f));
			IEntity *pSpawnedEntityL = spawnBasicEntity(spL);
			SChildAttachParams childParams;
			pSpawnedEntity->AttachChild(pSpawnedEntityL, childParams);
			pSpawnedEntityL->SetLocalTM(Matrix34::CreateIdentity());
			entityManager->addLight(pSpawnedEntity);
		}
	}
	IEntity* getClosestEntity(string name)
	{
		float constraintRadius = .2f;
		const Vec3 boxMin = GetEntity()->GetWorldPos() - Vec3(constraintRadius + 0.05f);
		const Vec3 boxMax = GetEntity()->GetWorldPos() + Vec3(constraintRadius + 0.05f);
		lastHoldArea = GetEntity()->GetPos();
		IPhysicalEntity** nearbyEntities = 0;
		IEntity* closest = NULL;
		float* smallestDist = NULL;
		if (size_t entityCount = gEnv->pPhysicalWorld->GetEntitiesInBox(boxMin, boxMax, nearbyEntities, ent_all))
		{
			for (size_t i = 0; i < entityCount; ++i)
			{
				if (IEntity* nearbyEntity = gEnv->pEntitySystem->GetEntityFromPhysics(nearbyEntities[i]))
				{
					string s = nearbyEntity->GetName();
					if (!s.compare(name))
					{
						float dist = GetEntity()->GetWorldPos().GetDistance(nearbyEntity->GetWorldPos());
						if (!smallestDist)
						{
							smallestDist = new float(dist);
							closest = nearbyEntity;
						}
						else if (dist < *smallestDist)
						{
							*smallestDist = dist;
							closest = nearbyEntity;
						}
					}
				}
			}
			return closest;
		}
	}

	virtual uint64 GetEventMask() const {
		return BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_START_LEVEL) | BIT64(ENTITY_EVENT_RESET);
	}

protected:
	bool getLightManager()
	{
		if (!entityManager)
		{
			auto* pEntity = gEnv->pEntitySystem->FindEntityByName("LightManager");
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
	void Reset()
	{
		holdingPlant.clear();
		if(IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager()) // Check, if the HMD Manager exists
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
	IEntity *m_moveIndicatorEntity = NULL;
	CEntityManagerEntity *entityManager = NULL;
	Quat offsetRotation = Quat(IDENTITY);
	bool leftController = true;
	Vec3 posArea;
	Vec3 lastHoldArea;
	Vec3 nextPlayerPos = Vec3(0, 0, 0);
	size_t areaId = 0;
	IArea* a = NULL;
	EHmdController currentController;
	bool hold = false;
	std::vector<IEntity*> holdingPlant = std::vector<IEntity*>();
	std::vector<IEntity*> holdingLight = std::vector<IEntity*>();
	std::vector<IEntity*> holdingRobot = std::vector<IEntity*>();
	IRenderNode* moveBoxRenderNode = NULL;
	bool touchGripLast = false;
	bool touchTrigLast = false;
	bool touchTrackLast = false;
	bool buttonMenuLast = false;
	bool foundSpot = false;
	Vec3* movePlayerDest = NULL;
	int testCounter = 0;
	IEntity* wand = NULL;
	bool wandVis = false;
	IEntity* playerMarker = NULL;
	IEntity* movementPlane = NULL;

	IEntity* plantPlaceholder = NULL;
	IEntity* lightPlaceholder = NULL;
	IEntity* robotPlaceholder = NULL;

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

CRYREGISTER_CLASS(CControllerREntity)

class CControllerEntityRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CControllerREntity>("ControllerR", "VR_Objects");
		// Flow Node Defaults
		CEntityFlowNodeFactory* pFlowNodeFactory = new CEntityFlowNodeFactory("entity:ControllerR");

		pFlowNodeFactory->m_inputs.push_back(InputPortConfig<int>("AreaEntityID", ""));
		pFlowNodeFactory->m_activateCallback = CControllerREntity::OnFlowgraphActivation;
		pFlowNodeFactory->Close();
	}
	virtual void Unregister() override
	{

	}
};
static CControllerEntityRegistrator g_entityRegistratorController;