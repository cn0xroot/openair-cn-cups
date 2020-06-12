# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
enable_testing()

include_directories("/usr/local/include/pcapplusplus")

################################################################################
# Specific part for oai_spgw_u folder

add_definitions("-DPACKAGE_NAME=\"SPGW-U\"")
add_definitions("-DSPDLOG_DEBUG_ON")

include_directories(${SRC_TOP_DIR}/oai_spgwu)
include_directories(${SRC_TOP_DIR}/common)
include_directories(${SRC_TOP_DIR}/itti)
include_directories(${SRC_TOP_DIR}/common/msg)
include_directories(${SRC_TOP_DIR}/common/utils)
include_directories(${SRC_TOP_DIR}/gtpv1u)
include_directories(${SRC_TOP_DIR}/pfcp)
include_directories(${SRC_TOP_DIR}/spgwu)
include_directories(${SRC_TOP_DIR}/spgwu/simpleswitch)
include_directories(${SRC_TOP_DIR}/udp)
include_directories(${SRC_TOP_DIR}/../build/ext/spdlog/include)

set(SRC_TEST 
    gtpu_l4_stack_test.cpp
    spgwu_tests.cpp
    ${SRC_TOP_DIR}/oai_spgwu/options.cpp
    ${SRC_TOP_DIR}/itti/itti.cpp
    ${SRC_TOP_DIR}/itti/itti_msg.cpp
)


add_executable(spgwu-test ${SRC_TEST})
target_link_libraries (spgwu-test ${ASAN}  -Wl,--start-group CN_UTILS SPGWU SPGW_SWITCH UDP GTPV1U PFCP 3GPP_COMMON_TYPES gflags glog dl double-conversion folly -Wl,--end-group
m rt config++ event boost_system ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}  /usr/local/lib/libPcap++.a /usr/local/lib/libPacket++.a /usr/local/lib/libCommon++.a)

# add_test(NAME ${BINARY} COMMAND ${BINARY})
# 