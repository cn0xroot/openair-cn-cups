// [BGN] main copied from oai_spgwu.cpp
// TODO (navarrothiago) Refactor.
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
// [END] main copied from oai_spgwu.cpp