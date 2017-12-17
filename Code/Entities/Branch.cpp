#include "StdAfx.h"
#include "Branch.h"
#include "GamePlugin.h"

class CBranchRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CBranchEntity>("Branch", "PlantTools");
	}
	virtual void Unregister() override
	{

	}

public:
	CBranchRegistrator() {}
	~CBranchRegistrator()
	{

	}
};
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
static CBranchRegistrator g_branchRegistrator;

CRYREGISTER_CLASS(CBranchEntity)


void CBranchEntity::Initialize()
{
	SEntityPhysicalizeParams params;
	params.type = PE_RIGID;
	GetEntity()->Physicalize(params);
	GetEntity()->SetUpdatePolicy(ENTITY_UPDATE_ALWAYS);
	GetEntity()->Activate(true); // necessary for UPDATE event to be called

	maxSections = rand() % 30 + 40;
	CryLogAlways(std::to_string(maxSections).c_str());
	pos = new Vec3[vertArraySize];
	tng = new Vec3[vertArraySize];
	uv = new Vec2[vertArraySize];
	ind = new vtx_idx[vertArraySize];
	faces = new SMeshFace[vertArraySize / 3];
	pStaticObject = gEnv->p3DEngine->CreateStatObj();
	pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/plant.mtl");
	pMat = gEnv->p3DEngine->GetMaterialManager()->CloneMultiMaterial(pMat);
	GetEntity()->SetSlotMaterial(0, pMat);
}

void CBranchEntity::SetParent(IEntity* par)
{
	parent = par;
}
Vec3 CBranchEntity::GetEnd()
{
	if (plant_sections.size() > 1)
	{
		return plant_sections.at(plant_sections.size()-2).pos;
	}
	return plant_sections.back().pos;
}
void CBranchEntity::SetRoot(Vec3 rootPos, Quat rootRot)
{
	plant_sections.push_back(plant_section());
	plant_sections.back().scale = Vec3(.005f);
	//plant_sections.back().rot = rot;
	//plant_sections.back().pos = offsetPos;
	plant_sections.back().rot = rootRot;
	plant_sections.back().pos = rootPos;
	plant_sections.back().numVertices = plantRingVertices;
}
float CBranchEntity::getLastSectionSize()
{
	return lastSectionSize;
}
float CBranchEntity::getPrevLastSectionSize()
{
	if (prev)
	{
		return prev->getLastSectionSize();
	}
	else
	{
		return 0;
	}
}
IEntity* CBranchEntity::GetBranchEntity()
{
	return GetEntity();
}
bool CBranchEntity::hasChild()
{
	if (next.size() > 0)
	{
		return true;
	}
	return false;
}
void CBranchEntity::addChildBranch(CBranchEntity* child)
{
	if (child)
	{
		next.push_back(child);
		child->prev = this;
	}
}
void CBranchEntity::JustLeaf()
{
	GetEntity()->LoadGeometry(1, m_leafGeometry);
	GetEntity()->SetSlotMaterial(1, pMat);
}
void CBranchEntity::updateGrowth(std::vector<IEntity*> lights) // called periodically to grow plant
{
	if (end) {
		Vec3 leadTarget = updateLeadTarget(lights);
		
		Quat nextRotation = rotatePosition(&leadTarget);
		float growDistance = .005f;
		Vec3 nextSectionPos = plant_sections.back().pos + nextRotation * Vec3(0.f, growDistance, 0.0f);
		if (plant_sections.size() > maxSections)
		{
			end = false;
		}
		else
		{ 
			if (!prev)
			{
				branchStability = 0.65f;
				maxSections = 150;
			}
			// add new section
			plant_sections.push_back(plant_section());
			plant_sections.back().scale = Vec3(.005f);
			plant_sections.back().rot = nextRotation;
			plant_sections.back().pos = nextSectionPos;
			plant_sections.back().numVertices = plantRingVertices;
		}

	}

	growSections();
	redraw();
}
Vec3 CBranchEntity::updateLeadTarget(std::vector<IEntity*> lights)
{
	Vec3* pos = NULL;
	float max_light_intensity = 0.0f;
	for (int i = 0; i < lights.size(); i++)
	{
		// to do, if enabled
		max_light_intensity += 1.0f;
		if ((lights.at(i)->GetPos() - plant_sections.back().pos).GetLength() < 1.0f)
		{
			max_light_intensity += 1.0f;
		}
	}
	float tmp = 0.0f;
	for (int i = 0; i < lights.size(); i++)
	{
		if (!pos) // first light
		{
			pos = new Vec3(lights.at(i)->GetPos());
			if ((lights.at(i)->GetPos() - plant_sections.back().pos).GetLength() < 1.0f)
			{
				tmp = 2.0f; // TO DO: Light intensity
			}
			else
			{
				tmp = 1.0f; // TO DO: Light intensity
			}
			*pos *= (tmp / max_light_intensity);
			continue;
		}
		*pos += lights.at(i)->GetPos() * (tmp / max_light_intensity);
	}
	if (pos == NULL) // no lights, just grow up for now
	{
		pos = new Vec3(plant_sections.back().pos + Vec3(0, 0, 1.f));
	}
	return *pos;
}
Quat CBranchEntity::rotatePosition(Vec3* target)
{
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
		stp = plant_sections.back().pos + parent->GetPos();
		stp.z += 10.0f;
		stp.y += randomx;
		tempTarget = stp;
	}
	stp.x -= (1 - branchStability) * (tempTarget.x - (parent->GetPos().x + plant_sections.back().pos.x));
	stp.y -= (1 - branchStability) * branchStability * (tempTarget.x - (parent->GetPos().x + plant_sections.back().pos.x));
	stp.z -= (1 - branchStability) * (tempTarget.z - (parent->GetPos().z + plant_sections.back().pos.z));
	/*
	TODO: if rotation angle is greater than 33 degree don't rotate or reposition LT
	*/
	int digis = 5;
	Quat q = QmMR(Quat::CreateSlerp(plant_sections.back().rot, Quat::CreateRotationVDir(stp - (plant_sections.back().pos + parent->GetPos())), branchStability / 10.0f), digis);
	if (cNEQ(q, plant_sections.back().rot, digis) && cNEQ(q, IDENTITY, digis))
	{
		return q;
	}
	return plant_sections.back().rot;
}
Quat CBranchEntity::QmMR(Quat q, int digis)
{
	q.v.x = mMR(q.v.x, digis);
	q.v.y = mMR(q.v.y, digis);
	q.v.z = mMR(q.v.z, digis);
	q.w = mMR(q.w, digis);
	return q;
}
double CBranchEntity::mMR(double d, int x)
{
	return floor((d * pow(10, x))) / pow(10, x);
}
bool CBranchEntity::cNEQ(Quat a, Quat b, int digis)
{
	return ((a.GetFwdX(), digis) != mMR(b.GetFwdX(), digis) || mMR(a.GetFwdY(), digis)
		!= mMR(b.GetFwdY(), digis) || mMR(a.GetFwdZ(), digis) != mMR(b.GetFwdZ(), digis) || mMR(a.w, digis) != mMR(b.w, digis));
}
void CBranchEntity::redraw()
{
	if (plant_sections.size() <= 1)
	{
		return; // only one right/section, can't draw yet
	}
	else
	{
		current_vertex_count = (plant_sections.size() - 1) * 6 * 8;
		if (current_vertex_count > vertArraySize)
		{
			CryLogAlways("MAX VERTEX COUNT");
			return;
		}
	}
	IIndexedMesh* idxMesh = pStaticObject->GetIndexedMesh();
	CMesh* mesh = idxMesh->GetMesh();
	mesh->SetIndexCount(current_vertex_count);
	mesh->SetVertexCount(current_vertex_count);
	mesh->SetTexCoordsCount(current_vertex_count);
	for (size_t i = 0; i < plant_sections.size(); i++)
	{
		plant_section *ps = &plant_sections.at(i);
		buildSection(ps->pos, ps->points, ps->scale.x, ps->rot, ps->numVertices);
	}
	for (int i = 1; i < plant_sections.size(); i++)
	{
		for (size_t j = 0; j <= plantRingVertices; j++)
		{
			createRectangle(&plant_sections.at(i - 1).points[j], &plant_sections.at(i).points[j],
				&pos[(i - 1) * (6 * plantRingVertices) + j * 6]);
		}
	}
	mesh->SetSharedStream(CMesh::POSITIONS, &pos[0], current_vertex_count);
	PlantTangents(tng, current_vertex_count);
	mesh->SetSharedStream(CMesh::TANGENTS, &tng[0], current_vertex_count);
	TexturePlant(plantRingVertices, uv, current_vertex_count, plant_sections.size());
	mesh->SetSharedStream(CMesh::TEXCOORDS, &uv[0], current_vertex_count);
	for (size_t i = 0; i < current_vertex_count; i++)
	{
		ind[i] = i;
	}
	mesh->SetSharedStream(CMesh::INDICES, &ind[0], current_vertex_count);
	subset.nNumIndices = current_vertex_count;
	subset.nNumVerts = current_vertex_count;
	subset.nFirstVertId = 0;
	subset.nFirstIndexId = 0;

	mesh->m_subsets.clear();
	mesh->m_subsets.push_back(subset);


	PlantFaces(faces);
	mesh->SetSharedStream(CMesh::FACES, &faces[0], 2);
	mesh->m_bbox = AABB(Vec3(-10, -10, -10), Vec3(10, 10, 10));
	bool ret = mesh->Validate(nullptr);

	// make the static object update
	pStaticObject->SetFlags(STATIC_OBJECT_GENERATED | STATIC_OBJECT_DYNAMIC);
	pStaticObject->Invalidate();
	pStaticObject->SetMaterial(pMat);
	GetEntity()->SetStatObj(pStaticObject, slot, false);
}
void CBranchEntity::buildSection(Vec3 Pos, Vec3 *points, float scale, Quat rotation, float vertices)
{
	// takes a central point, scale, and rotation
	// makes (# of vertices) points in defining circle around point with rotation
	float twoPi = 6.28f;
	float zRotation = twoPi / (float)vertices;
	float totalRotation = 0;
	for (size_t i = 0; i < vertices + 1; i++)
	{
		points[i] = Pos + (Vec3(scale, 0, 0) * Quat::CreateRotationZ(totalRotation)) * rotation;
		totalRotation -= zRotation;
	}
}
void CBranchEntity::createRectangle(Vec3 *lPoint, Vec3 *nPoint, Vec3 *p)
{
	// Takes 6 points and makes a rectangle
	p[0] = lPoint[0];
	p[1] = lPoint[1];
	p[2] = nPoint[1];

	p[3] = lPoint[0];
	p[4] = nPoint[1];
	p[5] = nPoint[0];
}
void CBranchEntity::PlantTangents(Vec3* tangents, int tng_count)
{
	// make a tangents the same
	for (size_t i = 0; i < tng_count; i = i + 3)
	{
		tangents[i] = Vec3(0, 1, 0);
		tangents[i + 1] = Vec3(0, 1, 0);
		tangents[i + 2] = Vec3(0, 1, 0);
	}
}
void CBranchEntity::TexturePlant(int ringVertexCount, Vec2* uv_tex, int vertex_count, int plant_section_size)
{
	int repeatEveryNumSections = 32;
	repeatEveryNumSections++;
	// puts seamless texture on plant
	for (size_t k = 0; k < (vertex_count + 1) / (6 * ringVertexCount); k++) // each ring
	{
		for (size_t i = 0; i < (6 * ringVertexCount); i = i + 6) // each rectangle
		{
			// text coord
			float bottom = (k % repeatEveryNumSections) / ((float)repeatEveryNumSections - 1);
			float top = ((k + 1) % repeatEveryNumSections) / ((float)repeatEveryNumSections - 1);

			float left = i / 6 / (float)(ringVertexCount);
			float right = (i / 6 + 1) / (float)(ringVertexCount);

			// make texture go around plant properly, right now it repeats left and right 
			uv_tex[k * (6 * ringVertexCount) + i] = Vec2(left, bottom);
			uv_tex[k * (6 * ringVertexCount) + i + 1] = Vec2(right, bottom);
			uv_tex[k * (6 * ringVertexCount) + i + 2] = Vec2(right, top);

			uv_tex[k * (6 * ringVertexCount) + i + 3] = Vec2(left, bottom);
			uv_tex[k * (6 * ringVertexCount) + i + 4] = Vec2(right, top);
			uv_tex[k * (6 * ringVertexCount) + i + 5] = Vec2(left, top);
		}
	}
}
void CBranchEntity::PlantFaces(SMeshFace *mesh_faces)
{
	// set all faces
	for (size_t i = 0; i < current_vertex_count / 3; i++)
	{
		mesh_faces[i].v[0] = 0;
		mesh_faces[i].v[1] = 1;
		mesh_faces[i].v[2] = 2;
		mesh_faces[i].nSubset = i + 1;
	}
}
void CBranchEntity::growSections()
{
	if (prev)
	{
		plant_sections.at(0).scale = prev->plant_sections.back().scale;
	}
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
	lastSectionSize = plant_sections.back().scale.x;
}
void CBranchEntity::RemoveSection()
{
	if (plant_sections.size() > 1)
	{
		plant_sections.pop_back();
		parent->UnphysicalizeSlot(slot);
		parent->FreeSlot(slot);
		parent->UpdateSlotPhysics(slot);
	}
	resizeSections();
}
void CBranchEntity::resizeSections()
{

	if (plant_sections.size() > 0)
	{
		plant_sections[plant_sections.size() - 1].scale = Vec3(.005f);
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

void CBranchEntity::ProcessEvent(SEntityEvent & event)
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

void CBranchEntity::SerializeProperties(Serialization::IArchive & archive)
{

	if (archive.isInput())
	{
		//Reset();
		//InitAI();
	}
}
void CBranchEntity::dummyFunction()
{
	CryLogAlways("DummyFunction");
}
void CBranchEntity::Reset()
{
	//GetEntity()->LoadGeometry(0, "Assets/Objects/Default/primitive_sphere_small.cgf");

	//SEntityPhysicalizeParams physParams;
	//physParams.type = PE_LIVING;
	//physParams.nSlot = 0;
	//physParams.mass = 10.0f;

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