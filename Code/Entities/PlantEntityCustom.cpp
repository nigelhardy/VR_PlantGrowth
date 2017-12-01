#include "StdAfx.h"
#include "PlantEntityCustom.h"
#include <math.h>

void CPlantEntityCustom::OnAction(const ActionId &action, int activationMode, float value)
{
	// This function is called when inputs trigger action map events
	// The handler below maps the event (see 'action') to a callback, further below in this file.
	m_actionHandler.Dispatch(this, GetEntityId(), action, activationMode, value);
}	

void CPlantEntityCustom::AddEventListener(IEntityEventListener* pListener)
{
	// Does not check uniquiness due to performance reasons.
	//m_listeners.push_back(pListener);
}
void CPlantEntityCustom::RemoveEventListener(IEntityEventListener* pListener)
{
	//stl::find_and_erase(m_listeners, pListener);
}

void CPlantEntityCustom::Initialize()
{		
	lastGrowth = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/plant.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
	potMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/pot_mtl_multi.mtl");
	potMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(potMat);
	invisMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("objects/pot/invis.mtl");
	invisMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(invisMat);
	GetEntity()->SetMaterial(pMat);
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
	pos = new Vec3[vertArraySize];
	tng = new Vec3[vertArraySize];
	uv = new Vec2[vertArraySize];
	ind = new vtx_idx[vertArraySize];
	faces = new SMeshFace[vertArraySize / 3];
	pStaticObject = gEnv->p3DEngine->CreateStatObj();
}

void CPlantEntityCustom::SerializeProperties(Serialization::IArchive& archive)  // give control in editor over some properties
{
	archive(Serialization::ModelFilename(m_geometry), "Geometry", "Geometry");
	if (archive.isInput())
	{
		Reset();
	}
}
uint64 CPlantEntityCustom::GetEventMask() const {
	return BIT64(ENTITY_EVENT_START_GAME) | BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_RESET) | BIT64(ENTITY_EVENT_RENDER_VISIBILITY_CHANGE);
}
void CPlantEntityCustom::ProcessEvent(SEntityEvent &event)
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
		//// old way
		//milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		//if (lastGrowth + growthRate < now && growActive) // add another cylinder periodically
		//{
		//	lastGrowth = now;
		//	updateGrowth();	
		//}
		//else if (lastGrowthSection + growthRate < now && growActive) // add another cylinder periodically
		//{
		//	lastGrowthSection = now;
		//	growSections();
		//}


		if (currentTime - offsetTime > plantTime) // add another cylinder periodically
		{
			updateGrowth();
			growSections();
			plantTime++;
		}
		else if (plantTime > currentTime - offsetTime)
		{
			RemoveSection();
			plantTime--;
		}
		break;
	}
}
void createRectangle(Vec3 *lPoint, Vec3 *nPoint, Vec3 *p)
{
	// Takes 6 points and makes a rectangle
	p[0] = lPoint[0];
	p[2] = lPoint[1];
	p[1] = nPoint[1];

	p[3] = lPoint[0];
	p[5] = nPoint[1];
	p[4] = nPoint[0];
}
void buildSection(Vec3 Pos, Vec3 *points, float scale, Quat rotation, float vertices)
{
	// takes a central point, scale, and rotation
	// makes (# of vertices) points in defining circle around point with rotation
	float twoPi = 6.28f;
	float zRotation = twoPi / (float) vertices;
	float totalRotation = 0;
	for (size_t i = 0; i < vertices + 1; i++)
	{
		points[i] = Pos + (Vec3(scale, 0, 0) * Quat::CreateRotationZ(totalRotation)) * rotation;
		totalRotation -= zRotation;
	}
}
void CPlantEntityCustom::TexturePlant()
{
	// puts seamless texture on plant
	for (size_t k = 0; k < count / (6 * plantRingVertices); k++) // each ring
	{
		for (size_t i = 0; i < (6 * plantRingVertices); i = i + 6) // each rectangle
		{
			// text coord
			/*float bottom = k / ((float)plant_sections.size() - 1) / 2.f;
			float top = (k + 1.0) / ((float)plant_sections.size() - 1) / 2.f;*/

			float bottom = ((float)plant_sections.size() - k) / ((float)100.f - 1) / 20.f + .5f;
			float top = (((float)plant_sections.size() - k) + 1.0) / ((float)100.f - 1) / 20.f + .5f;

			float left = i / (float)(6 * plantRingVertices);
			float right = (i + 6) / (float)(6 * plantRingVertices / 3);

			uv[k * (6 * plantRingVertices) + i] = Vec2(left, bottom);
			uv[k * (6 * plantRingVertices) + i + 1] = Vec2(right, bottom);
			uv[k * (6 * plantRingVertices) + i + 2] = Vec2(right, top);

			uv[k * (6 * plantRingVertices) + i + 3] = Vec2(left, bottom);
			uv[k * (6 * plantRingVertices) + i + 4] = Vec2(right, top);
			uv[k * (6 * plantRingVertices) + i + 5] = Vec2(left, top);
		}
	}
}
void CPlantEntityCustom::PlantTangents()
{
	// make a tangents the same
	for (size_t i = 0; i < count; i = i + 3)
	{
		tng[i] = Vec3(0, 1, 0);
		tng[i + 1] = Vec3(0, 1, 0);
		tng[i + 2] = Vec3(0, 1, 0);
	}
}
void CPlantEntityCustom::PlantFaces()
{
	// set all faces
	for (size_t i = 0; i < count / 3; i++)
	{
		faces[i].v[0] = 0;
		faces[i].v[1] = 1;
		faces[i].v[2] = 2;
		faces[i].nSubset = i + 1;
	}
}void CPlantEntityCustom::RemoveSection()
{
	if (plant_sections.size() <= 2)
	{
		return; // only one right/section, can't draw yet
	}
	else
	{
		// add vertices for next section
		count -= (6 * plantRingVertices);
		plant_sections.pop_back();
		GetEntity()->UnphysicalizeSlot(2);
		GetEntity()->FreeSlot(2);
		GetEntity()->UpdateSlotPhysics(2);
	}
	resizeSections();
}
void CPlantEntityCustom::AddSection(Vec3 Pos)
{
	if (plant_sections.size() <= 1)
	{
		return; // only one right/section, can't draw yet
	}
	else
	{
		// add vertices for next section
		count += (6 * plantRingVertices); 
		if (count > vertArraySize)
		{
			CryLogAlways("MAX VERTEX COUNT");
			return;
		}
	}
	redraw();
	//IIndexedMesh* idxMesh = pStaticObject->GetIndexedMesh();
	//CMesh* mesh = idxMesh->GetMesh();

	//mesh->SetIndexCount(count);
	//mesh->SetVertexCount(count);
	//mesh->SetTexCoordsCount(count);
	//
	//int gotElements = 0;
	////pos = mesh->GetStreamPtr<Vec3>(CMesh::POSITIONS, &gotElements);

	//for (size_t i = 0; i <= plantRingVertices; i++)
	//{
	//	createRectangle(&plant_sections.at(plant_sections.size() - 2).points[i], &plant_sections.back().points[i], 
	//		&pos[(plant_sections.size() - 2) * (6 * plantRingVertices) + i * 6]);
	//}
	//mesh->SetSharedStream(CMesh::POSITIONS, &pos[0], count);

	//PlantTangents();
	//mesh->SetSharedStream(CMesh::TANGENTS, &tng[0], count);
	////uv = mesh->GetStreamPtr<Vec2>(CMesh::TEXCOORDS, &gotElements);
	//TexturePlant();
	//mesh->SetSharedStream(CMesh::TEXCOORDS, &uv[0], count);

	////ind = mesh->GetStreamPtr<vtx_idx>(CMesh::INDICES, &gotElements);
	//for (size_t i = (plant_sections.size() - 2) * 6 * plantRingVertices; i < count; i++)
	//{
	//	ind[i] = i;
	//}
	//mesh->SetSharedStream(CMesh::INDICES, &ind[0], count);

	//subset.nNumIndices = count;
	//subset.nNumVerts = count;
	////subset.nMatID = 0;
	////subset.FixRanges(&ind[0]);
	//subset.nFirstVertId = 0;
	//subset.nFirstIndexId = 0;
	//mesh->m_subsets.clear();
	//mesh->m_subsets.push_back(subset);

	//PlantFaces();
	//mesh->SetSharedStream(CMesh::FACES, &faces[0], 2);

	//mesh->m_bbox = AABB(Vec3(-10, -10, -10), Vec3(10, 10, 10));
	//bool ret = mesh->Validate(nullptr);

	//// make the static object update
	//pStaticObject->SetFlags(STATIC_OBJECT_GENERATED | STATIC_OBJECT_DYNAMIC);
	//pStaticObject->Invalidate();
	//pStaticObject->SetMaterial(material);
	//GetEntity()->SetStatObj(pStaticObject, 2, false);
}
void CPlantEntityCustom::redraw()
{
	IIndexedMesh* idxMesh = pStaticObject->GetIndexedMesh();
	CMesh* mesh = idxMesh->GetMesh();
	mesh->SetIndexCount(count);
	mesh->SetVertexCount(count);
	mesh->SetTexCoordsCount(count);
	int gotElements = 0;
	for (size_t i = 0; i < plant_sections.size(); i++)
	{
		plant_section *ps = &plant_sections.at(i);
		buildSection(ps->pos, ps->points, ps->scale.x, ps->rot, ps->numVertices);
	}
	//pos = mesh->GetStreamPtr<Vec3>(CMesh::POSITIONS, &gotElements);
	for (int i = 1; i < plant_sections.size(); i++)
	{
		for (size_t j = 0; j <= plantRingVertices; j++)
		{
			createRectangle(&plant_sections.at(i - 1).points[j], &plant_sections.at(i).points[j],
				&pos[(i - 1) * (6 * plantRingVertices) + j * 6]);
		}
	}
	mesh->SetSharedStream(CMesh::POSITIONS, &pos[0], count);
	PlantTangents();
	mesh->SetSharedStream(CMesh::TANGENTS, &tng[0], count);
	//uv = mesh->GetStreamPtr<Vec2>(CMesh::TEXCOORDS, &gotElements);
	TexturePlant();
	mesh->SetSharedStream(CMesh::TEXCOORDS, &uv[0], count);
	//ind = mesh->GetStreamPtr<vtx_idx>(CMesh::INDICES, &gotElements);
	for (size_t i = 0; i < count; i++)
	{
		ind[i] = i;
	}
	mesh->SetSharedStream(CMesh::INDICES, &ind[0], count);
	subset.nNumIndices = count;
	subset.nNumVerts = count;
	//subset.nMatID = 0;
	//subset.FixRanges(&ind[0]);
	subset.nFirstVertId = 0;
	subset.nFirstIndexId = 0;
	mesh->m_subsets.clear();
	mesh->m_subsets.push_back(subset);


	PlantFaces();
	mesh->SetSharedStream(CMesh::FACES, &faces[0], 2);
	mesh->m_bbox = AABB(Vec3(-10, -10, -10), Vec3(10, 10, 10));
	bool ret = mesh->Validate(nullptr);

	// make the static object update
	pStaticObject->SetFlags(STATIC_OBJECT_GENERATED | STATIC_OBJECT_DYNAMIC);
	pStaticObject->Invalidate();
	pStaticObject->SetMaterial(material);
	GetEntity()->SetStatObj(pStaticObject, 2, false);
}
void CPlantEntityCustom::updateGrowth() // called periodically to grow plant
{
	updateLeadTarget();
	rotatePosition(leadTargetPos);
	lastSectionPos += lastSectionRot * Vec3(0.f,0.005f,0.0f);
	if (plant_sections.size() > 0)
	{
		lastSectionPos = plant_sections.back().pos + lastSectionRot * Vec3(0.f, 0.005f, 0.0f);
	}

	plant_sections.push_back(plant_section());
	plant_sections.back().scale = Vec3(.005f);
	plant_sections.back().rot = lastSectionRot * Quat::CreateRotationX(-90.0f * 3.14 / 180.0f);
	plant_sections.back().pos = lastSectionPos;
	plant_sections.back().numVertices = plantRingVertices;

	buildSection(lastSectionPos, plant_sections.back().points, plant_sections.back().scale.x, plant_sections.back().rot, plant_sections.back().numVertices);
	AddSection(lastSectionPos);
}
void CPlantEntityCustom::growSections()
{
	for (int i = 0; i < plant_sections.size(); i++)
	{
		if (plant_sections[i].scale.x < 0.1f - (float)plant_sections.size() / 40000.0f)
		{
			float newScale = plant_sections[i].scale.x * 1.01f;
			if (i > 0 && plant_sections[i - 1].scale.x - .000003f < newScale)
			{
				return;
			}
			plant_sections[i].scale = Vec3(newScale);
		}	
	}
	redraw();
}
void CPlantEntityCustom::resizeSections()
{

	if (plant_sections.size() > 0)
	{
		plant_sections[plant_sections.size()-1].scale = Vec3(.005f);
	}
	for (int i = plant_sections.size() - 1; i > 0; i--)
	{
		if (plant_sections[i - 1].scale.x < .1f - (float)plant_sections.size() / 40000.0f)
		{
			plant_sections[i - 1].scale = Vec3(plant_sections[i].scale.x * 1.01f);
		}
		
	}
	redraw();
}
void CPlantEntityCustom::updateLeadTarget()
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
		if (!pos) // first light
		{
			pos = new Vec3(lights.at(i)->GetPos());
			tmp = 1.0f; // TO DO: Light intensity
			*pos *= (tmp / max_light_intensity);
			continue;
		}
		*pos += lights.at(i)->GetPos() * (tmp / max_light_intensity);
	}
	if (pos == NULL) // no lights, just grow up for now
	{
		pos = new Vec3(GetEntity()->GetPos() + lastSectionPos + Vec3(0, 0, 1.f));
	}
	leadTargetPos = pos;
}
Quat CPlantEntityCustom::QmMR(Quat q, int digis)
{
	q.v.x = mMR(q.v.x, digis);
	q.v.y = mMR(q.v.y, digis);
	q.v.z = mMR(q.v.z, digis);
	q.w = mMR(q.w, digis);
	return q;
}
double CPlantEntityCustom::mMR(double d, int x)
{
	return floor((d * pow(10, x))) / pow(10, x);
}
bool CPlantEntityCustom::cNEQ(Quat a, Quat b, int digis)
{
	return ((a.GetFwdX(), digis) != mMR(b.GetFwdX(), digis) || mMR(a.GetFwdY(), digis)
		!= mMR(b.GetFwdY(), digis) || mMR(a.GetFwdZ(), digis) != mMR(b.GetFwdZ(), digis) || mMR(a.w, digis) != mMR(b.w, digis));
}
Quat CPlantEntityCustom::rotatePosition(Vec3* target)
{
	float tempStability = .5f;

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
	Quat q = QmMR(Quat::CreateSlerp(lastSectionRot, Quat::CreateRotationVDir(stp - (lastSectionPos + GetEntity()->GetPos())), tempStability/ 10.0f), digis);
	if (cNEQ(q, lastSectionRot, digis) && cNEQ(q, IDENTITY, digis))
	{
		lastSectionRot = q;
	}
	return lastSectionRot;
}
void CPlantEntityCustom::resetGrowth()
{
	GetEntity()->UnphysicalizeSlot(2);	
	GetEntity()->FreeSlot(2);
	GetEntity()->UpdateSlotPhysics(2);
	count = 0;
}
void CPlantEntityCustom::growthSwitch(bool active)
{
	growActive = active;
}
void CPlantEntityCustom::growthSwitch()
{
	growActive = !growActive;
}
void CPlantEntityCustom::SetTime(int time)
{
	currentTime = time;
}
void CPlantEntityCustom::SetTime(int time, int offsetTimeSet)
{
	currentTime = time;
	offsetTime = offsetTimeSet;
}
void CPlantEntityCustom::setLight(IEntity* l)
{
	if (l)
	{
		lights.push_back(l);
	}
}
void CPlantEntityCustom::setGrowthRate(int millis)
{
	growthRate = (milliseconds)millis;
}
void CPlantEntityCustom::removeLight(IEntity* light)
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
void CPlantEntityCustom::Reset()
{
	shrink = 1.0f;
	lastSectionPos = Vec3(0, 0, .13f);
	resetGrowth();
	lastSectionRot = Quat(Vec3(90.0f * 3.14f / 180.0f, 0.f, 0.f));
	plant_sections.clear();
	//IRenderNode* rendNode = GetEntity()->GetRenderNode(1);
	//Matrix34 lmat = GetEntity()->GetLocalTM();
	//Matrix34 scaleMat = Matrix34::CreateScale(Vec3(sectionScale, sectionScale, sectionScale));
	//rendNode->SetMatrix(scaleMat * lmat); // scaling, rotation, translate of new cylinder
	/*SEntityPhysicalizeParams params;
	params.type = PE_RIGID;
	GetEntity()->Physicalize(params);*/
}

CRYREGISTER_CLASS(CPlantEntityCustom)