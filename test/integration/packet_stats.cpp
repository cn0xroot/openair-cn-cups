#include "packet_stats.hpp"
#include <PcapLiveDevice.h>
#include <IPv4Layer.h>
#include <iostream>

packet_stats::packet_stats(const std::string& destination_address)
  : m_destination_address(destination_address)
{
  clear();
}

packet_stats::~packet_stats()
{
}

void packet_stats::on_packet_arrives(pcpp::RawPacket *packet, pcpp::PcapLiveDevice *dev, void *cookie)
{
  // extract the stats object form the cookie
  packet_stats *stats = (packet_stats *)cookie;

  // parsed the raw packet
  pcpp::Packet parsedPacket(packet);

  // collect stats from packet
  stats->consume_packet(parsedPacket);
}


void packet_stats::consume_packet(pcpp::Packet &packet)
{
  if (packet.isPacketOfType(pcpp::IPv4)) {
    auto destination_addresss = static_cast<pcpp::IPv4Layer*>(packet.getLayerOfType(pcpp::IPv4))->getDstIpAddress();
    std::cout << "Destination address " << destination_addresss.toString() << std::endl;

    if(destination_addresss == m_destination_address){
      m_ipv4_count++;
    }
  }
}

void packet_stats::clear()
{
  m_ipv4_count = 0;
}


unsigned int packet_stats::get_ipv4_count() const
{
 return m_ipv4_count; 
}