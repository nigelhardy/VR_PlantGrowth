#include "StdAfx.h"
#include <string>
#include "FlowNodes\LightManagerFlowNode.h"
#include "FlowNodes\FlowNodeHelper.cpp"
#include "Entities\dummy.h"
#include "GamePlugin.h"
#include "DummyFlowNode.h"
class CDummyFlowNodeRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{


	}
	virtual void Unregister() override
	{

	}
	
public:
	CDummyFlowNodeRegistrator() {}
	~CDummyFlowNodeRegistrator()
	{

	}
};
CDummyFlowNodeRegistrator g_myFlowNodeRegistrator;

REGISTER_FLOW_NODE("entity:DummyFlowNode", CFlowNode_Dummy);
CFlowNode_Dummy::CFlowNode_Dummy(SActivationInfo* pActInfo)
{
	// This means from now on you will get ProcessEvent calls with eFE_Update event.
	// To be removed again from this list call the same function with false as the second parameter.
	// Frequency: you will get 1 ProcessEvent(eFE_Updated) call per Game update call.
	pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
};

IFlowNodePtr CFlowNode_Dummy::Clone(SActivationInfo *pActInfo)
{
	return new CFlowNode_Dummy(pActInfo);
};

void  CFlowNode_Dummy::GetMemoryUsage(ICrySizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
}

void CFlowNode_Dummy::GetConfiguration(SFlowNodeConfig& config)
{
	// name - Do not use the underscore character '_' in port names

	static const SInputPortConfig in_config[] = {
		InputPortConfig<Vec3>("LightEntityId", _HELP("Light Entity Id"),0),
		InputPortConfig<int>("placeholder1", _HELP("useful help text2"),0, _UICONFIG("enum_int:Relaxed=0,Alert=1,Combat=2,Crouch=3")),
		InputPortConfig<string>("placeholder2", _HELP("useful help text2"),0, _UICONFIG("enum_string:antohska,kartosha,dolbaejka")),
		{ 0 }
	};
	static const SOutputPortConfig out_config[] = {
		OutputPortConfig<float>("TrackPadDiffX", _HELP("Useful for using trackpad")),
		OutputPortConfig<float>("TrackPadDiffY", _HELP("Useful for using trackpad")),
		{ 0 }
	};

	config.sDescription = _HELP("A description of this flow node");
	config.pInputPorts = in_config;
	config.pOutputPorts = out_config;
	config.SetCategory(EFLN_APPROVED);

}

void CFlowNode_Dummy::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	switch (event)
	{
		// *** Call per Game update call.
	case  eFE_Update:
	{
		//static int g = 0;
		//
		//// Send event to FG output port 
		//TFlowInputData output;
		//output.Set<int>(g++);
		//output.SetUserFlag(true);
		//ActivateOutput(pActInfo, 0, output);
		break;
	}
	// *** If one or more input ports have been activated.
	case eFE_Activate:
	{
		if (IsPortActive(pActInfo, eInputPort_LightEntityId))
		{
			TFlowInputData* inputValue = pActInfo->GetInputPort(eInputPort_LightEntityId);
			EntityId* value = inputValue->GetPtr<EntityId>();
			auto* pEntity = gEnv->pEntitySystem->FindEntityByName("LightManager");
			if (pEntity != NULL && value != NULL)
			{
				auto x = pEntity->GetComponent<CDummyEntity>();
				if (x != NULL)
				{
					x->dummyFunction();
				}
			}

		}
		else if (IsPortActive(pActInfo, eInputPort_SomeInput1))
		{
			TFlowInputData* inputValue = pActInfo->GetInputPort(eInputPort_SomeInput1);
			int* value = inputValue->GetPtr<int>();

			// TODO : 
			if (value)
			{
				// multiply by 2 and return
				TFlowInputData output;
				//output.Set<Vec3>((*value) * 2);
				//output.SetUserFlag(true);
				//ActivateOutput(pActInfo, eOutputPorts_ChangeFilter, output);
			}
		}
		else if (IsPortActive(pActInfo, eInputPort_SomeInput2))
		{
			TFlowInputData* inputValue = pActInfo->GetInputPort(eInputPort_SomeInput2);
			string* value = inputValue->GetPtr<string>();


			// TODO : 
			if (value)
			{
				// return selected string len
				TFlowInputData output;
				//output.Set<int>(value->length());
				//output.SetUserFlag(true);
				//ActivateOutput(pActInfo, eOutputPorts_ChangeFilter, output);
			}
		}

		break;
	}
	default:
		break;
	};
}