#pragma once

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
#include <ctime>
#include "GamePlugin.h"
#include "Entities/Helpers/ISimpleExtension.h"
#include <vector>
#include <map>
#include <Cry3DEngine\IIndexedMesh.h>

struct plant_section
{
	Quat rot;
	Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 pos;
	int numVertices = 0;
	Vec3 points[9];
};
class Branch
{
private:
	int vertArraySize = 240000;
	int plantRingVertices = 8;
	int current_vertex_count = 0;
	Vec3 *pos;
	Vec2 *uv;
	Vec3 *tng;
	vtx_idx *ind;
	SMeshSubset subset;
	SMeshFace *faces;
	_smart_ptr<IStatObj> pStaticObject = nullptr;
	Vec3* leadTargetPos = NULL;
	IEntity* parent = NULL;
	IMaterial* material = NULL;

	std::vector<Branch*> next;
	Branch* prev = NULL;

public:
	std::vector<plant_section> plant_sections;
	int slot = 0;
	Branch(IEntity* parent_entity, Vec3 offsetPos, Quat rot, IMaterial* mat, Branch* prev_branch,  int objectSlot)
	{
		parent = parent_entity;
		material = mat;
		prev = prev_branch;
		slot = objectSlot;
		pos = new Vec3[vertArraySize];
		tng = new Vec3[vertArraySize];
		uv = new Vec2[vertArraySize];
		ind = new vtx_idx[vertArraySize];
		faces = new SMeshFace[vertArraySize / 3];
		pStaticObject = gEnv->p3DEngine->CreateStatObj();

		plant_sections.push_back(plant_section());
		plant_sections.back().scale = Vec3(.005f);
		plant_sections.back().rot = rot * Quat::CreateRotationX(90.0f * 3.14 / 180.0f);
		plant_sections.back().pos = offsetPos;
		plant_sections.back().numVertices = plantRingVertices;

	}
	void addChildBranch(Branch* child)
	{
		next.push_back(child);
	}
	void updateGrowth(std::vector<IEntity*> lights) // called periodically to grow plant
	{
		Vec3 leadTarget = updateLeadTarget(lights);
		Quat nextRotation = rotatePosition(&leadTarget);
		float growDistance = .005f;
		Vec3 nextSectionPos = plant_sections.back().pos + nextRotation * Vec3(0.f, growDistance, 0.0f);
		plant_sections.push_back(plant_section());
		plant_sections.back().scale = Vec3(.005f);
		plant_sections.back().rot = nextRotation;
		plant_sections.back().pos = nextSectionPos;
		plant_sections.back().numVertices = plantRingVertices;

		//buildSection(lastSectionPos, plant_sections.back().points, plant_sections.back().scale.x, plant_sections.back().rot, plant_sections.back().numVertices);
		growSections();
		redraw();
	}
	Vec3 updateLeadTarget(std::vector<IEntity*> lights)
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
			pos = new Vec3(parent->GetPos() + plant_sections.back().pos + Vec3(0, 0, 1.f));
		}
		return *pos;
	}
	Quat rotatePosition(Vec3* target)
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
			stp = plant_sections.back().pos + parent->GetPos();
			stp.z += 10.0f;
			stp.y += randomx;
			tempTarget = stp;
		}
		stp.x -= (1 - tempStability) * (tempTarget.x - (parent->GetPos().x + plant_sections.back().pos.x));
		stp.y -= (1 - tempStability) * tempStability * (tempTarget.x - (parent->GetPos().x + plant_sections.back().pos.x));
		stp.z -= (1 - tempStability) * (tempTarget.z - (parent->GetPos().z + plant_sections.back().pos.z));
		/*
		TODO: if rotation angle is greater than 33 degree don't rotate or reposition LT
		*/
		int digis = 5;
		Quat q = QmMR(Quat::CreateSlerp(plant_sections.back().rot, Quat::CreateRotationVDir(stp - (plant_sections.back().pos + parent->GetPos())), tempStability / 10.0f), digis);
		if (cNEQ(q, plant_sections.back().rot, digis) && cNEQ(q, IDENTITY, digis))
		{
			return q;
		}
		return plant_sections.back().rot;
	}
	Quat QmMR(Quat q, int digis)
	{
		q.v.x = mMR(q.v.x, digis);
		q.v.y = mMR(q.v.y, digis);
		q.v.z = mMR(q.v.z, digis);
		q.w = mMR(q.w, digis);
		return q;
	}
	double mMR(double d, int x)
	{
		return floor((d * pow(10, x))) / pow(10, x);
	}
	bool cNEQ(Quat a, Quat b, int digis)
	{
		return ((a.GetFwdX(), digis) != mMR(b.GetFwdX(), digis) || mMR(a.GetFwdY(), digis)
			!= mMR(b.GetFwdY(), digis) || mMR(a.GetFwdZ(), digis) != mMR(b.GetFwdZ(), digis) || mMR(a.w, digis) != mMR(b.w, digis));
	}
	void redraw()
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
		pStaticObject->SetMaterial(material);
		parent->SetStatObj(pStaticObject, slot, false);
	}
	void buildSection(Vec3 Pos, Vec3 *points, float scale, Quat rotation, float vertices)
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
	void createRectangle(Vec3 *lPoint, Vec3 *nPoint, Vec3 *p)
	{
		// Takes 6 points and makes a rectangle
		p[0] = lPoint[0];
		p[1] = lPoint[1];
		p[2] = nPoint[1];

		p[3] = lPoint[0];
		p[4] = nPoint[1];
		p[5] = nPoint[0];
	}
	void PlantTangents(Vec3* tangents, int tng_count)
	{
		// make a tangents the same
		for (size_t i = 0; i < tng_count; i = i + 3)
		{
			tangents[i] = Vec3(0, 1, 0);
			tangents[i + 1] = Vec3(0, 1, 0);
			tangents[i + 2] = Vec3(0, 1, 0);
		}
	}
	void TexturePlant(int ringVertexCount, Vec2* uv_tex, int vertex_count, int plant_section_size)
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
	void PlantFaces(SMeshFace *mesh_faces)
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
	void growSections()
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
	}
	void RemoveSection()
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
	void resizeSections()
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
};

using namespace std::chrono;
class CPlantEntityCustom
	: public CGameObjectExtensionHelper<CPlantEntityCustom, ISimpleExtension>
	, public IEntityPropertyGroup
	, IActionListener
{
public:

	//static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CEntityFlowNode* pNode)
	//{
	//	if (auto* pEntity = gEnv->pEntitySystem->GetEntity(entityId))
	//	{
	//		auto* plantEntity = pEntity->GetComponent<CPlantEntityCustom>();
	//		if (IsPortActive(pActInfo, eInputPort_LightPosition)) // get light position
	//		{
	//			plantEntity->lightPosition = GetPortVec3(pActInfo, eInputPort_LightPosition);
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_LightIntensity)) // get light intensity
	//		{
	//			plantEntity->lightIntensity = GetPortFloat(pActInfo, eInputPort_LightIntensity);
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_Grow)) // check if growth is active
	//		{
	//			plantEntity->growActive = GetPortBool(pActInfo, eInputPort_Grow);
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_Reset)) // reset triggered from flow graph
	//		{
	//			plantEntity->resetGrowth();
	//		}
	//		if (IsPortActive(pActInfo, eInputPort_TriggerGrow)) // growth triggered by flowgraph
	//		{
	//			plantEntity->updateGrowth();
	//		}
	//	}
	//}

	//{0DCE65E63DD2456D-8275052A50DDDEB2}
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CPlantEntityCustom, "GPlantEntityCustom", 0x0DCE65E63DD2456D, 0x8275052A50DDDEB2);
	virtual void OnAction(const ActionId &action, int activationMode, float value) override;

	//IFlowGraph* m_pFlowGraph;
	/*virtual void SetFlowGraph(IFlowGraph* pFlowGraph) override
	{
	if (m_pFlowGraph)
	m_pFlowGraph->Release();
	m_pFlowGraph = pFlowGraph;
	if (m_pFlowGraph)
	m_pFlowGraph->AddRef();
	};
	virtual IFlowGraph* GetFlowGraph() override {
	return m_pFlowGraph;
	};*/
	void AddEventListener(IEntityEventListener* pListener);
	void RemoveEventListener(IEntityEventListener* pListener);

	virtual void Initialize() override;

	virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

	virtual const char* GetLabel() const override { return "GPlantEntityCustom"; }
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	virtual uint64 GetEventMask() const;
	virtual	void ProcessEvent(SEntityEvent &event);

	void startBranch();

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

	void growthSwitch(bool active);
	void growthSwitch();

	void setLight(IEntity* l);
	void setGrowthRate(int millis);
	void removeLight(IEntity* light);

	void Reset();
	void SetTime(int time);
	void SetTime(int time, int offsetTime);

protected:

	std::vector<IEntity*> lights = std::vector<IEntity*>();
	// random growth things
	Vec3 lastSectionPos = Vec3(0, 0, .13f);
	Quat lastSectionRot = Quat(IDENTITY);

	Quat lastSectionRotation = Quat(IDENTITY);
	IActionMapManager *pActionMapManager;
	milliseconds lastGrowth;
	milliseconds lastGrowthSection;
	milliseconds growthRate = (milliseconds)100;
	IMaterial *pMat;
	IMaterial *potMat;
	IMaterial *invisMat;
	float plantScale = .1f;
	float shrink = 1.0f;
	Vec3* lightPos = NULL; // set dynamically via FlowGraph
	float lightIntensity = 1.0f; // set dynamically via FlowGraph
	bool growActive = false;
	float sectionScale = .1f;
	std::vector<plant_section> plant_sections;
	std::vector<Branch> branches;

	Vec3* leadTargetPos = NULL;
	// Handler for actionmap events that maps actions to callbacks
	TActionHandler<CPlantEntityCustom> m_actionHandler;
	string m_geometry = "objects/default/custom_plant_cyl.cgf";
	string m_potGeometry = "objects/pot/pot.cgf";
	string m_potProxy = "objects/plant_cylinder.cgf";
	bool proxySet = false;

	// custom mesh
	_smart_ptr<IStatObj> pStaticObject = nullptr;
	_smart_ptr<IMaterial> material = nullptr;
	Matrix34 matrix;
	Vec3 *pos;
	Vec2 *uv;
	Vec3 *tng;
	vtx_idx *ind;
	SMeshSubset subset;
	SMeshFace *faces;
	int counter = 0;
	int current_vertex_count = 0;
	int vertArraySize = 240000;
	int plantRingVertices = 8;

	int currentTime = 0;
	int offsetTime = 0;
	int plantTime = 0;
public:
	enum EInputPorts
	{
		eInputPort_LightPosition = 0,
		eInputPort_LightIntensity,
		eInputPort_Grow,
		eInputPort_Reset,
		eInputPort_TriggerGrow
	};

	enum EOutputPorts
	{
		// placeholders right now
		eOutputPort_OnHide = 0,
		eOutputPort_OnUnHide,
		eOutputPort_OnCollision,
		eOutputPort_CollisionSurfaceName,
		eOutputPort_OnGeometryChanged
	};
};

class CPlantEntityCustomRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		RegisterEntityWithDefaultComponent<CPlantEntityCustom>("GBeanPlantCustom", "GPlantEntities");

		//// Flow Node Defaults
		//CEntityFlowNodeFactory* pFlowNodeFactory = new CEntityFlowNodeFactory("entity:GBeanPlant");

		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<Vec3>("LightPosition", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<float>("LightIntensity", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<bool>("GrowActive", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<bool>("ResetGrowth", ""));
		//pFlowNodeFactory->m_inputs.push_back(InputPortConfig<bool>("TriggerGrow", ""));
		//pFlowNodeFactory->m_activateCallback = CPlantEntityCustom::OnFlowgraphActivation;

		///*
		//placeholders right now, used for sending information back to flowgraph
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig_Void("OnHide"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig_Void("OnUnHide"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig<EntityId>("OnCollision"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig<string>("CollisionSurfaceName"));
		//pFlowNodeFactory->m_outputs.push_back(OutputPortConfig<string>("OnGeometryChanged"));*/

		//pFlowNodeFactory->Close();
	}
	virtual void Unregister() override
	{

	}
};
static  CPlantEntityCustomRegistrator g_entityRegistratorPlantCustom;