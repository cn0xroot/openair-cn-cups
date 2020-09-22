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

/*! \file pfcp_switch.hpp
   \author  Lionel GAUTHIER
   \date 2019
   \email: lionel.gauthier@eurecom.fr
*/

#ifndef FILE_PFCP_SWITCH_HPP_SEEN
#define FILE_PFCP_SWITCH_HPP_SEEN

//#include "concurrentqueue.h"
#include "itti.hpp"
#include "itti_msg_sxab.hpp"
#include "msg_pfcp.hpp"
#include "pfcp_session.hpp"
#include "uint_generator.hpp"
#include "thread_sched.hpp"

#include <folly/AtomicHashMap.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <unordered_map>
#include <memory>
#include <netinet/in.h>
#include <thread>
#include <vector>

class RulesUtilitiesImpl;

namespace spgwu {

  // Have to be tuned for sdt situations
#define PFCP_SWITCH_MAX_SESSIONS 512
#define PFCP_SWITCH_MAX_PDRS     512
#define PDN_INTERFACE_NAME      "pdn"
//class switching_records{
//public:
//  switching_records() {
//    cp_fseid2pfcp_sessions = {};
//    up_seid2pfcp_sessions = {};
//    ul_s1u_teid2pfcp_pdr = {};
//    ue_ipv4_hbo2pfcp_pdr = {};
//  }
//  std::unordered_map<pfcp::fseid_t, std::shared_ptr<pfcp::pfcp_session>>                cp_fseid2pfcp_sessions;
//  std::unordered_map<uint64_t, std::shared_ptr<pfcp::pfcp_session>>                           up_seid2pfcp_sessions;
//};
//
//class switching_data_per_cpu_socket {
//public:
//  switching_data_per_cpu_socket() {
//    msg_iov_= {};
//    switching_up_traffic_index = 0;
//    switching_control_index = 1;
//    switching_records[0] = {};
//    switching_records[1] = {};
//  }
//  switching_records                         switching_records[2];
//  struct iovec                              msg_iov_;        /* scatter/gather array */
//  uint8_t                                   switching_up_traffic_index;
//  uint8_t                                   switching_control_index;
//};

class pfcp_switch {
private:
  // Very unoptimized
#define PFCP_SWITCH_RECV_BUFFER_SIZE 2048
#define ROOM_FOR_GTPV1U_G_PDU        64
  char                                      recv_buffer_[PFCP_SWITCH_RECV_BUFFER_SIZE];
  int                                       sock_r;
  int                                       sock_w;
  std::thread                               thread_sock_;
  //std::string                               gw_mac_address;
  int                                       pdn_if_index;


  util::uint_generator<uint64_t>   seid_generator_;
  util::uint_generator<teid_t>     teid_s1u_generator_;

#define TASK_SPGWU_PFCP_SWITCH_MAX_COMMIT_INTERVAL     (0)
#define TASK_SPGWU_PFCP_SWITCH_MIN_COMMIT_INTERVAL     (1)

#define PFCP_SWITCH_MAX_COMMIT_INTERVAL_MILLISECONDS 200
#define PFCP_SWITCH_MIN_COMMIT_INTERVAL_MILLISECONDS  50

  //switching_data_per_cpu_socket             switching_data[];
  
   // scatter/gather array 
  struct iovec                              msg_iov_;        

  std::unordered_map<pfcp::fseid_t, std::shared_ptr<pfcp::pfcp_session>>                        cp_fseid2pfcp_sessions;
  folly::AtomicHashMap<uint64_t, std::shared_ptr<pfcp::pfcp_session>>                           up_seid2pfcp_sessions;
  folly::AtomicHashMap<teid_t, std::shared_ptr<std::vector<std::shared_ptr<pfcp::pfcp_pdr>>>>   ul_s1u_teid2pfcp_pdr;
  folly::AtomicHashMap<uint32_t, std::shared_ptr<std::vector<std::shared_ptr<pfcp::pfcp_pdr>>>> ue_ipv4_hbo2pfcp_pdr;

  //moodycamel::ConcurrentQueue<pfcp::pfcp_session*>            create_session_q;

  
  /**
   * @brief Loop for reading pdn packets
   * 
   * @param sched_params Parametros da thread.
   */
  void pdn_read_loop(const util::thread_sched_params& sched_params);
  int create_pdn_socket (const char * const ifname, const bool promisc, int& if_index);
  int create_pdn_socket (const char * const ifname);

  /**
   * @brief Set the up pdn interfaces object
   * Condigure ipv4 for the interface created
   */
  void setup_pdn_interfaces();

  timer_id_t timer_max_commit_interval_id;
  timer_id_t timer_min_commit_interval_id;

  void stop_timer_min_commit_interval();
  void start_timer_min_commit_interval();
  void stop_timer_max_commit_interval();
  void start_timer_max_commit_interval();

  void commit_changes();


  bool get_pfcp_session_by_cp_fseid(const pfcp::fseid_t&, std::shared_ptr<pfcp::pfcp_session>&) const;
  bool get_pfcp_session_by_up_seid(const uint64_t, std::shared_ptr<pfcp::pfcp_session>&) const;
  bool get_pfcp_ul_pdrs_by_up_teid(const teid_t, std::shared_ptr<std::vector<std::shared_ptr<pfcp::pfcp_pdr>>>&) const;
  bool get_pfcp_dl_pdrs_by_ue_ip(const uint32_t, std::shared_ptr<std::vector<std::shared_ptr<pfcp::pfcp_pdr>>>&) const;

  void add_pfcp_session_by_cp_fseid(const pfcp::fseid_t&, std::shared_ptr<pfcp::pfcp_session>&);
  void add_pfcp_session_by_up_seid(const uint64_t, std::shared_ptr<pfcp::pfcp_session>&);
  void add_pfcp_ul_pdr_by_up_teid(const teid_t teid, std::shared_ptr<pfcp::pfcp_pdr>& );

  void remove_pfcp_session(std::shared_ptr<pfcp::pfcp_session>&);



  uint64_t generate_seid() {return seid_generator_.get_uid();};

  teid_t generate_teid_s1u() {return teid_s1u_generator_.get_uid();};

public:
  std::shared_ptr<RulesUtilitiesImpl> mpRulesImpl;
  pfcp_switch();
  pfcp_switch(pfcp_switch const&)    = delete;
  void operator=(pfcp_switch const&)     = delete;

  void add_pfcp_dl_pdr_by_ue_ip(const uint32_t ue_ip, std::shared_ptr<pfcp::pfcp_pdr>& );

  pfcp::fteid_t generate_fteid_s1u();
  bool create_packet_in_access(std::shared_ptr<pfcp::pfcp_pdr>& pdr, const pfcp::fteid_t& in, uint8_t& cause);

  /**
   * @brief Process packet flow in UP function - UE to PDN (uplink).
   * @details See TS 29.244 version 15.5.0 Release 15.
   * Figure 5.2.1-1: Packet processing flow in the UP function.
   * 
   * @param iph The ip header.
   * @param num_bytes The number of bytes TODO: complete.
   * @param r_endpoint The endpoint socket.
   * @param tunnel_id The tunnel idenfication.
   */
  void pfcp_session_look_up_pack_in_access(struct iphdr* const iph, const std::size_t num_bytes, const endpoint& r_endpoint, const uint32_t tunnel_id);
  void pfcp_session_look_up_pack_in_access(struct ipv6hdr* const iph, const std::size_t num_bytes, const endpoint& r_endpoint, const uint32_t tunnel_id);
  void pfcp_session_look_up_pack_in_access(struct iphdr* const iph, const std::size_t num_bytes, const endpoint& r_endpoint) {};
  void pfcp_session_look_up_pack_in_access(struct ipv6hdr* const iph, const std::size_t num_bytes, const endpoint& r_endpoint) {};
  //void pfcp_session_look_up(struct ethhdr* const ethh, const std::size_t num_bytes);

  /**
   * @brief Packet from PDN to UE (downlink)
   * 
   * @param buffer 
   * @param num_bytes 
   */
  void pfcp_session_look_up_pack_in_core(const char *buffer, const std::size_t num_bytes);

  void send_to_core(char* const ip_packet, const ssize_t len);

  void handle_pfcp_session_establishment_request(std::shared_ptr<itti_sxab_session_establishment_request> sreq, itti_sxab_session_establishment_response* );
  void handle_pfcp_session_modification_request(std::shared_ptr<itti_sxab_session_modification_request> sreq, itti_sxab_session_modification_response* );
  void handle_pfcp_session_deletion_request(std::shared_ptr<itti_sxab_session_deletion_request> sreq, itti_sxab_session_deletion_response* );

  void time_out_min_commit_interval(const uint32_t timer_id);
  void time_out_max_commit_interval(const uint32_t timer_id);

  void remove_pfcp_session(const pfcp::fseid_t& cp_fseid);
  void remove_pfcp_ul_pdrs_by_up_teid(const teid_t);
  void remove_pfcp_dl_pdrs_by_ue_ip(const uint32_t);

  std::string to_string() const;
};
}
#endif /* FILE_PFCP_SWITCH_HPP_SEEN */
