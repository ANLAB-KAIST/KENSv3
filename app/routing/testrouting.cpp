
#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_TimeUtil.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_NetworkUtil.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Switch.hpp>
#include <E/Networking/E_Wire.hpp>
#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>

#include <array>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "RoutingAssignment.hpp"

#define RANDOM_SEED_DEFAULT 1614233283
constexpr size_t routing_run_time = 3000;

using namespace E;

template <size_t N> class RouteTesting : public ::testing::Test {
protected:
  void setup_env() {

    int seed = RANDOM_SEED_DEFAULT;

    const char *random_seed = std::getenv("RANDOM_SEED");
    if (random_seed) {
      seed = atoi(random_seed);
    }
    srand(seed);
    RecordProperty("random_seed", seed);
    printf("[RANDOM_SEED : %d]\n", seed);
  }

  NetworkSystem netSystem;
  std::array<std::shared_ptr<Host>, N> routers;

  size_t next_mac_id;
  size_t next_network_id;

  std::vector<std::shared_ptr<Switch>> switches;

  RouteTesting() {
    next_mac_id = 1;
    next_network_id = 1;
  }

  mac_t mac_addr(size_t mac_id) {
    mac_t mac{0xBC, 0, 0, 0, 0, 0};
    size_t i = 5;
    while (mac_id > 0 && i > 0) {
      mac[i] = mac_id % 0x100;
      i -= 1;
      mac_id /= 0x100;
    }
    assert(mac_id == 0);
    return mac;
  }

  ipv4_t network_addr(size_t network_id) {
    ipv4_t network{10, 0, 0, 0};
    size_t i = 2;
    while (network_id > 0 && i > 0) {
      network[i] = network_id % 256;
      i -= 1;
      network_id /= 256;
    }
    assert(network_id == 0);
    return network;
  }

  bool intialized = false;
  void
  setupGraph(const std::vector<std::tuple<size_t, size_t, size_t>> &graph) {
    assert(!intialized);
    intialized = true;

    // Install Routers
    for (size_t i = 0; i < N; ++i) {
      routers[i] =
          netSystem.addModule<Host>("Router" + std::to_string(i), netSystem);
    }

    // Connect Routers
    for (auto &edge : graph) {

      size_t network_id = next_network_id++;
      ipv4_t network = network_addr(network_id);

      auto sw = netSystem.addModule<Switch>(
          "Switch" + std::to_string(network_id), netSystem);

      size_t i = std::get<0>(edge);
      size_t j = std::get<1>(edge);
      size_t cost = std::get<2>(edge);

      size_t bps = CostLCM / cost;

      auto ports_i = netSystem.addWire(*routers[i], *sw, 1000000, bps).second;
      auto ports_j = netSystem.addWire(*routers[j], *sw, 1000000, bps).second;

      size_t i_mac_id = next_mac_id++;
      size_t j_mac_id = next_mac_id++;

      mac_t i_mac = mac_addr(i_mac_id);
      mac_t j_mac = mac_addr(j_mac_id);

      ipv4_t i_ipv4 = network;
      i_ipv4[3] = 1;
      ipv4_t j_ipv4 = network;
      j_ipv4[3] = 2;

      int i_port = ports_i.first;
      int j_port = ports_j.first;

      routers[i]->setMACAddr(i_mac, i_port);
      routers[i]->setARPTable(j_mac, j_ipv4);
      routers[i]->setIPAddr(i_ipv4, i_port);
      routers[i]->setRoutingTable(network, 24, i_port);

      routers[j]->setMACAddr(j_mac, j_port);
      routers[j]->setARPTable(i_mac, i_ipv4);
      routers[j]->setIPAddr(j_ipv4, j_port);
      routers[j]->setRoutingTable(network, 24, j_port);

      sw->addMACEntry(ports_i.second, i_mac);
      sw->addMACEntry(ports_j.second, j_mac);

      const ::testing::TestInfo *const test_info =
          ::testing::UnitTest::GetInstance()->current_test_info();
      std::string file_name(test_info->name());
      file_name.append("-Network");
      file_name.append(std::to_string(network_id));
      file_name.append(".pcap");

      sw->enablePCAPLogging(file_name);

      switches.push_back(sw);
    }

    // Intialize Routers
    for (size_t i = 0; i < N; ++i) {
      Host &router = *routers[i];
      router.addHostModule<Ethernet>(router);
      router.addHostModule<IPv4>(router);
      router.addHostModule<RoutingAssignment>(router);
      router.initializeHostModule("UDP");
    }
  }

  void doTesting(
      const std::vector<std::tuple<size_t, size_t, size_t>> &test_vectors) {
    for (auto &vec : test_vectors) {
      size_t router_i = std::get<0>(vec);
      size_t router_j = std::get<1>(vec);
      size_t cost = std::get<2>(vec);
      for (size_t port_num = 0; port_num < routers[router_i]->getPortCount();
           ++port_num) {
        ipv4_t ip = routers[router_i]->getIPAddr(port_num).value();
        size_t cost_ret = std::any_cast<size_t>(
            routers[router_j]->diagnoseHostModule("UDP", ip));
        EXPECT_EQ(cost_ret, cost);
      }
      for (size_t port_num = 0; port_num < routers[router_j]->getPortCount();
           ++port_num) {
        ipv4_t ip = routers[router_j]->getIPAddr(port_num).value();
        size_t cost_ret = std::any_cast<size_t>(
            routers[router_i]->diagnoseHostModule("UDP", ip));
        EXPECT_EQ(cost_ret, cost);
      }
    }
  }

  void cleanUpGraph() {
    for (size_t i = 0; i < N; ++i) {
      routers[i]->cleanUp();
      routers[i]->finalizeHostModule("UDP");
    }
  }

  virtual void TearDown() {}
};

class RoutingEnvRipTwoNodes : public RouteTesting<2> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {{0, 1, 1}};
    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {{0, 1, 1}};
    doTesting(test_vector);
    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

class RoutingEnvCustomTwoNodes : public RouteTesting<2> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {{0, 1, 10}};
    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {{0, 1, 10}};
    doTesting(test_vector);
    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

class RoutingEnvRip1 : public RouteTesting<4> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {1, 2, 1}, {2, 3, 1}};
    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {1, 2, 1}, {1, 3, 2}, {2, 3, 1}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

class RoutingEnvCustom1 : public RouteTesting<4> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 1}, {0, 2, 3}, {0, 3, 7}, {1, 2, 1}, {2, 3, 2}};

    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 1}, {0, 2, 2}, {0, 3, 4}, {1, 2, 1}, {1, 3, 3}, {2, 3, 2}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

class RoutingEnvRip2 : public RouteTesting<7> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 1}, {0, 2, 1}, {0, 5, 1}, {1, 2, 1}, {1, 3, 1}, {2, 3, 1},
        {2, 4, 1}, {2, 5, 1}, {3, 4, 1}, {4, 5, 1}, {4, 6, 1}, {5, 6, 1}};

    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 1}, {0, 2, 1}, {0, 3, 2}, {0, 4, 2}, {0, 5, 1}, {0, 6, 2},
        {1, 2, 1}, {1, 3, 1}, {1, 4, 2}, {1, 5, 2}, {1, 6, 3}, {2, 3, 1},
        {2, 4, 1}, {2, 5, 1}, {2, 6, 2}, {3, 4, 1}, {3, 5, 2}, {3, 6, 2},
        {4, 5, 1}, {4, 6, 1}, {5, 6, 1}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};
class RoutingEnvCustom2 : public RouteTesting<7> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 2}, {0, 2, 4}, {0, 5, 7}, {1, 2, 3}, {1, 3, 3}, {2, 3, 4},
        {2, 4, 3}, {2, 5, 8}, {3, 4, 6}, {4, 5, 6}, {4, 6, 8}, {5, 6, 12}};

    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 2}, {0, 2, 4}, {0, 3, 5},  {0, 4, 7}, {0, 5, 7},  {0, 6, 15},
        {1, 2, 3}, {1, 3, 3}, {1, 4, 6},  {1, 5, 9}, {1, 6, 14}, {2, 3, 4},
        {2, 4, 3}, {2, 5, 8}, {2, 6, 11}, {3, 4, 6}, {3, 5, 12}, {3, 6, 14},
        {4, 5, 6}, {4, 6, 8}, {5, 6, 12}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

class RoutingEnvRip3 : public RouteTesting<5> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 1}, {0, 3, 1}, {1, 2, 1}, {1, 4, 1}, {2, 3, 1}, {2, 4, 1}};
    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 1}, {0, 2, 2}, {0, 3, 1}, {0, 4, 2}, {1, 2, 1},
        {1, 3, 2}, {1, 4, 1}, {2, 3, 1}, {2, 4, 1}, {3, 4, 2}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};
class RoutingEnvCustom3 : public RouteTesting<5> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 1}, {0, 3, 2}, {1, 2, 3}, {1, 4, 6}, {2, 3, 3}, {2, 4, 2}};
    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 1}, {0, 2, 4}, {0, 3, 2}, {0, 4, 6}, {1, 2, 3},
        {1, 3, 3}, {1, 4, 5}, {2, 3, 3}, {2, 4, 2}, {3, 4, 5}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

class RoutingEnvRip4 : public RouteTesting<9> {
protected:
  virtual void SetUp() {
    setup_env();

    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 1}, {0, 3, 1}, {1, 2, 1}, {1, 4, 1}, {3, 4, 1}, {3, 6, 1},
        {4, 5, 1}, {4, 7, 1}, {5, 8, 1}, {6, 7, 1}, {7, 8, 1}};

    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 1}, {0, 2, 2}, {0, 3, 1}, {0, 4, 2}, {0, 5, 3}, {0, 6, 2},
        {0, 7, 3}, {0, 8, 4}, {1, 2, 1}, {1, 3, 2}, {1, 4, 1}, {1, 5, 2},
        {1, 6, 3}, {1, 7, 2}, {1, 8, 3}, {2, 3, 3}, {2, 4, 2}, {2, 5, 3},
        {2, 6, 4}, {2, 7, 3}, {2, 8, 4}, {3, 4, 1}, {3, 5, 2}, {3, 6, 1},
        {3, 7, 2}, {3, 8, 3}, {4, 5, 1}, {4, 6, 2}, {4, 7, 1}, {4, 8, 2},
        {5, 6, 3}, {5, 7, 2}, {5, 8, 1}, {6, 7, 1}, {6, 8, 2}, {7, 8, 1}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};
class RoutingEnvCustom4 : public RouteTesting<9> {
protected:
  virtual void SetUp() {
    setup_env();
    std::vector<std::tuple<size_t, size_t, size_t>> graph = {
        {0, 1, 8}, {0, 3, 1}, {1, 2, 1}, {1, 4, 1}, {3, 4, 1}, {3, 6, 1},
        {4, 5, 1}, {4, 7, 1}, {5, 8, 1}, {6, 7, 1}, {7, 8, 1}};
    setupGraph(graph);
  }
  void runTest() {
    netSystem.run(TimeUtil::makeTime(routing_run_time, TimeUtil::SEC));

    std::vector<std::tuple<size_t, size_t, size_t>> test_vector = {
        {0, 1, 3}, {0, 2, 4}, {0, 3, 1}, {0, 4, 2}, {0, 5, 3}, {0, 6, 2},
        {0, 7, 3}, {0, 8, 4}, {1, 2, 1}, {1, 3, 2}, {1, 4, 1}, {1, 5, 2},
        {1, 6, 3}, {1, 7, 2}, {1, 8, 3}, {2, 3, 3}, {2, 4, 2}, {2, 5, 3},
        {2, 6, 4}, {2, 7, 3}, {2, 8, 4}, {3, 4, 1}, {3, 5, 2}, {3, 6, 1},
        {3, 7, 2}, {3, 8, 3}, {4, 5, 1}, {4, 6, 2}, {4, 7, 1}, {4, 8, 2},
        {5, 6, 3}, {5, 7, 2}, {5, 8, 1}, {6, 7, 1}, {6, 8, 2}, {7, 8, 1}};
    doTesting(test_vector);

    cleanUpGraph();
    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
};

TEST_F(RoutingEnvRipTwoNodes, TestRoutingRipTwoNodes) { this->runTest(); }
TEST_F(RoutingEnvCustomTwoNodes, TestRoutingCustomTwoNodes) { this->runTest(); }
TEST_F(RoutingEnvRip1, TestRoutingRip1) { this->runTest(); }
TEST_F(RoutingEnvCustom1, TestRoutingCustom1) { this->runTest(); }
TEST_F(RoutingEnvRip2, TestRoutingRip2) { this->runTest(); }
TEST_F(RoutingEnvCustom2, TestRoutingCustom2) { this->runTest(); }
TEST_F(RoutingEnvRip3, TestRoutingRip3) { this->runTest(); }
TEST_F(RoutingEnvCustom3, TestRoutingCustom3) { this->runTest(); }
TEST_F(RoutingEnvRip4, TestRoutingRip4) { this->runTest(); }
TEST_F(RoutingEnvCustom4, TestRoutingCustom4) { this->runTest(); }
