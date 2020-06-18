#include <RawPacket.h>
#include <PcapLiveDevice.h>
#include <Packet.h>

/**
 * @brief Class for packet stats for pcapplusplus.
 */
class packet_stats
{
public:
  /**
   * @brief Constructor.
   * 
   * @param destination_address The destination address that will be analysed.
   */
  packet_stats(const std::string &destination_address);
  /**
   * @brief Destroy the packet stats object.
   * 
   */
  virtual ~packet_stats();
  /**
   * @brief Callback function used async when a packet is received.
   * 
   * @param packet Packet received. 
   * @param dev Device opened.
   * @param cookie Cookies from client.
   */
  static void on_packet_arrives(pcpp::RawPacket *packet, pcpp::PcapLiveDevice *dev, void *cookie);
  /**
   * @brief Logic to consume the packet.
   * 
   * @param packet The packet received.
   */
  void consume_packet(pcpp::Packet &packet);
  /**
   * @brief Clear counters.
   */
  void clear();
  /**
   * @brief Get the ipv4 count object.
   * 
   * @return unsigned int Number of packets received address to destination address.
   */
  unsigned int get_ipv4_count() const;

private:
  /**
   * @brief Packet counter.
   */
  unsigned int m_ipv4_count;
  /**
   * @brief Destination address that will be analysed for counting.
   */
  std::string m_destination_address;
};
