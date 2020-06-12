#include "gtest/gtest.h"
#include "gtpv1u.hpp"
#include "gtpu.h"
#include "gtpu_l4_stack_test.hpp"

// Pcapplusplus
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <RawPacket.h>

// [BGN] main
#include "async_shell_cmd.hpp"
#include "common_defs.h"
#include "itti.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "pfcp_switch.hpp"
#include "pid_file.hpp"
#include "spgwu_app.hpp"
#include "spgwu_config.hpp"

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <signal.h>
#include <stdint.h>
#include <unistd.h> // get_pid(), pause()
#include <vector>

using namespace spgwu;
using namespace util;
using namespace std;

itti_mw *itti_inst = nullptr;
async_shell_cmd *async_shell_cmd_inst = nullptr;
pfcp_switch *pfcp_switch_inst = nullptr;
spgwu_app *spgwu_app_inst = nullptr;
spgwu_config spgwu_cfg;
boost::asio::io_service io_service;

void my_app_signal_handler(int s)
{
  std::cout << "Caught signal " << s << std::endl;
  Logger::system().startup("exiting");
  itti_inst->send_terminate_msg(TASK_SPGWU_APP);
  itti_inst->wait_tasks_end();
  std::cout << "Freeing Allocated memory..." << std::endl;
  if (async_shell_cmd_inst)
    delete async_shell_cmd_inst;
  async_shell_cmd_inst = nullptr;
  std::cout << "Async Shell CMD memory done." << std::endl;
  if (itti_inst)
    delete itti_inst;
  itti_inst = nullptr;
  std::cout << "ITTI memory done." << std::endl;
  if (spgwu_app_inst)
    delete spgwu_app_inst;
  spgwu_app_inst = nullptr;
  std::cout << "SPGW-U APP memory done." << std::endl;
  std::cout << "Freeing Allocated memory done" << std::endl;
  exit(0);
}

// [END] main

class SpgwuTests : public ::testing::Test
{
public:
  SpgwuTests()
  {
    // initialization code here
  }

  void SetUp()
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

    // pause();
    // return 0;
  }

  void TearDown()
  {
  }
};

// Inject G_PDU to spgwu_s1u stack using gtpu_l4_stack client.
// Create GTP packet - https://pcapplusplus.github.io/docs/tutorials/packet-crafting#packet-creation
// Send GTP packet to network device - https://pcapplusplus.github.io/docs/tutorials/capture-packets#sending-packets
TEST_F(SpgwuTests, send_GTPU_G_PDU_received_GTPU_ERROR_INDICATION)
{
   u_int32_t clientPort = 8000;
  gtpu_l4_stack_test gptu_client(spgwu_cfg.s1_up.addr4, clientPort, spgwu_cfg.s1_up.thread_rd_sched_params);

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(2152);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  // create a new IPv4 layer.
  pcpp::IPv4Layer newIPLayer(pcpp::IPv4Address(std::string("192.168.15.1")), pcpp::IPv4Address(std::string("10.0.0.1")));
  newIPLayer.getIPv4Header()->ipId = htons(2000);
  newIPLayer.getIPv4Header()->timeToLive = 64;

  // create a packet with initial capacity of 100 bytes (will grow automatically if needed)
  pcpp::Packet newPacket(100);

  // add all the layers we created
  newPacket.addLayer(&newIPLayer);

  // compute all calculated fields
  newPacket.computeCalculateFields();

  // G-PDU GTP - T-PDU + Header GTP - where T-PDU correspond to an IP datagram and is the payload tunneled in 
  //the user GTP tunnel associated  with  the concerned  PDP context. 

  // header.
  struct gtpuhdr header;
  memset(&header, 0, sizeof(header));

  // T-PDU (IP datagram).
  u_int32_t payloadLength = newPacket.getRawPacket()->getRawDataLen();
  std::vector<char> gtpuPacket(sizeof(header), 0);
  gtpuPacket.insert(gtpuPacket.end(), newPacket.getRawPacket()->getRawData(), newPacket.getRawPacket()->getRawData() + payloadLength);
  
  // save payload size in header field.
  header.message_length = gtpuPacket.size();

  // Message expected to be received.
  gptu_client.message_type_expected(GTPU_ERROR_INDICATION);

  // send data.
  std::cout << "send_GTPU_G_PDU" << std::endl;
  gptu_client.send_g_pdu(serverAddr, static_cast<teid_t>(0), gtpuPacket.data() + sizeof(header), payloadLength);

  // pause, cin.get is portable
  std::cin.get();
}
