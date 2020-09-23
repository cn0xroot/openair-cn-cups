#include "SessionBpfImpl.h"
#include <utils/LogDefines.h>

SessionBpfImpl::SessionBpfImpl(pfcp::pfcp_session &session) 
  : SessionBpf()
{
  LOG_FUNC();
  mSession.seid = session.seid;
  mSession.fars_counter = 0;
  mSession.pdrs_counter = 0;
}

SessionBpfImpl::~SessionBpfImpl() 
{
  LOG_FUNC();
}

seid_t_ SessionBpfImpl::getSeid() 
{
  LOG_FUNC();
  return mSession.seid;
}

pfcp_session_t_ SessionBpfImpl::getData() 
{
  LOG_FUNC();
  return mSession;
}
