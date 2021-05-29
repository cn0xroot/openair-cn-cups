#include "PacketDetectionRulesImpl.h"
#include <utils/LogDefines.h>

PacketDetectionRulesImpl::PacketDetectionRulesImpl(pfcp::pfcp_pdr &myPdr)
{
  LOG_FUNC();
  mPdr.pdr_id.rule_id = myPdr.pdr_id.rule_id;
  if(myPdr.outer_header_removal.first){
    mPdr.outer_header_removal.outer_header_removal_description = myPdr.outer_header_removal.second.outer_header_removal_description;
  }

  if(myPdr.pdi.first){
    if(myPdr.pdi.second.source_interface.first){
     mPdr.pdi.source_interface.interface_value = myPdr.pdi.second.source_interface.second.interface_value; 
    }

    if(myPdr.pdi.second.local_fteid.first){
      mPdr.pdi.fteid.teid = myPdr.pdi.second.local_fteid.second.teid;
    }
  }

  if(myPdr.far_id.first){
    mPdr.far_id.far_id = myPdr.far_id.second.far_id;
  }
}

PacketDetectionRulesImpl::PacketDetectionRulesImpl(pfcp_pdr_t_ &myPdr) 
{
  LOG_FUNC();
  mPdr = myPdr;
}

PacketDetectionRulesImpl::~PacketDetectionRulesImpl()
{
  LOG_FUNC();
}

teid_t_ PacketDetectionRulesImpl::getTeid()
{
  LOG_FUNC();
  return mPdr.pdi.fteid.teid;
}

pdr_id_t_ PacketDetectionRulesImpl::getPdrId()
{
  LOG_FUNC();
  return mPdr.pdr_id;
}

precedence_t_ PacketDetectionRulesImpl::getPrecedence()
{
  LOG_FUNC();
  return mPdr.precedence;
}

pdi_t_ PacketDetectionRulesImpl::getPdi()
{
  LOG_FUNC();
  return mPdr.pdi;
}

outer_header_removal_t_ PacketDetectionRulesImpl::getOuterHeaderRemoval()
{
  LOG_FUNC();
  return mPdr.outer_header_removal;
}

far_id_t_ PacketDetectionRulesImpl::getFarId()
{
  LOG_FUNC();
  return mPdr.far_id;
}

urr_id_t_ PacketDetectionRulesImpl::gerUrrId()
{
  LOG_FUNC();
  return mPdr.urr_id;
}

qer_id_t_ PacketDetectionRulesImpl::getQerId()
{
  LOG_FUNC();
  return mPdr.qer_id;
}

activate_predefined_rules_t_ PacketDetectionRulesImpl::getActivatePredefinedRules()
{
  LOG_FUNC();
  return mPdr.activate_predefined_rules;
}

pfcp_pdr_t_ PacketDetectionRulesImpl::getData()
{
  LOG_FUNC();
  return mPdr;
}
