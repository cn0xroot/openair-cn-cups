# Locate GTest.
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
enable_testing()

# pcapplusplus.
include_directories("/usr/local/include/pcapplusplus")

# Enable SPDLOG debug.
add_definitions("-DSPDLOG_DEBUG_ON")

# Change visibily off all private to public.
# Easy for testing.
# add_definitions("-Dprivate=public")

################################################################################
# Specific part for oai_spgw_u folder

add_definitions("-DPACKAGE_NAME=\"SPGW-U\"")

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
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/src)
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/src/bpf)
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/include)
include_directories(${SRC_TOP_DIR}/../build/ext/upf-bpf/src/skel)

set(SRC_TEST 
    gtpu_l4_stack_test.cpp
    packet_stats.cpp
    spgwu_tests.cpp
    ${SRC_TOP_DIR}/oai_spgwu/options.cpp
    ${SRC_TOP_DIR}/itti/itti.cpp
    ${SRC_TOP_DIR}/itti/itti_msg.cpp
)


# Workaround - check https://github.com/navarrothiago/upf-bpf/issues/23
add_definitions(-DPCAP_DONT_INCLUDE_PCAP_BPF_H)

add_executable(spgwu-test ${SRC_TEST})
target_link_libraries (spgwu-test ${ASAN} -Wl,--start-group CN_UTILS SPGWU SPGW_SWITCH UDP GTPV1U PFCP 3GPP_COMMON_TYPES gflags glog dl double-conversion folly -Wl,--end-group
pthread m rt config++ event boost_system ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}   /usr/local/lib/libPacket++.a /usr/local/lib/libPcap++.a /usr/local/lib/libCommon++.a pcap
UpfBpf::upf_xdp
)

# add_test(NAME ${BINARY} COMMAND ${BINARY})
