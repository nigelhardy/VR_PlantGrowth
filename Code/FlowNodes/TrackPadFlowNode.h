#pragma once
#include <CryFlowGraph\IFlowBaseNode.h>

class CFlowNode_TrackPad final : public CFlowBaseNode<eNCT_Instanced>
{
public:
	enum EInputPorts
	{
		eInputPort_TrackPadVec0 = 0,
		eInputPort_SomeInput1,
		eInputPort_SomeInput2,
		eInputPort_Num
	};

	enum EOutputPorts
	{
		eOutputPorts_ChangeFilterX = 0,
		eOutputPorts_ChangeFilterY,
		eOutputPorts_Num
	};
	Vec3 lastTrackPadPos = Vec3(0, 0, 0);
	Vec3 lastTrackPadPosAlways = Vec3(0, 0, 0);
public:
	CFlowNode_TrackPad(SActivationInfo* pActInfo);

	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo) override;
	virtual void GetMemoryUsage(ICrySizer* s) const override;
	virtual void GetConfiguration(SFlowNodeConfig& config) override;
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo) override;
};