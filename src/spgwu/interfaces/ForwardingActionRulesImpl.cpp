#include "ForwardingActionRulesImpl.h"
#include <utils/LogDefines.h>

ForwardingActionRulesImpl::ForwardingActionRulesImpl(pfcp::pfcp_far &myFarStruct)
    : ForwardingActionRules()
{
  LOG_FUNC();
  // Store BAR id.
  if (myFarStruct.bar_id.first)
  {
    mFar.bar_id.bar_id = myFarStruct.bar_id.second.bar_id;
  }

  // Store FAR id.
  mFar.far_id.far_id = myFarStruct.far_id.far_id;

  // Save action rules.
  mFar.apply_action.buff = myFarStruct.apply_action.buff;
  mFar.apply_action.drop = myFarStruct.apply_action.drop;
  mFar.apply_action.dupl = myFarStruct.apply_action.dupl;
  mFar.apply_action.forw = myFarStruct.apply_action.forw;
  mFar.apply_action.nocp = myFarStruct.apply_action.nocp;

  if (myFarStruct.forwarding_parameters.first)
  {
    if (myFarStruct.forwarding_parameters.second.outer_header_creation.first)
    {
      mFar.forwarding_parameters.outer_header_creation.ipv4_address.s_addr = myFarStruct.forwarding_parameters.second.outer_header_creation.second.ipv4_address.s_addr;
      mFar.forwarding_parameters.outer_header_creation.ipv6_address.in6_u = myFarStruct.forwarding_parameters.second.outer_header_creation.second.ipv6_address.in6_u;
    }
  }

  if (myFarStruct.duplicating_parameters.first)
  {
    if (myFarStruct.duplicating_parameters.second.destination_interface.first)
    {
      mFar.duplicating_parameters.destination_interface.interface_value = myFarStruct.duplicating_parameters.second.destination_interface.second.interface_value;
    }
    // TODO navarrothiago - complete implementation.
  }
}

ForwardingActionRulesImpl::ForwardingActionRulesImpl(pfcp_far_t_ &myFarStruct)
{
  LOG_FUNC();
  mFar = myFarStruct;
}

ForwardingActionRulesImpl::~ForwardingActionRulesImpl()
{
  LOG_FUNC();
}

far_id_t_ ForwardingActionRulesImpl::getFARId()
{
  // Do not put LOG_FUNC() here.
  return mFar.far_id;
}

apply_action_t_ ForwardingActionRulesImpl::getApplyRules()
{
  LOG_FUNC();
  return mFar.apply_action;
}

forwarding_parameters_t_ ForwardingActionRulesImpl::getForwardingParameters()
{
  LOG_FUNC();
  return mFar.forwarding_parameters;
}

duplicating_parameters_t_ ForwardingActionRulesImpl::getDuplicatingParameters()
{
  LOG_FUNC();
  return mFar.duplicating_parameters;
}

bar_id_t_ ForwardingActionRulesImpl::getBarId()
{
  LOG_FUNC();
  return mFar.bar_id;
}

pfcp_far_t_ ForwardingActionRulesImpl::getData()
{
  LOG_FUNC();
  return mFar;
}
