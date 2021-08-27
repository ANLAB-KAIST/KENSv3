/*
 * testenv.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: leeopop
 */

#ifndef APP_TESTTCP_TESTENV_HPP_
#define APP_TESTTCP_TESTENV_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_TimeUtil.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Switch.hpp>
#include <E/Networking/E_Wire.hpp>
#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>
#include <E/Networking/TCP/E_TCPApplication.hpp>
#include <E/Networking/TCP/E_TCPSolution.hpp>

#include "TCPAssignment.hpp"
#include <gtest/gtest.h>

#define RANDOM_SEED_DEFAULT 1614233283

using namespace E;

class KensTesting : public ::testing::Test {
protected:
  void setup_env() {

#ifdef RUN_SOLUTION
    const bool run_solution = true;
#else
    const bool run_solution = false;
#endif

#ifdef UNRELIABLE
    const bool unreliable = true;
#else
    const bool unreliable = false;
#endif

    int seed = RANDOM_SEED_DEFAULT;

    const char *random_seed = std::getenv("RANDOM_SEED");
    if (random_seed) {
      seed = atoi(random_seed);
    }
    srand(seed);
    RecordProperty("random_seed", seed);
    RecordProperty("run_solution", run_solution);
    RecordProperty("unreliable", unreliable);
    printf("[RANDOM_SEED : %d RUN_SOLUTION : %d UNRELIABLE : %d]\n", seed,
           run_solution, unreliable);
  }
};

template <class Target> class TestEnv1 : public KensTesting {
protected:
  NetworkSystem netSystem;
  std::shared_ptr<Host> host1;
  std::shared_ptr<Host> host2;
  std::shared_ptr<Switch> switchingHub;

  virtual void SetUp() {
    setup_env();

    host1 = netSystem.addModule<Host>("TestHost1", netSystem);
    host2 = netSystem.addModule<Host>("TestHost2", netSystem);
    switchingHub = netSystem.addModule<Switch>("Switch1", netSystem);

    auto host1_port_1 = netSystem
                            .addWire(*host1, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;
    auto host1_port_2 = netSystem
                            .addWire(*host1, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;
    auto host2_port_1 = netSystem
                            .addWire(*host2, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;
    auto host2_port_2 = netSystem
                            .addWire(*host2, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;

    mac_t mac1{0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC};
    mac_t mac1_2{0xCB, 0xCB, 0xCB, 0xCB, 0xCB, 0xCB};
    mac_t mac2{0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD};
    mac_t mac2_2{0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC};
    ipv4_t ip1{192, 168, 0, 7};
    ipv4_t ip1_2{192, 168, 0, 8};
    ipv4_t ip2{10, 0, 1, 4};
    ipv4_t ip2_2{10, 0, 1, 5};
    host1->setMACAddr(mac1, host1_port_1.first);
    host1->setMACAddr(mac1_2, host1_port_2.first);
    host1->setARPTable(mac2, ip2);
    host1->setARPTable(mac2_2, ip2_2);
    host1->setIPAddr(ip1, host1_port_1.first);
    host1->setIPAddr(ip1_2, host1_port_2.first);
    host1->setRoutingTable(ip2, 16, host1_port_1.first);
    host1->setRoutingTable(ip2_2, 16, host1_port_2.first);

    host2->setMACAddr(mac2, host2_port_1.first);
    host2->setMACAddr(mac2_2, host2_port_2.first);
    host2->setARPTable(mac1, ip1);
    host2->setARPTable(mac1_2, ip1_2);
    host2->setIPAddr(ip2, host2_port_1.first);
    host2->setIPAddr(ip2_2, host2_port_2.first);
    host2->setRoutingTable(ip1, 16, host2_port_1.first);
    host2->setRoutingTable(ip1_2, 16, host2_port_2.first);

    switchingHub->addMACEntry(host1_port_1.second, mac1);
    switchingHub->addMACEntry(host1_port_2.second, mac1_2);
    switchingHub->addMACEntry(host2_port_1.second, mac2);
    switchingHub->addMACEntry(host2_port_2.second, mac2_2);

    const ::testing::TestInfo *const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();
    std::string file_name(test_info->name());
    file_name.append(".pcap");
    switchingHub->enablePCAPLogging(file_name);

    host1->addHostModule<Ethernet>(*host1);
    host2->addHostModule<Ethernet>(*host2);

    host1->addHostModule<IPv4>(*host1);
    host2->addHostModule<IPv4>(*host2);

    Target::allocate(*host1);
    TCPSolutionProvider::allocate(*host2, false, false, false);

    host1->initializeHostModule("TCP");
    host2->initializeHostModule("TCP");
  }
  virtual void TearDown() {}

  void runTest() {
    netSystem.run(TimeUtil::makeTime(1000, TimeUtil::SEC));

    host1->cleanUp();
    host2->cleanUp();
    host1->finalizeHostModule("TCP");
    host2->finalizeHostModule("TCP");
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

template <class Target, class Adversary, bool Unreliable>
class TestEnv2 : public KensTesting {
protected:
  NetworkSystem netSystem;
  std::shared_ptr<Host> host1;
  std::shared_ptr<Host> host2;
  std::shared_ptr<Switch> switchingHub;

  virtual void SetUp() {
    setup_env();

    host1 = netSystem.addModule<Host>("TestHost1", netSystem);
    host2 = netSystem.addModule<Host>("TestHost2", netSystem);
    switchingHub =
        netSystem.addModule<Switch>("Switch1", netSystem, Unreliable);

    auto host1_port_1 = netSystem
                            .addWire(*host1, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;
    auto host1_port_2 = netSystem
                            .addWire(*host1, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;
    auto host2_port_1 = netSystem
                            .addWire(*host2, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;
    auto host2_port_2 = netSystem
                            .addWire(*host2, *switchingHub,
                                     TimeUtil::makeTime(1, TimeUtil::MSEC))
                            .second;

    mac_t mac1{0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC};
    mac_t mac1_2{0xCB, 0xCB, 0xCB, 0xCB, 0xCB, 0xCB};
    mac_t mac2{0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD};
    mac_t mac2_2{0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC};
    ipv4_t ip1{192, 168, 0, 7};
    ipv4_t ip1_2{192, 168, 0, 8};
    ipv4_t ip2{10, 0, 1, 4};
    ipv4_t ip2_2{10, 0, 1, 5};

    host1->setMACAddr(mac1, host1_port_1.first);
    host1->setMACAddr(mac1_2, host1_port_2.first);
    host1->setARPTable(mac2, ip2);
    host1->setARPTable(mac2_2, ip2_2);
    host1->setIPAddr(ip1, host1_port_1.first);
    host1->setIPAddr(ip1_2, host1_port_2.first);
    host1->setRoutingTable(ip2, 16, host1_port_1.first);
    host1->setRoutingTable(ip2_2, 16, host1_port_2.first);

    host2->setMACAddr(mac2, host2_port_1.first);
    host2->setMACAddr(mac2_2, host2_port_2.first);
    host2->setARPTable(mac1, ip1);
    host2->setARPTable(mac1_2, ip1_2);
    host2->setIPAddr(ip2, host2_port_1.first);
    host2->setIPAddr(ip2_2, host2_port_2.first);
    host2->setRoutingTable(ip1, 16, host2_port_1.first);
    host2->setRoutingTable(ip1_2, 16, host2_port_2.first);

    switchingHub->setQueueSize(16);

    switchingHub->addMACEntry(host1_port_1.second, mac1);
    switchingHub->addMACEntry(host1_port_2.second, mac1_2);
    switchingHub->addMACEntry(host2_port_1.second, mac2);
    switchingHub->addMACEntry(host2_port_2.second, mac2_2);

    const ::testing::TestInfo *const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();
    std::string file_name(test_info->name());
    file_name.append(".pcap");
    switchingHub->enablePCAPLogging(file_name);

    host1->addHostModule<Ethernet>(*host1);
    host2->addHostModule<Ethernet>(*host2);
    host1->addHostModule<IPv4>(*host1);
    host2->addHostModule<IPv4>(*host2);

    Target::allocate(*host1);
    Adversary::allocate(*host2);

    host1->initializeHostModule("TCP");
    host2->initializeHostModule("TCP");
  }
  virtual void TearDown() {}

  void runTest() {
    netSystem.run(TimeUtil::makeTime(1000, TimeUtil::SEC));

    host1->cleanUp();
    host2->cleanUp();
    host1->finalizeHostModule("TCP");
    host2->finalizeHostModule("TCP");
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

template <class Target, class Adversary, int CLIENTS, int TIMEOUT>
class TestEnv3 : public KensTesting {
protected:
  NetworkSystem netSystem;
  std::shared_ptr<Host> server_host;
  std::array<std::shared_ptr<Host>, CLIENTS> client_hosts;
  std::shared_ptr<Switch> switchingHub;

  static constexpr int num_client = CLIENTS;
  Size port_speed = 10000000;
  Time propagationDelay = TimeUtil::makeTime(10, TimeUtil::MSEC);
  uint64_t prev_log;

  virtual void SetUp() {

    setup_env();

    prev_log = NetworkLog::defaultLevel;
    NetworkLog::defaultLevel |= (
        //(1 << NetworkLog::SYSCALL_RAISED) |
        //(1 << NetworkLog::SYSCALL_FINISHED) |
        //(1 << NetworkLog::PACKET_ALLOC) |
        //(1 << NetworkLog::PACKET_CLONE) |
        //(1 << NetworkLog::PACKET_FREE) |
        //(1 << NetworkLog::PACKET_TO_MODULE) |
        //(1 << NetworkLog::PACKET_FROM_MODULE) |
        //(1 << NetworkLog::PACKET_TO_HOST) |
        //(1 << NetworkLog::PACKET_FROM_HOST) |
        //(1 << NetworkLog::PACKET_QUEUE) |
        //(1 << NetworkLog::TCP_LOG) |
        0UL);

    server_host = netSystem.addModule<Host>("CongestionServer", netSystem);
    switchingHub = netSystem.addModule<Switch>("Switch1", netSystem);
    auto server_port =
        netSystem
            .addWire(*server_host, *switchingHub, propagationDelay, port_speed)
            .second;

    mac_t server_mac{0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC};
    ipv4_t server_ip{192, 168, 1, 7};
    ipv4_t server_mask{192, 168, 1, 0};
    ipv4_t client_mask{10, 0, 1, 0};

    server_host->addHostModule<Ethernet>(*server_host);
    server_host->addHostModule<IPv4>(*server_host);

    server_host->setMACAddr(server_mac, server_port.first);
    server_host->setIPAddr(server_ip, server_port.first);
    server_host->setRoutingTable(client_mask, 24, server_port.first);

    Adversary::allocate(*server_host);
    server_host->initializeHostModule("TCP");

    switchingHub->addMACEntry(server_port.second, server_mac);

    char name_buf[128];
    for (int k = 0; k < num_client; k++) {
      snprintf(name_buf, sizeof(name_buf), "CongestionClient%d", k);
      client_hosts[k] = netSystem.addModule<Host>(name_buf, netSystem);
      auto client_port = netSystem
                             .addWire(*client_hosts[k], *switchingHub,
                                      propagationDelay, port_speed)
                             .second;
      Host &client_host = *client_hosts[k];
      client_host.addHostModule<Ethernet>(client_host);
      client_host.addHostModule<IPv4>(client_host);
      Target::allocate(client_host);
      client_host.initializeHostModule("TCP");

      ipv4_t client_ip{10, 0, 1, 10};
      client_ip[3] += k;
      mac_t client_mac{0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x00};
      client_mac[5] += k;

      client_host.setMACAddr(client_mac, client_port.first);
      client_host.setIPAddr(client_ip, client_port.first);
      client_host.setRoutingTable(server_mask, 24, client_port.first);
      client_host.setARPTable(server_mac, server_ip);

      server_host->setARPTable(client_mac, client_ip);
      switchingHub->addMACEntry(client_port.second, client_mac);
    }

    switchingHub->setLinkSpeed(port_speed);
    switchingHub->setQueueSize(64);

    const ::testing::TestInfo *const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();
    std::string file_name(test_info->name());
    file_name.append(".pcap");
    switchingHub->enablePCAPLogging(file_name, 64);
  }
  virtual void TearDown() { NetworkLog::defaultLevel = prev_log; }

  void runTest() {
    netSystem.run(TimeUtil::makeTime(TIMEOUT, TimeUtil::SEC));

    server_host->cleanUp();
    for (int k = 0; k < num_client; k++)
      client_hosts[k]->cleanUp();

    server_host->finalizeHostModule("TCP");
    for (int k = 0; k < num_client; k++)
      client_hosts[k]->finalizeHostModule("TCP");
    netSystem.run(TimeUtil::makeTime(1000 + TIMEOUT, TimeUtil::SEC));
  }
};

//#define UNRELIABLE
//#define RUN_SOLUTION
#ifdef RUN_SOLUTION
typedef TestEnv1<TCPSolutionProvider> TestEnv_Reliable;
#ifdef UNRELIABLE
typedef TestEnv2<TCPSolutionProvider, TCPSolutionProvider, true> TestEnv_Any;
#else
typedef TestEnv2<TCPSolutionProvider, TCPSolutionProvider, false> TestEnv_Any;
#endif
typedef TestEnv2<TCPSolutionProvider, TCPSolutionProvider, true>
    TestEnv_Unreliable;
typedef TestEnv3<TCPSolutionProvider, TCPSolutionProvider, 1, 1000>
    TestEnv_Congestion0;
typedef TestEnv3<TCPSolutionProvider, TCPSolutionProvider, 2, 1000>
    TestEnv_Congestion1;
typedef TestEnv3<TCPSolutionProvider, TCPSolutionProvider, 8, 1000>
    TestEnv_Congestion2;
#else
typedef TestEnv1<TCPAssignmentProvider> TestEnv_Reliable;
#ifdef UNRELIABLE
typedef TestEnv2<TCPAssignmentProvider, TCPSolutionProvider, true> TestEnv_Any;
#else
typedef TestEnv2<TCPAssignmentProvider, TCPSolutionProvider, false> TestEnv_Any;
#endif
typedef TestEnv2<TCPAssignmentProvider, TCPSolutionProvider, true>
    TestEnv_Unreliable;
typedef TestEnv3<TCPAssignmentProvider, TCPSolutionProvider, 1, 1000>
    TestEnv_Congestion0;
typedef TestEnv3<TCPAssignmentProvider, TCPSolutionProvider, 2, 1000>
    TestEnv_Congestion1;
typedef TestEnv3<TCPAssignmentProvider, TCPSolutionProvider, 8, 1000>
    TestEnv_Congestion2;
#endif

#endif /* APP_TESTTCP_TESTENV_HPP_ */
