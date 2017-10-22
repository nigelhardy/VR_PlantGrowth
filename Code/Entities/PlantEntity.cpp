#include "StdAfx.h"
#include "PlantEntity.h"
#include <math.h>

void CPlantEntityComponent::OnAction(const ActionId &action, int activationMode, float value)
{
	// This function is called when inputs trigger action map events
	// The handler below maps the event (see 'action') to a callback, further below in this file.
	m_actionHandler.Dispatch(this, GetEntityId(), action, activationMode, value);
}	

void CPlantEntityComponent::AddEventListener(IEntityEventListener* pListener)
{
	// Does not check uniquiness due to performance reasons.
	//m_listeners.push_back(pListener);
}
void CPlantEntityComponent::RemoveEventListener(IEntityEventListener* pListener)
{
	//stl::find_and_erase(m_listeners, pListener);
}

void CPlantEntityComponent::Initialize()
{		
	lastGrowth = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/plant.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
	potMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/pot_mtl_multi.mtl");
	potMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(potMat);
	invisMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/invis.mtl");
	invisMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(invisMat);
	GetEntity()->SetMaterial(potMat);
	GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
	GetEntity()->Activate(true); // necessary for UPDATE event to be called
	GetEntity()->LoadGeometry(0, m_potGeometry);
	GetEntity()->SetSlotMaterial(0, potMat);
	GetEntity()->LoadGeometry(1, m_potProxy);
	GetEntity()->SetSlotMaterial(1, invisMat);
	SEntityPhysicalizeParams params;
	params.type = PE_RIGID;
	GetEntity()->Physicalize(params);
	srand(time(NULL));
	lastSectionRot = Quat(Vec3(90.0f * 3.14f / 180.0f, 0.f, 0.f));
}

void CPlantEntityComponent::SerializeProperties(Serialization::IArchive& archive)  // give control in editor over some properties
{
	archive(Serialization::ModelFilename(m_geometry), "Geometry", "Geometry");
	if (archive.isInput())
	{
		Reset();
	}
}
uint64 CPlantEntityComponent::GetEventMask() const {
	return BIT64(ENTITY_EVENT_START_GAME) | BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_RESET) | BIT64(ENTITY_EVENT_RENDER_VISIBILITY_CHANGE);
}
void CPlantEntityComponent::ProcessEvent(SEntityEvent &event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_START_GAME:
		Reset();
		CryLogAlways("Start Game");
		break;
	case ENTITY_EVENT_RENDER_VISIBILITY_CHANGE:
		break;
	case ENTITY_EVENT_RESET:
		if (event.nParam[0] == 1)
		{
			CryLogAlways("Reset1");
			growActive = true;
		}
		else
		{
			CryLogAlways("Reset0");
			growActive = false;
		}
		Reset();
		break;
	case ENTITY_EVENT_UPDATE:
		// grow plant periodically (using flow graph instead) 
		milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		if (lastGrowth + growthRate < now && growActive) // add another cylinder periodically
		{
			lastGrowth = now;
			updateGrowth();	
		}
		else if (lastGrowthSection + growthRate < now && growActive) // add another cylinder periodically
		{
			lastGrowthSection = now;
			growSections();
		}
		break;
	}
}
void CPlantEntityComponent::updateGrowth() // called periodically to grow plant
{
	updateLeadTarget();
	rotatePosition(leadTargetPos);

	Vec3 plantWorldPosition = GetEntity()->GetWorldPos();
	Matrix34 translateMat = Matrix34::CreateTranslationMat(Vec3(0, 0, 0));
	
	Matrix34 localTM = GetEntity()->GetLocalTM();
	Matrix34 rotMat = Matrix34(lastSectionRot) * Matrix34(Quat::CreateRotationX(90.0f * 3.14 / 180.0f));
	
	//shrink *= .9999f;
	Vec3 scaleVec = Vec3(sectionScale * shrink, sectionScale * shrink, sectionScale * shrink);
	Matrix34 scaleMat = Matrix34::CreateScale(Vec3(sectionScale * shrink, sectionScale * shrink, sectionScale * shrink));

	Matrix34 finalMatrix = localTM * scaleMat * rotMat * translateMat;
	
	lastSectionPos += lastSectionRot * Vec3(0.f,0.005f,0.0f);
	Vec3 offsetPosition = lastSectionPos;
	
	GetEntity()->LoadGeometry(geometryCounter, m_geometry);
	GetEntity()->SetSlotMaterial(geometryCounter, pMat);
	IRenderNode* rendNode = GetEntity()->GetRenderNode(geometryCounter);
	rendNode->SetMatrix(finalMatrix); // scaling, rotation, translate of new cylinder
	rendNode->OffsetPosition(offsetPosition); // relative position of new cylinder
	SEntityPhysicalizeParams params;
	rendNode->SetViewDistUnlimited();
	
	plant_pieces[geometryCounter] = plant_piece();
	plant_pieces[geometryCounter].cyl = rendNode;
	plant_pieces[geometryCounter].scale = scaleVec;
	plant_pieces[geometryCounter].rot = rotMat;
	plant_pieces[geometryCounter].offset = offsetPosition;

	geometryCounter++;
}
void CPlantEntityComponent::growSections()
{
	Matrix34 locTM = GetEntity()->GetLocalTM();
	for (int i = 2; i < geometryCounter - 1; i++)
	{
		if (plant_pieces[i].scale.x < 2.0f - (float) geometryCounter / 200.0f)
		{
			float newScale = plant_pieces[i].scale.x * 1.01f;
			if (i > 2 && plant_pieces[i - 1].scale.x < newScale)
			{
				return;
			}
			plant_pieces[i].scale.x = newScale;
			plant_pieces[i].scale.z = newScale;
			Matrix34 scaleMat = Matrix34::CreateScale(plant_pieces[i].scale);
			plant_pieces[i].cyl->SetMatrix(locTM * scaleMat * plant_pieces[i].rot);
			plant_pieces[i].cyl->OffsetPosition(plant_pieces[i].offset);
		}	
	}
}
void CPlantEntityComponent::updateLeadTarget()
{
	Vec3* pos = NULL;
	float max_light_intensity = 0.0f;
	for (int i = 0; i < lights.size(); i++)
	{
		// to do, if enabled
		max_light_intensity += 1.0f;
	}
	float tmp = 0.0f;
	for (int i = 0; i < lights.size(); i++)
	{
		if (true) // TO DO: light enabled, assume always enabled for now
		{
			if (!pos)
			{
				pos = new Vec3(lights.at(i)->GetPos());
				tmp = 1.0f; // TO DO: Light intensity
				*pos *= (tmp / max_light_intensity);
				continue;
			}
			*pos += lights.at(i)->GetPos() * (tmp / max_light_intensity);
			// BELOW IS FROM UNITY, DOESNT MAKE SENSE TO ME
			//*pos += *pos * (tmp / max_light_intensity) - lights.at(i)->GetPos() * (tmp / max_light_intensity);
		}
	}
	if (pos == NULL) // no lights, just grow up for now
	{
		pos = new Vec3(GetEntity()->GetPos() + lastSectionPos + Vec3(0, 0, 1.f));
	}
	leadTargetPos = pos;
}
double CPlantEntityComponent::mMR(double d, int x)
{
	return floor((d * pow(10, x))) / pow(10, x);
}
bool CPlantEntityComponent::cNEQ(Quat a, Quat b, int digis)
{
	return ((a.GetFwdX(), digis) != mMR(b.GetFwdX(), digis) || mMR(a.GetFwdY(), digis)
		!= mMR(b.GetFwdY(), digis) || mMR(a.GetFwdZ(), digis) != mMR(b.GetFwdZ(), digis) || mMR(a.w, digis) != mMR(b.w, digis));
}
Quat CPlantEntityComponent::rotatePosition(Vec3* target)
{
	//Berechnung von SubTargetPoint
	//DONE:STP.Position = Target.Position
	//DONE:STP.Position = Neigungskorrektur nach Steifheit
	//TODO:STP.Position = Neigungskorrektur nach Links-Rechts-Dreher
	//DONE:Rotiere zu STP.Position
	
	// TEMP DEFAULTS
	float tempStability;
	if (geometryCounter < 30)
	{
		tempStability = .05f;
	}
	else
	{
		tempStability = .5f;
	}

	Vec3 stp;
	Vec3 tempTarget;
	if (target)
	{
		stp = *target;
		tempTarget = *target;
	}
	else // no light
	{
		float randomx = (rand() % 100) / 10.0f - 1.0f;
		stp = lastSectionPos + GetEntity()->GetPos();
		stp.z += 10.0f;
		stp.y += randomx;
		tempTarget = stp;
	}
	
	stp.x -= (1 - tempStability) * (tempTarget.x - (GetEntity()->GetPos().x + lastSectionPos.x));
	stp.y -= (1 - tempStability) * tempStability * (tempTarget.x - (GetEntity()->GetPos().x + lastSectionPos.x));
	stp.z -= (1 - tempStability) * (tempTarget.z - (GetEntity()->GetPos().z + lastSectionPos.z));
	/*
	TODO: if rotation angle is greater than 33 degree don't rotate or reposition LT
	*/
	int digis = 5;
	//Quaternion q = QmMR(mc.safeLerp(transform.rotation, mc.safeLookRotation(V3mMR(stp, digis) - V3mMR(transform.position, digis)), Time.deltaTime * mc.Stability), digis);
	
	Quat q = Quat::CreateSlerp(lastSectionRot, Quat::CreateRotationVDir(stp - (lastSectionPos + GetEntity()->GetPos())), tempStability/ 10.0f);
	//Quaternion q = mc.safeLerp(transform.rotation, mc.safeLookRotation(target.position - transform.position), Time.deltaTime * mc.Stability);
	if (cNEQ(q, lastSectionRot, digis) && cNEQ(q, IDENTITY, digis))
	{
		lastSectionRot = q;
	}
	return lastSectionRot;
}
void CPlantEntityComponent::resetGrowth()
{
	for (size_t i = 2; i < geometryCounter; i++) // erases all cylinders at game start
	{
		GetEntity()->UnphysicalizeSlot(i);	
		GetEntity()->FreeSlot(i);
		GetEntity()->UpdateSlotPhysics(i);
	}
	geometryCounter = 2;
}
void CPlantEntityComponent::growthSwitch(bool active)
{
	growActive = active;
}
void CPlantEntityComponent::growthSwitch()
{
	growActive = !growActive;
}
void CPlantEntityComponent::setLight(IEntity* l)
{
	if (l)
	{
		lights.push_back(l);
	}
}
void CPlantEntityComponent::setGrowthRate(int millis)
{
	growthRate = (milliseconds)millis;
}
void CPlantEntityComponent::removeLight(IEntity* light)
{
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights.at(i)->GetId() == light->GetId())
		{
			lights.erase(lights.begin() + i);
			break;
		}
	}
}
void CPlantEntityComponent::Reset()
{
	shrink = 1.0f;
	lastSectionPos = Vec3(0, 0, .13f);
	resetGrowth();
	lastSectionRot = Quat(Vec3(90.0f * 3.14f / 180.0f, 0.f, 0.f));
	plant_pieces.clear();
	//IRenderNode* rendNode = GetEntity()->GetRenderNode(1);
	//Matrix34 lmat = GetEntity()->GetLocalTM();
	//Matrix34 scaleMat = Matrix34::CreateScale(Vec3(sectionScale, sectionScale, sectionScale));
	//rendNode->SetMatrix(scaleMat * lmat); // scaling, rotation, translate of new cylinder
	/*SEntityPhysicalizeParams params;
	params.type = PE_RIGID;
	GetEntity()->Physicalize(params);*/
}

CRYREGISTER_CLASS(CPlantEntityComponent)