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
//class Branch
//{
//private:
//	int vertArraySize = 240000;
//	Vec3 *pos;
//	Vec2 *uv;
//	Vec3 *tng;
//	vtx_idx *ind;
//	SMeshSubset subset;
//	SMeshFace *faces;
//	_smart_ptr<IStatObj> pStaticObject = nullptr;
//	Branch()
//	{
//		pos = new Vec3[vertArraySize];
//		tng = new Vec3[vertArraySize];
//		uv = new Vec2[vertArraySize];
//		ind = new vtx_idx[vertArraySize];
//		faces = new SMeshFace[vertArraySize / 3];
//		pStaticObject = gEnv->p3DEngine->CreateStatObj();
//	}
//	void buildSection(Vec3 Pos, Vec3 *points, float scale, Quat rotation, float vertices)
//	{
//		// takes a central point, scale, and rotation
//		// makes (# of vertices) points in defining circle around point with rotation
//		float twoPi = 6.28f;
//		float zRotation = twoPi / (float)vertices;
//		float totalRotation = 0;
//		for (size_t i = 0; i < vertices + 1; i++)
//		{
//			points[i] = Pos + (Vec3(scale, 0, 0) * Quat::CreateRotationZ(totalRotation)) * rotation;
//			totalRotation -= zRotation;
//		}
//	}
//	void TexturePlant()
//	{
//		// puts seamless texture on plant
//		for (size_t k = 0; k < count / (6 * plantRingVertices); k++) // each ring
//		{
//			for (size_t i = 0; i < (6 * plantRingVertices); i = i + 6) // each rectangle
//			{
//				// text coord
//				float bottom = k / ((float)plant_sections.size() - 1) * 100.f;
//				float top = (k + 1.0) / ((float)plant_sections.size() - 1) * 100.f;
//
//
//				//float left = i / (float)(6 * plantRingVertices);
//				//float right = (i + 6) / (float)(6 * plantRingVertices / 3);
//				float left = i / 6 / (float)(plantRingVertices);
//				float right = (i / 6 + 1) / (float)(plantRingVertices);
//
//				// make texture go around plant properly, right now it repeats left and right 
//				uv[k * (6 * plantRingVertices) + i] = Vec2(left, bottom);
//				uv[k * (6 * plantRingVertices) + i + 1] = Vec2(right, bottom);
//				uv[k * (6 * plantRingVertices) + i + 2] = Vec2(right, top);
//
//				uv[k * (6 * plantRingVertices) + i + 3] = Vec2(left, bottom);
//				uv[k * (6 * plantRingVertices) + i + 4] = Vec2(right, top);
//				uv[k * (6 * plantRingVertices) + i + 5] = Vec2(left, top);
//			}
//		}
//	}
//	void PlantTangents()
//	{
//		// make a tangents the same
//		for (size_t i = 0; i < count; i = i + 3)
//		{
//			tng[i] = Vec3(0, 1, 0);
//			tng[i + 1] = Vec3(0, 1, 0);
//			tng[i + 2] = Vec3(0, 1, 0);
//		}
//	}
//	void PlantFaces()
//	{
//		// set all faces
//		for (size_t i = 0; i < count / 3; i++)
//		{
//			faces[i].v[0] = 0;
//			faces[i].v[1] = 1;
//			faces[i].v[2] = 2;
//			faces[i].nSubset = i + 1;
//		}
//	}void RemoveSection()
//	{
//		if (plant_sections.size() <= 2)
//		{
//			return; // only one right/section, can't draw yet
//		}
//		else
//		{
//			// add vertices for next section
//			count -= (6 * plantRingVertices);
//			plant_sections.pop_back();
//			GetEntity()->UnphysicalizeSlot(2);
//			GetEntity()->FreeSlot(2);
//			GetEntity()->UpdateSlotPhysics(2);
//		}
//		resizeSections();
//	}
//	void AddSection(Vec3 Pos)
//	{
//		if (plant_sections.size() <= 1)
//		{
//			return; // only one right/section, can't draw yet
//		}
//		else
//		{
//			if (plant_sections.size() > 100 && plant_sections.size() % 10 == 0)
//			{
//				// reduce sections
//			}
//			// add vertices for next section
//			count += (6 * plantRingVertices);
//			if (count > vertArraySize)
//			{
//				CryLogAlways("MAX VERTEX COUNT");
//				return;
//			}
//		}
//		redraw();
//	}
//};

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
	double CPlantEntityCustom::mMR(double d, int x);
	bool CPlantEntityCustom::cNEQ(Quat a, Quat b, int digis);
	void CPlantEntityCustom::AddSection(Vec3 Pos);
	void CPlantEntityCustom::RemoveSection();

	void PlantFaces(SMeshFace * mesh_faces);

	virtual void Initialize() override;

	virtual IEntityPropertyGroup* GetPropertyGroup() override { return this; }

	virtual const char* GetLabel() const override { return "GPlantEntityCustom"; }
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	virtual uint64 GetEventMask() const;
	virtual	void ProcessEvent(SEntityEvent &event);
	void createRectangle(Vec3 * lPoint, Vec3 * nPoint, Vec3 * p);
	void buildSection(Vec3 Pos, Vec3 * points, float scale, Quat rotation, float vertices);
	void TexturePlant(int ringVertexCount, Vec2 * uv_tex, int vertex_count, int plant_section_size);
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
	void updateGrowth();
	void resetGrowth();
	void growthSwitch(bool active);
	void growthSwitch();
	void redraw();
	void PlantTangents(Vec3 * tangents, int tng_count);
	void setLight(IEntity* l);
	void setGrowthRate(int millis);
	void removeLight(IEntity* light);
	void updateLeadTarget();
	Quat QmMR(Quat q, int digis);
	Quat rotatePosition(Vec3* target);
	void growSections();
	void resizeSections();
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
	int count = 0;
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