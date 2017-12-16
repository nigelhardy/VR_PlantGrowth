#pragma once
#include "StdAfx.h"

#include "GamePlugin.h"

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>
#include <CryEntitySystem/IEntitySystem.h>

#include <CryAISystem/IAISystem.h>
#include <CryAISystem/IMovementSystem.h>
#include <CryAISystem/MovementRequest.h>
#include <IActorSystem.h>
#include <Cry3DEngine\IIndexedMesh.h>
//#include "Player/ISimpleActor.h"

struct plant_section
{
	Quat rot;
	Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 pos;
	int numVertices = 0;
	Vec3 points[9];
};

class CBranchEntity
	: public IEntityComponent
	, public IEntityPropertyGroup
{
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CBranchEntity, "Branch", 0xFC920E825A434C76, 0x9F6203A8B08D603C);

	class CPathObstacles
		: public IPathObstacles
	{
		// IPathObstacles
		virtual bool IsPathIntersectingObstacles(const NavigationMeshID meshID, const Vec3& start, const Vec3& end, float radius) const { return false; }
		virtual bool IsPointInsideObstacles(const Vec3& position) const { return false; }
		virtual bool IsLineSegmentIntersectingObstaclesOrCloseToThem(const Lineseg& linesegToTest, float maxDistanceToConsiderClose) const override { return false; }
		// ~IPathObstacles
	};

public:
	// IEntityComponent
	virtual void Initialize() override;
	void SetParent(IEntity * par);
	Vec3 GetEnd();
	void SetRoot(Vec3 rootPos, Quat rootRot);
	float getLastSectionSize();
	float getPrevLastSectionSize();
	bool hasChild();
	void addChildBranch(CBranchEntity * child);
	void updateGrowth(std::vector<IEntity*> lights);
	Vec3 updateLeadTarget(std::vector<IEntity*> lights);
	Quat rotatePosition(Vec3 * target);
	Quat QmMR(Quat q, int digis);
	double mMR(double d, int x);
	bool cNEQ(Quat a, Quat b, int digis);
	void redraw();
	void buildSection(Vec3 Pos, Vec3 * points, float scale, Quat rotation, float vertices);
	void createRectangle(Vec3 * lPoint, Vec3 * nPoint, Vec3 * p);
	void PlantTangents(Vec3 * tangents, int tng_count);
	void TexturePlant(int ringVertexCount, Vec2 * uv_tex, int vertex_count, int plant_section_size);
	void PlantFaces(SMeshFace * mesh_faces);
	void growSections();
	void RemoveSection();
	void resizeSections();
	virtual	void ProcessEvent(SEntityEvent &event) override;
	virtual uint64 GetEventMask() const override { return (BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_RESET) | BIT64(ENTITY_EVENT_START_LEVEL)); }
	// IEntityComponent

	// IEntityPropertyGroup
	virtual const char* GetLabel() const override { return "Branch"; };
	virtual void SerializeProperties(Serialization::IArchive& archive) override;
	// IEntityPropertyGroup
	void dummyFunction();


protected:
	void Reset();

	string geometry = "";
	float mass = 10;


	bool m_bOnGround;
	Vec3 m_groundNormal;

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
	std::vector<CBranchEntity>* branches;
	std::vector<CBranchEntity*> next;
	IMaterial *pMat;
	float branchStability = .8f;
	int maxSections = 0;
	int slot = 0;
public:
	std::vector<plant_section> plant_sections;
	CBranchEntity* prev = NULL;
	
	bool end = true;
	char grammar = 'A';
	float lastSectionSize = 0.f;
};


