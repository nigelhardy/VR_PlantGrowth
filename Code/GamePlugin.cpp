#include "StdAfx.h"
#include "GamePlugin.h"

// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>

#include <ILevelSystem.h>
//#include "Entities\Helpers\FlowBaseNode.h"
#include <CryFlowGraph\IFlowBaseNode.h>

//USE_CRYPLUGIN_FLOWNODES

IEntityRegistrator *IEntityRegistrator::g_pFirst = nullptr;
IEntityRegistrator *IEntityRegistrator::g_pLast = nullptr;

CAutoRegFlowNodeBase * CAutoRegFlowNodeBase::m_pFirst = nullptr;
CAutoRegFlowNodeBase * CAutoRegFlowNodeBase::m_pLast = nullptr;

CGamePlugin::~CGamePlugin()
{
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
	while (pTemp != nullptr)
	{
		pTemp->Unregister();
		pTemp = pTemp->m_pNext;
	}
}

bool CGamePlugin::RegisterFlowNodes()
{
	//// Start with registering the entity nodes
	IFlowSystem *pFlowSystem = gEnv->pGameFramework->GetIFlowSystem();

	CAutoRegFlowNodeBase *pFactory = CAutoRegFlowNodeBase::m_pFirst;
	while (pFactory)
	{
		pFlowSystem->RegisterType(pFactory->m_sClassName, pFactory);
		pFactory = pFactory->m_pNext;
	}
	return true;
}

bool CGamePlugin::UnregisterFlowNodes()
{
	IFlowSystem* pFlowSystem = gEnv->pGameFramework->GetIFlowSystem();
	if (pFlowSystem)
	{
		CAutoRegFlowNodeBase* pFactory = CAutoRegFlowNodeBase::m_pFirst;
		while (pFactory)
		{
			pFlowSystem->UnregisterType(pFactory->m_sClassName);
			pFactory = pFactory->m_pNext;
		}
	}
	return true;
}

bool CGamePlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	return true;
}

void CGamePlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_GAME_POST_INIT:
	{
		// Register entities
		IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
		while (pTemp != nullptr)
		{
			pTemp->Register();
			pTemp = pTemp->m_pNext;
		}

		gEnv->pConsole->ExecuteString("map RefactorTest", false, false);
		//gEnv->pConsole->ExecuteString("e_TimeOfDay 12.7");
		//gEnv->pConsole->ExecuteString("e_TimeOfDaySpeed 1.1");
		//gEnv->pAISystem->GetNavigationSystem()


	}
	break;
	}
}

CRYREGISTER_SINGLETON_CLASS(CGamePlugin)
