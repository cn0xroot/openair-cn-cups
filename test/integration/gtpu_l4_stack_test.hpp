#include "gtpv1u.hpp"
#include <netinet/in.h>

class gtpu_l4_stack_test : public gtpv1u::gtpu_l4_stack
{
public:
  /**
   * @brief Construct a new gtpu l4 stack test object.
   * 
   * @param address Server address.
   * @param port_num Server port.
   * @param sched_params Thread params.
   */
  gtpu_l4_stack_test(const struct in_addr& address, const uint16_t port_num, const util::thread_sched_params& sched_params);
  /**
   * @brief Destroy the gtpu l4 stack test object.
   */
  ~gtpu_l4_stack_test();
  // From gtpv1u::gtpu_l4_stack.
  virtual void handle_receive(char* recv_buffer, const std::size_t bytes_transferred, const endpoint& r_endpoint);
  /**
   * @brief Message type expected when receive a gtp message. 
   * 
   * @param message_type_expected Expected message type received.
   */
  void message_type_expected(uint8_t message_type_expected);
private:
  uint8_t m_message_type;
};


