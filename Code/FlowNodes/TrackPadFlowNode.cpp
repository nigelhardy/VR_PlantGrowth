#include "StdAfx.h"
#include <string>
#include "FlowNodes\TrackPadFlowNode.h"
#include "GamePlugin.h"
class CMyFlowNodeRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{


	}
	virtual void Unregister() override
	{

	}

public:
	CMyFlowNodeRegistrator() {}
	~CMyFlowNodeRegistrator()
	{

	}
};
CMyFlowNodeRegistrator g_myFlowNodeRegistrator;
REGISTER_FLOW_NODE("entity:MyFlowNode", CFlowNode_TrackPad);

CFlowNode_TrackPad::CFlowNode_TrackPad(SActivationInfo* pActInfo)
{
	// This means from now on you will get ProcessEvent calls with eFE_Update event.
	// To be removed again from this list call the same function with false as the second parameter.
	// Frequency: you will get 1 ProcessEvent(eFE_Updated) call per Game update call.
	pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
};

IFlowNodePtr CFlowNode_TrackPad::Clone(SActivationInfo *pActInfo)
{
	return new CFlowNode_TrackPad(pActInfo);
};

void  CFlowNode_TrackPad::GetMemoryUsage(ICrySizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
}

void CFlowNode_TrackPad::GetConfiguration(SFlowNodeConfig& config)
{
	// name - Do not use the underscore character '_' in port names

	static const SInputPortConfig in_config[] = {
		InputPortConfig<Vec3>("TrackPadInput", _HELP("TrackPad Vec3 Info"),0),
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

void CFlowNode_TrackPad::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
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
		if (IsPortActive(pActInfo, eInputPort_TrackPadVec0))
		{
			TFlowInputData* inputValue = pActInfo->GetInputPort(eInputPort_TrackPadVec0);
			Vec3* value = inputValue->GetPtr<Vec3>();

			// TODO : 
			if (value)
			{
				
				lastTrackPadPosAlways = *value;
				if (value->x != 0 || value->z != 0)
				{
					Vec3 change = lastTrackPadPos - *value;
					lastTrackPadPos = *value;
					std::string logOutput = std::to_string(change.x) + " " + std::to_string(change.y);
					//CryLogAlways(logOutput.c_str());
					// increment and return
					TFlowInputData output;
					output.Set<float>(change.x);
					output.SetUserFlag(true);
					ActivateOutput(pActInfo, eOutputPorts_ChangeFilterX, output);
					output.Set<float>(change.y);
					output.SetUserFlag(true);
					ActivateOutput(pActInfo, eOutputPorts_ChangeFilterY, output);
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