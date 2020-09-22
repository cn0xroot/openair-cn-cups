################################################################################
# Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The OpenAirInterface Software Alliance licenses this file to You under
# the OAI Public License, Version 1.1  (the "License"); you may not use this file
# except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.openairinterface.org/?page_id=698
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#-------------------------------------------------------------------------------
# For more information about the OpenAirInterface (OAI) Software Alliance:
#      contact@openairinterface.org
################################################################################
include_directories(${SRC_TOP_DIR}/common)
include_directories(${SRC_TOP_DIR}/common/msg)
include_directories(${SRC_TOP_DIR}/common/utils)
include_directories(${SRC_TOP_DIR}/itti)
include_directories(${SRC_TOP_DIR}/gtpv1u)
include_directories(${SRC_TOP_DIR}/gtpv2c)
include_directories(${SRC_TOP_DIR}/oai_spgwu)
include_directories(${SRC_TOP_DIR}/pfcp)
include_directories(${SRC_TOP_DIR}/spgwu)
include_directories(${SRC_TOP_DIR}/udp)
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/include)
include_directories(${SRC_TOP_DIR}/../build/ext/spdlog/include)
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/src)
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/src/bpf)
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/src/skel)
# include_directories(/workspaces/openair-cn-cups/build/ext/upf-bpf/package/include)

#find_library(FOLLY folly)
find_package(UpfBpf CONFIG REQUIRED)

add_library (SPGW_SWITCH STATIC
  pfcp_far.cpp
  pfcp_pdr.cpp
  pfcp_session.cpp
  pfcp_switch.cpp
  spgwu_s1u.cpp
  )

file(GENERATE 
    OUTPUT "includes-spgwu-switch.txt" 
    CONTENT "$<TARGET_PROPERTY:SPGW_SWITCH,INCLUDE_DIRECTORIES>\n"
)

  target_link_libraries(SPGW_SWITCH INTERFACE UpfBpf::upf_xdp)
  # add_dependencies(SPGW_SWITCH upf_bpf)
  
