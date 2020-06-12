#include "gtpu_l4_stack_test.hpp"
#include "gtpu.h"
#include "logger.hpp"
#include <gtest/gtest.h>

gtpu_l4_stack_test::gtpu_l4_stack_test(const struct in_addr& address, const uint16_t port_num, const util::thread_sched_params& sched_params) 
  : gtpv1u::gtpu_l4_stack(address, port_num, sched_params), m_message_type(0)
{
  Logger::gtpv1_u().debug("gtpu_l4_stack_test");
}

gtpu_l4_stack_test::~gtpu_l4_stack_test() 
{
  Logger::gtpv1_u().debug("~gtpu_l4_stack_test");
}

void gtpu_l4_stack_test::handle_receive(char* recv_buffer, const std::size_t bytes_transferred, const endpoint& r_endpoint) 
{
  struct gtpuhdr* gtpuh = (struct gtpuhdr*)&recv_buffer[0];

  if (gtpuh->version == 1) {
    // Do it fast, do not go throught handle_receive_gtpv1u_msg()
    Logger::gtpv1_u().debug("GTP messsage type  %d", gtpuh->message_type);
    ASSERT_EQ(m_message_type, GTPU_ERROR_INDICATION);
  }
}

void gtpu_l4_stack_test::message_type_expected(uint8_t message_type_expected) 
{
  m_message_type = message_type_expected;
}