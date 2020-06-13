#include "gtest/gtest.h"
#include "gtpv1u.hpp"
#include "gtpu.h"
#include "gtpu_l4_stack_test.hpp"

// Pcapplusplus
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <RawPacket.h>

// includes for spgwu components
#include "spgwu_tests_utils.hpp"

class SpgwuTests : public ::testing::Test
{
public:
  /**
   * @brief Construct a new Spgwu Tests object.
   * @details Initializa spgwu_app which has a gtpu server for incoming messages. 
   */
  SpgwuTests()
      : m_clientPort(8000),
        m_newPacket(100)

  {
    init_spgwu_app();
    mp_gptu_client = std::make_shared<gtpu_l4_stack_test>(spgwu_cfg.s1_up.addr4, m_clientPort, spgwu_cfg.s1_up.thread_rd_sched_params);
  }

  void SetUp()
  {
    memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(2152);
    m_serverAddr.sin_addr.s_addr = INADDR_ANY;

    // create a new IPv4 layer.
    m_newIPLayer = pcpp::IPv4Layer(pcpp::IPv4Address(std::string("192.168.15.1")), pcpp::IPv4Address(std::string("10.0.0.1")));
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

  void TearDown()
  {
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
