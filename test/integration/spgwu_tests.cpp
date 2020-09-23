#include "gtest/gtest.h"
#include "gtpv1u.hpp"
#include "gtpu.h"
#include "gtpu_l4_stack_test.hpp"

// Pcapplusplus
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <RawPacket.h>
#include <stdlib.h>
#include <PcapLiveDeviceList.h>
#include <PlatformSpecificUtils.h>

// includes for spgwu components
#include "spgwu_tests_utils.hpp"

// for test fixture
#include "3gpp_29.244.h"
#include "packet_stats.hpp"

class SpgwuTests : public ::testing::Test
{
public:
  /**
   * @brief Construct a new Spgwu Tests object.
   * @details Initializa spgwu_app which has a gtpu server for incoming messages. 
   */
  SpgwuTests()
      : m_clientPort(8000),
        m_newPacket(100),
        m_source_address("192.168.15.1"),
        m_destination_address("10.0.0.1"),
        m_sgi_iface("eth0")

  {
    init_spgwu_app();
    init_gtp_packet();
    mp_gptu_client = std::make_shared<gtpu_l4_stack_test>(spgwu_cfg.s1_up.addr4, m_clientPort, spgwu_cfg.s1_up.thread_rd_sched_params);
  }

  void SetUp()
  {
  }

  void TearDown()
  {
  }

  void init_gtp_packet()
  {
    memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(2152);
    m_serverAddr.sin_addr.s_addr = INADDR_ANY;

    // create a new IPv4 layer.
    m_newIPLayer = pcpp::IPv4Layer(pcpp::IPv4Address(m_source_address), pcpp::IPv4Address(m_destination_address));
    m_newIPLayer.getIPv4Header()->ipId = htons(2000);
    m_newIPLayer.getIPv4Header()->timeToLive = 64;

    // add all the layers we created
    m_newPacket.addLayer(&m_newIPLayer);

    // compute all calculated fields
    m_newPacket.computeCalculateFields();

    // G-PDU GTP - T-PDU + Header GTP - where T-PDU correspond to an IP datagram and is the payload tunneled in
    //the user GTP tunnel associated  with  the concerned  PDP context.
    memset(&m_header, 0, sizeof(m_header));

    // T-PDU (IP datagram).
    m_payloadLength = m_newPacket.getRawPacket()->getRawDataLen();
    // allocate header size in packet.
    m_gtpuPacket = std::vector<char>(sizeof(m_header), 0);
    // copy payload.
    m_gtpuPacket.insert(m_gtpuPacket.end(), m_newPacket.getRawPacket()->getRawData(), m_newPacket.getRawPacket()->getRawData() + m_payloadLength);

    // save payload size in m_header field.
    m_header.message_length = m_gtpuPacket.size();
  }

  void init_spgwu_app()
  {
    // char *argv[] = {"program-name", "arg1", "arg2", ..., "argn", NULL}
    char *argv[] = {"spwgu-test", "-c", "/workspaces/openair-cn-cups/etc/spgw_u-dev.conf", "-o", NULL};
    int argc = sizeof(argv) / sizeof(char *) - 1;

    // Command line options
    if (!Options::parse(argc, argv))
    {
      std::cout << "Options::parse() failed" << std::endl;
    }

    // Logger
    Logger::init("spgwu", Options::getlogStdout(), Options::getlogRotFilelog());

    Logger::spgwu_app().startup("Options parsed");

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_app_signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    // Config
    spgwu_cfg.load(Options::getlibconfigConfig());
    spgwu_cfg.display();

    // Inter task Interface
    itti_inst = new itti_mw();
    itti_inst->start(spgwu_cfg.itti.itti_timer_sched_params);

    // system command
    async_shell_cmd_inst = new async_shell_cmd(spgwu_cfg.itti.async_cmd_sched_params);

    // PGW application layer
    spgwu_app_inst = new spgwu_app(Options::getlibconfigConfig());

    // PID file
    // Currently hard-coded value. TODO: add as config option.
    string pid_file_name = get_exe_absolute_path("/var/run", spgwu_cfg.instance);
    if (!is_pid_file_lock_success(pid_file_name.c_str()))
    {
      Logger::spgwu_app().error("Lock PID file %s failed\n", pid_file_name.c_str());
      exit(-EDEADLK);
    }

    FILE *fp = NULL;
    std::string filename = fmt::format("/tmp/spgwu_{}.status", getpid());
    fp = fopen(filename.c_str(), "w+");
    fprintf(fp, "STARTED\n");
    fflush(fp);
    fclose(fp);

    // once all udp servers initialized
    io_service.run();
  }

  void send_session_establishment_request(uint32_t &teid)
  {
    pfcp::fteid_t fteid;
    memset(&fteid, 0, sizeof(pfcp::fteid_t));
    fteid.ch = true;
    uint64_t lseid = 0;
    uint8_t cause = pfcp::CAUSE_VALUE_SYSTEM_FAILURE;

    // Table 7.5.2.2-1: Create PDR IE within PFCP Session Establishment Request
    // Check pfcp::pfcp_pdr::look_up_pack_in_access
    pfcp::create_pdr create_pdr;
    create_pdr.set(pfcp::pdr_id_t());

    //  Create PDI IE for Create PDR.
    pfcp::pdi pdi;
    pdi.set(pfcp::source_interface_t{pfcp::INTERFACE_VALUE_ACCESS});
    pdi.set(pfcp::ue_ip_address_t{
      ipv6d : 0, // This bit is only applicable to the UE IP address IE in the PDI IE and whhen V6 bit is set to "1". If this bit is set to "1", then the IPv6 Prefix Delegation Bits field shall be present, otherwise the UP function shall consider IPv6 prefix is default /64.
      sd : 0,    // This bit is only applicable to the UE IP Address IE in the PDI IE. It shall be set to "0" and ignored by the receiver in IEs other than PDI IE. In the PDI IE, if this bit is set to "0", this indicates a Source IP address; if this bit is set to "1", this indicates a Destination IP address.
      v4 : 1,    // If this bit is set to "1", then the IPv4 address field shall be present in the UE IP Address, otherwise the IPv4 address field shall not be present.
      v6 : 0,    // If this bit is set to "1", then the IPv6 address field shall be present in the UE IP Address, otherwise the IPv6 address field shall not be present.
      ipv4_address : *m_newPacket.getLayerOfType<pcpp::IPv4Layer>()->getSrcIpAddress().toInAddr()
    });
    pdi.set(fteid);
    pdi.set(pfcp::sdf_filter_t());
    create_pdr.set(pdi);

    // Create Outer Header Removal IE.
    create_pdr.set(pfcp::outer_header_removal_t{
        .outer_header_removal_description = OUTER_HEADER_REMOVAL_GTPU_UDP_IPV4});

    // Create precedence for Create PDR IE.
    create_pdr.set(pfcp::precedence_t{0});
    std::shared_ptr<pfcp::pfcp_pdr> pdr = std::make_shared<pfcp::pfcp_pdr>(create_pdr);

    // Create paramenter for Create FAR IE.
    pfcp::far_id_t far_id = {0};
    pfcp::apply_action_t far_apply_action;
    far_apply_action.forw = true;
    far_apply_action.dupl = false;
    pfcp::forwarding_parameters fowarding_parameters;
    fowarding_parameters.set(pfcp::destination_interface_t{pfcp::INTERFACE_VALUE_CORE});

    // Create FAR
    pfcp::create_far create_far;
    create_far.set(far_id);
    create_far.set(far_apply_action);
    create_far.set(fowarding_parameters);
    create_pdr.set(far_id);

    // Create source request.
    std::shared_ptr<itti_sxab_session_establishment_request> sreq;
    task_id_t origin = TASK_FIRST, destination = TASK_FIRST;
    sreq = std::make_shared<itti_sxab_session_establishment_request>(origin, destination);

    // Initialize IE in request.
    pfcp::fseid_t fseid;
    memset(&fseid, 0, sizeof(pfcp::fseid_t));
    sreq->pfcp_ies.set(fseid);
    sreq->pfcp_ies.set(create_pdr);
    sreq->pfcp_ies.set(create_far);

    std::shared_ptr<itti_sxab_session_establishment_response> resp;
    resp = std::make_shared<itti_sxab_session_establishment_response>(origin, destination);

    // Session establishment request.
    pfcp_switch_inst->handle_pfcp_session_establishment_request(sreq, resp.get());
    ASSERT_EQ(resp->pfcp_ies.created_pdrs.size(), 1);
    teid = resp->pfcp_ies.created_pdrs[0].local_fteid.second.teid;
    ASSERT_EQ(teid, 1);
  }

  // gtpu client.
  std::shared_ptr<gtpu_l4_stack_test> mp_gptu_client;

  // server address.
  struct sockaddr_in m_serverAddr;

  // server port.
  u_int32_t m_clientPort;

  // IP PDU
  pcpp::IPv4Layer m_newIPLayer;

  // create a packet with initial capacity of 100 bytes (will grow automatically if needed)
  pcpp::Packet m_newPacket;

  // T-PDU (IP datagram).
  u_int32_t m_payloadLength;
  std::vector<char> m_gtpuPacket;

  // m_header.
  struct gtpuhdr m_header;

  // Source address for gtp packet.
  std::string m_source_address;

  // Destination address for gtp packet.
  std::string m_destination_address;

  // SGi interface. TODO navarrothiago Get it from config file.
  std::string m_sgi_iface;
};

// Inject G_PDU to spgwu_s1u stack using gtpu_l4_stack client.
// Create GTP packet - https://pcapplusplus.github.io/docs/tutorials/packet-crafting#packet-creation
// Send GTP packet to network device - https://pcapplusplus.github.io/docs/tutorials/capture-packets#sending-packets
// Receive GTPU_ERROR_INDICATION
TEST_F(SpgwuTests, send_GTPU_G_PDU_received_GTPU_ERROR_INDICATION)
{
  // Message expected to be received.
  mp_gptu_client->message_type_expected(GTPU_ERROR_INDICATION);

  // send data.
  Logger::gtpv1_u().debug("send_GTPU_G_PDU");
  mp_gptu_client->send_g_pdu(m_serverAddr, static_cast<teid_t>(0), m_gtpuPacket.data() + sizeof(m_header), m_payloadLength);

  // pause, cin.get is portable
  std::cin.get();
}

TEST_F(SpgwuTests, send_session_establishment_request)
{
  uint32_t teid;
  send_session_establishment_request(teid);

  // Pause.
  std::cin.get();
}

TEST_F(SpgwuTests, send_GTPU_G_PDU_success)
{
  uint32_t teid;
  send_session_establishment_request(teid);

  // Find the interface by IP address.
  // Check SGi in spgw_u-dev.conf file.
  pcpp::PcapLiveDevice *dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByName(m_sgi_iface);
  ASSERT_TRUE(dev != NULL);
  ASSERT_TRUE(dev->open());

  // Create the stats object
  packet_stats stats(m_destination_address);

  // start capture in async mode. Give a callback function to call to whenever a packet is captured and the stats object as the cookie
  ASSERT_TRUE(dev->startCapture(packet_stats::on_packet_arrives, &stats));

  // Send data to core access. It should apply the Rules.
  Logger::gtpv1_u().debug("send_GTPU_G_PDU");
  mp_gptu_client->send_g_pdu(m_serverAddr, teid, m_gtpuPacket.data() + sizeof(m_header), m_payloadLength);

  // Sleep for 10 seconds in main thread, in the meantime packets are captured in the async thread
  PCAP_SLEEP(4);

  // Expect to receive some data in pdn interface.
  ASSERT_GT(stats.get_ipv4_count(), 0);

  // Pause.
  std::cin.get();

  // Stop capturing packets.
  dev->stopCapture();
}