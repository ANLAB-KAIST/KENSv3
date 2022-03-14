
#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_TimeUtil.hpp>

#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Switch.hpp>

#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>
#include <E/Networking/TCP/E_TCPApplication.hpp>
#include <E/Networking/TCP/E_TCPSolution.hpp>

#include "EchoAssignment.hpp"

#include <gtest/gtest.h>

using namespace E;

struct EchoHost {
  std::shared_ptr<Host> host;

  struct EchoApp {
    int pid;
    std::map<std::string, std::string> answers;
    std::vector<char *> args;
    EchoApp(std::initializer_list<const char *> arg_list) {
      for (const char *arg : arg_list) {
        char *arg_c = strdup(arg);
        args.push_back(arg_c);
      }
    }
    ~EchoApp() {
      for (size_t i = 0; i < args.size(); ++i) {
        free(args[i]);
      }
    }
    EchoApp(EchoApp &&other)
        : pid(other.pid), answers(std::move(other.answers)) {
      for (size_t i = 0; i < other.args.size(); ++i) {
        args.push_back(other.args[i]);
      }
      other.args.resize(0);
    }

    EchoApp &operator=(EchoApp &&other) {
      pid = other.pid;
      answers = std::move(other.answers);
      for (size_t i = 0; i < other.args.size(); ++i) {
        args.push_back(other.args[i]);
      }
      other.args.resize(0);
      return *this;
    }
    EchoApp(const EchoApp &other) = delete;
    EchoApp &operator=(const EchoApp &other) = delete;
  };

  std::vector<EchoApp> apps;
};

class EchoTesting : public ::testing::Test {
protected:
  NetworkSystem netSystem;
  std::vector<EchoHost> hosts;
  std::shared_ptr<Switch> switchingHub;

  std::vector<const char *> servers;
  std::vector<std::tuple<uint8_t, uint8_t, const char *, size_t>> connections;

  void SetUp(std::initializer_list<const char *> __servers,
             const uint8_t num_clients,
             std::initializer_list<std::tuple<uint8_t, uint8_t, const char *>>
                 __connections) {
    servers = __servers;

    assert(servers.size() < 50);
    uint8_t num_servers = servers.size();
    assert(num_clients < 150);

    const uint8_t num_hosts = num_clients + num_servers;

    switchingHub = netSystem.addModule<Switch>("Switch0", netSystem);
    for (uint8_t host_id = 1; host_id <= num_hosts; ++host_id) {
      std::string host_name = "TestHost_" + std::to_string(host_id);

      EchoHost host;
      host.host = netSystem.addModule<Host>(host_name, netSystem);

      mac_t mac{0xBC, 0xBC, 0xBC, 0xBC, 0xBC, host_id};
      ipv4_t ip{10, 0, 0, host_id};
      auto wire = netSystem
                      .addWire(*host.host, *switchingHub,
                               TimeUtil::makeTime(1, TimeUtil::MSEC))
                      .second;

      host.host->setMACAddr(mac, wire.first);

      for (uint8_t peer_id = 1; peer_id <= num_hosts; ++peer_id) {
        if (peer_id == host_id)
          continue;

        mac_t peer_mac{0xBC, 0xBC, 0xBC, 0xBC, 0xBC, peer_id};
        ipv4_t peer_ip{10, 0, 0, peer_id};
        host.host->setARPTable(peer_mac, peer_ip);
      }
      host.host->setIPAddr(ip, wire.first);
      host.host->setRoutingTable({10, 0, 0, 0}, 16, wire.first);

      switchingHub->addMACEntry(wire.second, mac);
      host.host->addHostModule<Ethernet>(*host.host);
      host.host->addHostModule<IPv4>(*host.host);

      TCPSolutionProvider::allocate(*host.host);

      hosts.push_back(std::move(host));
    }
    const ::testing::TestInfo *const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();
    std::string file_name(test_info->name());
    file_name.append(".pcap");
    switchingHub->enablePCAPLogging(file_name);

    for (auto &host : hosts) {
      host.host->initializeHostModule("TCP");
    }

    for (uint8_t server_id = 0; server_id < num_servers; ++server_id) {
      auto args = {"echo", "s", "0.0.0.0", "9000", servers[server_id]};
      hosts[server_id].apps.emplace_back(args);
    }

    for (auto &connection : __connections) {
      uint8_t server_id = std::get<0>(connection);
      uint8_t client_id = std::get<1>(connection);
      const char *command = std::get<2>(connection);
      assert(server_id < num_servers);
      assert(client_id < num_clients);

      auto &client = hosts[num_servers + client_id];

      std::string server_ip = "10.0.0." + std::to_string(server_id + 1);
      auto args = {"echo", "c", server_ip.c_str(), "9000", command};
      connections.emplace_back(server_id, client_id, command,
                               client.apps.size());
      client.apps.emplace_back(args);
    }
  }
  void runTest() {

    // launch apps
    for (auto &host : hosts) {
      for (auto &app : host.apps) {
        app.pid = host.host->addApplication<EchoAssignment>(
            *host.host, app.answers, app.args.size(), app.args.data());
        host.host->launchApplication(app.pid);
      }
    }

    netSystem.run(TimeUtil::makeTime(1000, TimeUtil::SEC));
    for (auto &host : hosts) {
      host.host->cleanUp();
      host.host->finalizeHostModule("TCP");
    }

    netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
  }
  void checkTest(bool skip_server = false) {
    for (auto &connection : connections) {
      uint8_t server_id = std::get<0>(connection);
      uint8_t client_id = std::get<1>(connection);
      std::string client_command = std::get<2>(connection);
      size_t app_id = std::get<3>(connection);
      std::string server_hello = servers[server_id];

      std::string server_ip = "10.0.0." + std::to_string(server_id + 1);
      std::string client_ip =
          "10.0.0." + std::to_string(servers.size() + client_id + 1);
      auto &server = hosts[server_id];
      auto &client = hosts[servers.size() + client_id];
      std::string server_ans = server.apps[0].answers[client_ip];
      std::string expected_server_ans = client_command;
      if (!skip_server)
        EXPECT_EQ(server_ans, expected_server_ans);

      auto &client_app = client.apps[app_id];
      std::string client_ans = client_app.answers[server_ip];
      std::string expected_client_ans;
      if (client_command == "hello") {
        expected_client_ans = server_hello;
      } else if (client_command == "whoami") {
        expected_client_ans = client_ip;
      } else if (client_command == "whoru") {
        expected_client_ans = server_ip;
      } else {
        expected_client_ans = client_command;
      }
      EXPECT_EQ(client_ans, expected_client_ans);
    }
  }
};

TEST_F(EchoTesting, SingleEcho) {
  SetUp({"server-hello"}, 1, {{0, 0, "echo-test"}});
  runTest();
  checkTest();
}
TEST_F(EchoTesting, SingleWhoRU) {
  SetUp({"server-hello1"}, 1, {{0, 0, "whoru"}});
  runTest();
  checkTest();
}
TEST_F(EchoTesting, SingleWhoAmI) {
  SetUp({"server-hello2"}, 1, {{0, 0, "whoami"}});
  runTest();
  checkTest();
}
TEST_F(EchoTesting, SingleHello) {
  SetUp({"server-hello3"}, 1, {{0, 0, "hello"}});
  runTest();
  checkTest();
}
TEST_F(EchoTesting, OnetoManyEcho) {
  SetUp({"server-hello4"}, 33,
        {
            {0, 0, "echo-hello0"},   {0, 1, "echo-hello1"},
            {0, 2, "echo-hello2"},   {0, 3, "echo-hello3"},
            {0, 4, "echo-hello4"},   {0, 5, "echo-hello5"},
            {0, 6, "echo-hello6"},   {0, 7, "echo-hello7"},
            {0, 8, "echo-hello8"},   {0, 9, "echo-hello9"},
            {0, 10, "echo-hello10"}, {0, 11, "echo-hello11"},
            {0, 12, "echo-hello12"}, {0, 13, "echo-hello13"},
            {0, 14, "echo-hello14"}, {0, 15, "echo-hello15"},
            {0, 16, "echo-hello16"}, {0, 17, "echo-hello17"},
            {0, 18, "echo-hello18"}, {0, 19, "echo-hello19"},
            {0, 20, "echo-hello20"}, {0, 21, "echo-hello21"},
            {0, 22, "echo-hello22"}, {0, 23, "echo-hello23"},
            {0, 24, "echo-hello24"}, {0, 25, "echo-hello25"},
            {0, 26, "echo-hello26"}, {0, 27, "echo-hello27"},
            {0, 28, "echo-hello28"}, {0, 29, "echo-hello29"},
            {0, 30, "echo-hello30"}, {0, 31, "echo-hello31"},
            {0, 32, "echo-hello32"},
        });
  runTest();
  checkTest();
}
TEST_F(EchoTesting, OnetoManyWhoRU) {
  SetUp({"server-hello5"}, 33,
        {
            {0, 0, "whoru"},  {0, 1, "whoru"},  {0, 2, "whoru"},
            {0, 3, "whoru"},  {0, 4, "whoru"},  {0, 5, "whoru"},
            {0, 6, "whoru"},  {0, 7, "whoru"},  {0, 8, "whoru"},
            {0, 9, "whoru"},  {0, 10, "whoru"}, {0, 11, "whoru"},
            {0, 12, "whoru"}, {0, 13, "whoru"}, {0, 14, "whoru"},
            {0, 15, "whoru"}, {0, 16, "whoru"}, {0, 17, "whoru"},
            {0, 18, "whoru"}, {0, 19, "whoru"}, {0, 20, "whoru"},
            {0, 21, "whoru"}, {0, 22, "whoru"}, {0, 23, "whoru"},
            {0, 24, "whoru"}, {0, 25, "whoru"}, {0, 26, "whoru"},
            {0, 27, "whoru"}, {0, 28, "whoru"}, {0, 29, "whoru"},
            {0, 30, "whoru"}, {0, 31, "whoru"}, {0, 32, "whoru"},
        });
  runTest();
  checkTest();
}

TEST_F(EchoTesting, OnetoManyWhoAmI) {
  SetUp({"server-hello6"}, 33,
        {
            {0, 0, "whoami"},  {0, 1, "whoami"},  {0, 2, "whoami"},
            {0, 3, "whoami"},  {0, 4, "whoami"},  {0, 5, "whoami"},
            {0, 6, "whoami"},  {0, 7, "whoami"},  {0, 8, "whoami"},
            {0, 9, "whoami"},  {0, 10, "whoami"}, {0, 11, "whoami"},
            {0, 12, "whoami"}, {0, 13, "whoami"}, {0, 14, "whoami"},
            {0, 15, "whoami"}, {0, 16, "whoami"}, {0, 17, "whoami"},
            {0, 18, "whoami"}, {0, 19, "whoami"}, {0, 20, "whoami"},
            {0, 21, "whoami"}, {0, 22, "whoami"}, {0, 23, "whoami"},
            {0, 24, "whoami"}, {0, 25, "whoami"}, {0, 26, "whoami"},
            {0, 27, "whoami"}, {0, 28, "whoami"}, {0, 29, "whoami"},
            {0, 30, "whoami"}, {0, 31, "whoami"}, {0, 32, "whoami"},
        });
  runTest();
  checkTest();
}

TEST_F(EchoTesting, OnetoManyHello) {
  SetUp({"server-hello7"}, 33,
        {
            {0, 0, "hello"},  {0, 1, "hello"},  {0, 2, "hello"},
            {0, 3, "hello"},  {0, 4, "hello"},  {0, 5, "hello"},
            {0, 6, "hello"},  {0, 7, "hello"},  {0, 8, "hello"},
            {0, 9, "hello"},  {0, 10, "hello"}, {0, 11, "hello"},
            {0, 12, "hello"}, {0, 13, "hello"}, {0, 14, "hello"},
            {0, 15, "hello"}, {0, 16, "hello"}, {0, 17, "hello"},
            {0, 18, "hello"}, {0, 19, "hello"}, {0, 20, "hello"},
            {0, 21, "hello"}, {0, 22, "hello"}, {0, 23, "hello"},
            {0, 24, "hello"}, {0, 25, "hello"}, {0, 26, "hello"},
            {0, 27, "hello"}, {0, 28, "hello"}, {0, 29, "hello"},
            {0, 30, "hello"}, {0, 31, "hello"}, {0, 32, "hello"},
        });
  runTest();
  checkTest();
}

TEST_F(EchoTesting, OnetoManyAll) {
  SetUp({"server-hello8"}, 33,
        {
            {0, 0, "echo-hello0"},   {0, 1, "echo-hello1"},
            {0, 2, "echo-hello2"},   {0, 3, "echo-hello3"},
            {0, 4, "echo-hello4"},   {0, 5, "echo-hello5"},
            {0, 6, "echo-hello6"},   {0, 7, "echo-hello7"},
            {0, 8, "echo-hello8"},   {0, 9, "echo-hello9"},
            {0, 10, "echo-hello10"}, {0, 11, "echo-hello11"},
            {0, 12, "echo-hello12"}, {0, 13, "echo-hello13"},
            {0, 14, "echo-hello14"}, {0, 15, "echo-hello15"},
            {0, 16, "echo-hello16"}, {0, 17, "echo-hello17"},
            {0, 18, "echo-hello18"}, {0, 19, "echo-hello19"},
            {0, 20, "echo-hello20"}, {0, 21, "echo-hello21"},
            {0, 22, "echo-hello22"}, {0, 23, "echo-hello23"},
            {0, 24, "echo-hello24"}, {0, 25, "echo-hello25"},
            {0, 26, "echo-hello26"}, {0, 27, "echo-hello27"},
            {0, 28, "echo-hello28"}, {0, 29, "echo-hello29"},
            {0, 30, "echo-hello30"}, {0, 31, "echo-hello31"},
            {0, 32, "echo-hello32"}, {0, 0, "whoru"},
            {0, 1, "whoru"},         {0, 2, "whoru"},
            {0, 3, "whoru"},         {0, 4, "whoru"},
            {0, 5, "whoru"},         {0, 6, "whoru"},
            {0, 7, "whoru"},         {0, 8, "whoru"},
            {0, 9, "whoru"},         {0, 10, "whoru"},
            {0, 11, "whoru"},        {0, 12, "whoru"},
            {0, 13, "whoru"},        {0, 14, "whoru"},
            {0, 15, "whoru"},        {0, 16, "whoru"},
            {0, 17, "whoru"},        {0, 18, "whoru"},
            {0, 19, "whoru"},        {0, 20, "whoru"},
            {0, 21, "whoru"},        {0, 22, "whoru"},
            {0, 23, "whoru"},        {0, 24, "whoru"},
            {0, 25, "whoru"},        {0, 26, "whoru"},
            {0, 27, "whoru"},        {0, 28, "whoru"},
            {0, 29, "whoru"},        {0, 30, "whoru"},
            {0, 31, "whoru"},        {0, 32, "whoru"},
            {0, 0, "whoami"},        {0, 1, "whoami"},
            {0, 2, "whoami"},        {0, 3, "whoami"},
            {0, 4, "whoami"},        {0, 5, "whoami"},
            {0, 6, "whoami"},        {0, 7, "whoami"},
            {0, 8, "whoami"},        {0, 9, "whoami"},
            {0, 10, "whoami"},       {0, 11, "whoami"},
            {0, 12, "whoami"},       {0, 13, "whoami"},
            {0, 14, "whoami"},       {0, 15, "whoami"},
            {0, 16, "whoami"},       {0, 17, "whoami"},
            {0, 18, "whoami"},       {0, 19, "whoami"},
            {0, 20, "whoami"},       {0, 21, "whoami"},
            {0, 22, "whoami"},       {0, 23, "whoami"},
            {0, 24, "whoami"},       {0, 25, "whoami"},
            {0, 26, "whoami"},       {0, 27, "whoami"},
            {0, 28, "whoami"},       {0, 29, "whoami"},
            {0, 30, "whoami"},       {0, 31, "whoami"},
            {0, 32, "whoami"},       {0, 0, "hello"},
            {0, 1, "hello"},         {0, 2, "hello"},
            {0, 3, "hello"},         {0, 4, "hello"},
            {0, 5, "hello"},         {0, 6, "hello"},
            {0, 7, "hello"},         {0, 8, "hello"},
            {0, 9, "hello"},         {0, 10, "hello"},
            {0, 11, "hello"},        {0, 12, "hello"},
            {0, 13, "hello"},        {0, 14, "hello"},
            {0, 15, "hello"},        {0, 16, "hello"},
            {0, 17, "hello"},        {0, 18, "hello"},
            {0, 19, "hello"},        {0, 20, "hello"},
            {0, 21, "hello"},        {0, 22, "hello"},
            {0, 23, "hello"},        {0, 24, "hello"},
            {0, 25, "hello"},        {0, 26, "hello"},
            {0, 27, "hello"},        {0, 28, "hello"},
            {0, 29, "hello"},        {0, 30, "hello"},
            {0, 31, "hello"},        {0, 32, "hello"},
        });
  runTest();
  checkTest(true);
}

TEST_F(EchoTesting, ManytoOneEcho) {
  SetUp(
      {
          "server0",
          "server1",
          "server2",
          "server3",
          "server4",
          "server5",
          "server6",
          "server7",
          "server8",
          "server9",
      },
      1,
      {
          {0, 0, "echo-hello0"},
          {1, 0, "echo-hello1"},
          {2, 0, "echo-hello2"},
          {3, 0, "echo-hello3"},
          {4, 0, "echo-hello4"},
          {5, 0, "echo-hello5"},
          {6, 0, "echo-hello6"},
          {7, 0, "echo-hello7"},
          {8, 0, "echo-hello8"},
          {9, 0, "echo-hello9"},
      });
  runTest();
  checkTest();
}

TEST_F(EchoTesting, ManytoOneWhoRU) {
  SetUp(
      {
          "server0",
          "server1",
          "server2",
          "server3",
          "server4",
          "server5",
          "server6",
          "server7",
          "server8",
          "server9",
      },
      1,
      {
          {0, 0, "whoru"},
          {1, 0, "whoru"},
          {2, 0, "whoru"},
          {3, 0, "whoru"},
          {4, 0, "whoru"},
          {5, 0, "whoru"},
          {6, 0, "whoru"},
          {7, 0, "whoru"},
          {8, 0, "whoru"},
          {9, 0, "whoru"},
      });
  runTest();
  checkTest();
}
TEST_F(EchoTesting, ManytoOneWhoAmI) {
  SetUp(
      {
          "server0",
          "server1",
          "server2",
          "server3",
          "server4",
          "server5",
          "server6",
          "server7",
          "server8",
          "server9",
      },
      1,
      {
          {0, 0, "whoami"},
          {1, 0, "whoami"},
          {2, 0, "whoami"},
          {3, 0, "whoami"},
          {4, 0, "whoami"},
          {5, 0, "whoami"},
          {6, 0, "whoami"},
          {7, 0, "whoami"},
          {8, 0, "whoami"},
          {9, 0, "whoami"},
      });
  runTest();
  checkTest();
}
TEST_F(EchoTesting, ManytoOneHello) {
  SetUp(
      {
          "server0",
          "server1",
          "server2",
          "server3",
          "server4",
          "server5",
          "server6",
          "server7",
          "server8",
          "server9",
      },
      1,
      {
          {0, 0, "hello"},
          {1, 0, "hello"},
          {2, 0, "hello"},
          {3, 0, "hello"},
          {4, 0, "hello"},
          {5, 0, "hello"},
          {6, 0, "hello"},
          {7, 0, "hello"},
          {8, 0, "hello"},
          {9, 0, "hello"},
      });
  runTest();
  checkTest();
}
TEST_F(EchoTesting, ManytoOneAll) {
  SetUp(
      {
          "server0",
          "server1",
          "server2",
          "server3",
          "server4",
          "server5",
          "server6",
          "server7",
          "server8",
          "server9",
      },
      1,
      {
          {0, 0, "echo-hello0"}, {1, 0, "echo-hello1"}, {2, 0, "echo-hello2"},
          {3, 0, "echo-hello3"}, {4, 0, "echo-hello4"}, {5, 0, "echo-hello5"},
          {6, 0, "echo-hello6"}, {7, 0, "echo-hello7"}, {8, 0, "echo-hello8"},
          {9, 0, "echo-hello9"}, {0, 0, "whoru"},       {1, 0, "whoru"},
          {2, 0, "whoru"},       {3, 0, "whoru"},       {4, 0, "whoru"},
          {5, 0, "whoru"},       {6, 0, "whoru"},       {7, 0, "whoru"},
          {8, 0, "whoru"},       {9, 0, "whoru"},       {0, 0, "whoami"},
          {1, 0, "whoami"},      {2, 0, "whoami"},      {3, 0, "whoami"},
          {4, 0, "whoami"},      {5, 0, "whoami"},      {6, 0, "whoami"},
          {7, 0, "whoami"},      {8, 0, "whoami"},      {9, 0, "whoami"},
          {0, 0, "hello"},       {1, 0, "hello"},       {2, 0, "hello"},
          {3, 0, "hello"},       {4, 0, "hello"},       {5, 0, "hello"},
          {6, 0, "hello"},       {7, 0, "hello"},       {8, 0, "hello"},
          {9, 0, "hello"},
      });
  runTest();
  checkTest(true);
}
TEST_F(EchoTesting, ManytoMany) {
  SetUp(
      {
          "server0",
          "server1",
          "server2",
          "server3",
          "server4",
          "server5",
          "server6",
          "server7",
          "server8",
          "server9",
      },
      33,
      {
          {7, 32, "whoru"},         {9, 16, "whoru"},
          {6, 26, "echo-hello3"},   {5, 13, "whoami"},
          {0, 30, "echo-hello5"},   {6, 31, "echo-hello6"},
          {5, 1, "whoru"},          {4, 20, "echo-hello8"},
          {3, 30, "whoru"},         {1, 4, "whoami"},
          {2, 1, "whoru"},          {0, 13, "echo-hello12"},
          {8, 30, "whoami"},        {2, 2, "echo-hello14"},
          {3, 18, "whoru"},         {2, 20, "hello"},
          {9, 15, "hello"},         {5, 32, "whoru"},
          {6, 14, "hello"},         {8, 6, "hello"},
          {8, 4, "whoami"},         {7, 24, "hello"},
          {7, 7, "hello"},          {6, 28, "whoru"},
          {2, 5, "hello"},          {8, 31, "echo-hello26"},
          {2, 7, "whoru"},          {4, 28, "hello"},
          {7, 7, "whoami"},         {1, 1, "echo-hello30"},
          {9, 10, "hello"},         {7, 2, "whoami"},
          {4, 23, "whoru"},         {6, 29, "whoami"},
          {2, 1, "hello"},          {3, 29, "whoru"},
          {8, 32, "echo-hello37"},  {7, 5, "whoru"},
          {7, 11, "hello"},         {2, 24, "hello"},
          {1, 13, "echo-hello41"},  {6, 2, "whoru"},
          {1, 24, "whoami"},        {1, 11, "whoami"},
          {2, 24, "whoru"},         {7, 32, "echo-hello46"},
          {2, 8, "hello"},          {0, 15, "hello"},
          {9, 30, "hello"},         {8, 9, "whoami"},
          {0, 6, "whoru"},          {3, 25, "whoru"},
          {3, 29, "whoami"},        {1, 17, "hello"},
          {5, 4, "whoru"},          {3, 25, "whoru"},
          {9, 1, "whoami"},         {6, 17, "whoru"},
          {7, 25, "echo-hello59"},  {7, 0, "hello"},
          {6, 13, "whoru"},         {5, 19, "whoami"},
          {2, 12, "whoami"},        {2, 9, "hello"},
          {2, 17, "hello"},         {6, 26, "whoru"},
          {0, 21, "whoami"},        {9, 28, "hello"},
          {6, 20, "whoru"},         {6, 2, "echo-hello70"},
          {0, 23, "whoru"},         {2, 10, "whoru"},
          {4, 5, "echo-hello73"},   {2, 2, "whoru"},
          {9, 31, "whoru"},         {4, 32, "whoru"},
          {3, 26, "hello"},         {4, 21, "whoami"},
          {0, 4, "echo-hello79"},   {2, 10, "whoru"},
          {1, 29, "whoami"},        {4, 11, "whoami"},
          {4, 32, "whoru"},         {0, 26, "hello"},
          {6, 2, "whoru"},          {6, 14, "whoami"},
          {3, 3, "whoru"},          {3, 29, "echo-hello88"},
          {3, 32, "whoru"},         {0, 21, "whoru"},
          {8, 27, "echo-hello91"},  {7, 13, "whoami"},
          {5, 17, "echo-hello93"},  {4, 22, "hello"},
          {6, 14, "echo-hello95"},  {8, 14, "whoami"},
          {7, 24, "hello"},         {1, 9, "whoru"},
          {5, 4, "hello"},          {1, 23, "hello"},
          {1, 16, "hello"},         {1, 8, "whoru"},
          {6, 28, "hello"},         {7, 0, "echo-hello104"},
          {9, 30, "whoru"},         {4, 8, "echo-hello106"},
          {7, 11, "echo-hello107"}, {6, 24, "echo-hello108"},
          {3, 6, "echo-hello109"},  {4, 31, "whoami"},
          {8, 21, "hello"},         {0, 8, "echo-hello112"},
          {8, 6, "hello"},          {6, 1, "whoru"},
          {9, 21, "hello"},         {5, 18, "hello"},
          {5, 19, "whoru"},         {2, 15, "whoami"},
          {5, 17, "whoami"},        {9, 20, "hello"},
          {9, 28, "whoru"},         {6, 28, "whoami"},
          {5, 13, "whoru"},         {5, 23, "whoami"},
          {7, 11, "echo-hello125"}, {4, 21, "whoami"},
          {1, 19, "hello"},         {0, 9, "whoami"},
          {8, 32, "echo-hello129"}, {3, 14, "echo-hello130"},
          {5, 0, "whoru"},          {7, 7, "echo-hello132"},
          {9, 27, "whoami"},        {0, 24, "echo-hello134"},
          {3, 22, "hello"},         {5, 29, "whoami"},
          {2, 5, "whoami"},         {1, 25, "hello"},
          {5, 32, "hello"},         {5, 18, "whoru"},
          {8, 27, "hello"},         {4, 24, "hello"},
          {0, 9, "whoami"},         {1, 11, "hello"},
          {4, 15, "hello"},         {8, 16, "hello"},
          {9, 15, "whoami"},        {6, 25, "echo-hello148"},
          {4, 25, "whoru"},         {2, 4, "whoami"},
          {4, 13, "hello"},         {7, 17, "hello"},
          {9, 31, "whoami"},        {2, 26, "hello"},
          {7, 32, "whoami"},        {7, 32, "whoru"},
          {1, 27, "whoami"},        {2, 12, "whoami"},
          {1, 4, "hello"},          {0, 8, "whoru"},
          {4, 15, "whoami"},        {4, 19, "hello"},
          {1, 12, "whoru"},         {2, 23, "whoami"},
          {4, 22, "hello"},         {5, 22, "whoru"},
          {4, 29, "whoami"},        {9, 18, "echo-hello168"},
          {8, 21, "echo-hello169"}, {8, 5, "whoru"},
          {8, 17, "hello"},         {6, 8, "whoami"},
          {8, 20, "echo-hello173"}, {0, 5, "hello"},
          {1, 5, "whoru"},          {6, 7, "whoami"},
          {2, 26, "echo-hello177"}, {9, 28, "whoami"},
          {6, 3, "echo-hello179"},  {0, 26, "whoami"},
          {4, 6, "echo-hello181"},  {9, 20, "hello"},
          {7, 16, "whoru"},         {4, 19, "hello"},
          {9, 7, "echo-hello185"},  {2, 14, "hello"},
          {0, 8, "whoru"},          {9, 28, "echo-hello188"},
          {4, 11, "echo-hello189"}, {3, 32, "echo-hello190"},
          {4, 24, "hello"},         {3, 1, "whoru"},
          {4, 23, "whoami"},        {4, 3, "echo-hello194"},
          {0, 2, "hello"},          {4, 3, "echo-hello196"},
          {2, 17, "hello"},         {0, 8, "echo-hello198"},
          {1, 21, "echo-hello199"}, {9, 7, "echo-hello200"},
          {5, 12, "whoru"},         {6, 15, "whoami"},
          {3, 18, "echo-hello203"}, {8, 29, "whoru"},
          {3, 25, "whoru"},         {3, 25, "whoami"},
          {2, 3, "whoami"},         {2, 11, "whoru"},
          {8, 13, "hello"},         {9, 21, "whoru"},
          {2, 4, "whoru"},          {1, 23, "hello"},
          {6, 29, "hello"},         {4, 11, "whoami"},
          {1, 5, "hello"},          {5, 6, "whoami"},
      });
  runTest();
  checkTest(true);
}