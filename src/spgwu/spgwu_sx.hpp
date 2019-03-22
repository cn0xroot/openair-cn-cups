/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file spgwu_sx.hpp
   \author  Lionel GAUTHIER
   \date 2019
   \email: lionel.gauthier@eurecom.fr
*/

#ifndef FILE_SGWU_SX_HPP_SEEN
#define FILE_SGWU_SX_HPP_SEEN

#include "pfcp.hpp"
#include "itti_msg_sxab.hpp"
#include "msg_pfcp.hpp"
#include "spgwu_pfcp_association.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include <thread>

namespace oai::cn::nf::spgwu {

#define TASK_SPGWU_SX_TRIGGER_HEARTBEAT_REQUEST     (0)
#define TASK_SPGWU_SX_TIMEOUT_HEARTBEAT_REQUEST     (1)
#define TASK_SPGWU_SX_TIMEOUT_ASSOCIATION_REQUEST   (2)


class spgwu_sx : public oai::cn::proto::pfcp::pfcp_l4_stack {
private:
  std::thread::id                      thread_id;
  std::thread                          thread;

  uint64_t                             recovery_time_stamp; //timestamp in seconds
  core::pfcp::up_function_features_t   up_function_features;

  void start_association(const core::pfcp::node_id_t& node_id);

public:
  spgwu_sx();
  spgwu_sx(spgwu_sx const&)    = delete;
  void operator=(spgwu_sx const&)     = delete;

  void handle_itti_msg (core::itti::itti_sxab_heartbeat_request& s) {};
  void handle_itti_msg (core::itti::itti_sxab_heartbeat_response& s) {};
  void handle_itti_msg (core::itti::itti_sxab_association_setup_request& s) {};
  void handle_itti_msg (core::itti::itti_sxab_association_setup_response& s) {};
  void handle_itti_msg (core::itti::itti_sxab_association_update_request& s) {};
  void handle_itti_msg (core::itti::itti_sxab_association_update_response& s) {};
  void handle_itti_msg (core::itti::itti_sxab_association_release_request& s) {};
  void handle_itti_msg (core::itti::itti_sxab_association_release_response& s) {};
  void handle_itti_msg (core::itti::itti_sxab_version_not_supported_response& s) {};
  void handle_itti_msg (core::itti::itti_sxab_node_report_response& s) {};
  void handle_itti_msg (core::itti::itti_sxab_session_set_deletion_request& s) {};
  void handle_itti_msg (core::itti::itti_sxab_session_establishment_response& s);
  void handle_itti_msg (core::itti::itti_sxab_session_modification_response& s);
  void handle_itti_msg (core::itti::itti_sxab_session_deletion_response& s);
  void handle_itti_msg (core::itti::itti_sxab_session_report_response& s) {};

  void send_sx_msg (core::itti::itti_sxab_heartbeat_request& s) {};
  void send_sx_msg (core::itti::itti_sxab_heartbeat_response& s) {};
  void send_sx_msg (core::itti::itti_sxab_association_setup_request& s);
  void send_sx_msg (core::itti::itti_sxab_association_setup_response& s) {};
  void send_sx_msg (core::itti::itti_sxab_association_update_request& s) {};
  void send_sx_msg (core::itti::itti_sxab_association_update_response& s) {};
  void send_sx_msg (core::itti::itti_sxab_association_release_request& s) {};
  void send_sx_msg (core::itti::itti_sxab_association_release_response& s) {};
  void send_sx_msg (core::itti::itti_sxab_version_not_supported_response& s) {};
  void send_sx_msg (core::itti::itti_sxab_node_report_request& s) {};
  void send_sx_msg (core::itti::itti_sxab_session_set_deletion_response& s) {};
  void send_sx_msg (core::itti::itti_sxab_session_establishment_response& s);
  void send_sx_msg (core::itti::itti_sxab_session_modification_response& s);
  void send_sx_msg (core::itti::itti_sxab_session_deletion_response& s);
  void send_sx_msg (core::itti::itti_sxab_session_report_request& s) {};

  void send_sx_msg(const core::pfcp::fseid_t& cp_fseid, const oai::cn::proto::pfcp::pfcp_session_report_request& s);
  
  void send_heartbeat_request(std::shared_ptr<pfcp_association>& a);
  void send_heartbeat_response(const boost::asio::ip::udp::endpoint& r_endpoint, const uint64_t trxn_id);

  void handle_receive_pfcp_msg( proto::pfcp::pfcp_msg& msg, const boost::asio::ip::udp::endpoint& remote_endpoint);
  void handle_receive(char* recv_buffer, const std::size_t bytes_transferred, boost::asio::ip::udp::endpoint& remote_endpoint);
  // node related
  void handle_receive_heartbeat_request(proto::pfcp::pfcp_msg& msg, const boost::asio::ip::udp::endpoint& remote_endpoint);
  void handle_receive_heartbeat_response(proto::pfcp::pfcp_msg& msg, const boost::asio::ip::udp::endpoint& remote_endpoint);
  void handle_receive_association_setup_response(proto::pfcp::pfcp_msg& msg, const boost::asio::ip::udp::endpoint& remote_endpoint);
  // session related
  void handle_receive_session_establishment_request(proto::pfcp::pfcp_msg& msg, const boost::asio::ip::udp::endpoint& remote_endpoint);
  void handle_receive_session_modification_request(proto::pfcp::pfcp_msg& msg, const boost::asio::ip::udp::endpoint& remote_endpoint);
  void handle_receive_session_deletion_request(proto::pfcp::pfcp_msg& msg, const boost::asio::ip::udp::endpoint& remote_endpoint);



  void time_out_itti_event(const uint32_t timer_id);
};
}
#endif /* FILE_SGWU_SX_HPP_SEEN */
