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
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Port.hpp>
#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_Switch.hpp>
#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>
#include <E/Networking/TCP/E_TCPApplication.hpp>
#include <E/Networking/TCP/E_TCPSolution.hpp>
#include <E/E_TimeUtil.hpp>

#include <gtest/gtest.h>
#include "TCPAssignment.hpp"

using namespace E;

template <class Target>
class TestEnv1 : public ::testing::Test
{
protected:
	NetworkSystem netSystem;
	Host* host1;
	Host* host2;
	Switch* switchingHub;
	Ethernet* ethernet1;
	Ethernet* ethernet2;
	IPv4* ipv4_1;
	IPv4* ipv4_2;
	HostModule* interface;
	HostModule* interface2;

	virtual void SetUp()
	{
		host1 = new Host("TestHost1", 2, &netSystem);
		host2 = new Host("TestHost2", 2, &netSystem);

		uint8_t mac1[6] = {0xBC,0xBC,0xBC,0xBC,0xBC,0xBC};
		uint8_t mac1_2[6] = {0xCB,0xCB,0xCB,0xCB,0xCB,0xCB};
		uint8_t mac2[6] = {0xCD,0xCD,0xCD,0xCD,0xCD,0xCD};
		uint8_t mac2_2[6] = {0xDC,0xDC,0xDC,0xDC,0xDC,0xDC};
		uint8_t ip1[4] = {192,168,0,7};
		uint8_t ip1_2[4] = {192,168,0,8};
		uint8_t ip2[4] = {10,0,1,4};
		uint8_t ip2_2[4] = {10,0,1,5};
		host1->setMACAddr(mac1, 0);
		host1->setMACAddr(mac1_2, 1);
		host1->setARPTable(mac2, ip2);
		host1->setARPTable(mac2_2, ip2_2);
		host1->setIPAddr(ip1, 0);
		host1->setIPAddr(ip1_2, 1);
		host1->setRoutingTable(ip2, 16, 0);
		host1->setRoutingTable(ip2_2, 16, 1);

		host2->setMACAddr(mac2, 0);
		host2->setMACAddr(mac2_2, 1);
		host2->setARPTable(mac1, ip1);
		host2->setARPTable(mac1_2, ip1_2);
		host2->setIPAddr(ip2, 0);
		host2->setIPAddr(ip2_2, 1);
		host2->setRoutingTable(ip1, 16, 0);
		host2->setRoutingTable(ip1_2, 16, 1);

		host1->getPort(0)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));
		host1->getPort(1)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));
		host2->getPort(0)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));
		host2->getPort(1)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));

		switchingHub = new Switch("Switch1", &netSystem);
		switchingHub->addPort(host1->getPort(0));
		switchingHub->addPort(host1->getPort(1));
		switchingHub->addPort(host2->getPort(0));
		switchingHub->addPort(host2->getPort(1));

		switchingHub->addMACEntry(host1->getPort(0), mac1);
		switchingHub->addMACEntry(host1->getPort(1), mac1_2);
		switchingHub->addMACEntry(host2->getPort(0), mac2);
		switchingHub->addMACEntry(host2->getPort(1), mac2_2);


		ethernet1 = new Ethernet(host1);
		ethernet2 = new Ethernet(host2);
		ipv4_1 = new IPv4(host1);
		ipv4_2 = new IPv4(host2);

		interface = Target::allocate(host1);
		interface2 = TCPSolutionProvider::allocate(host2, false, false, false);
		interface->initialize();
		interface2->initialize();
	}
	virtual void TearDown()
	{
		delete interface;
		delete interface2;


		delete ipv4_1;
		delete ipv4_2;

		delete ethernet1;
		delete ethernet2;


		delete switchingHub;

		delete host1;
		delete host2;
	}

	void runTest()
	{
		netSystem.run(TimeUtil::makeTime(1000, TimeUtil::SEC));

		host1->cleanUp();
		host2->cleanUp();
		interface2->finalize();
		interface->finalize();
		netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
	}
};

template <class Target, class Adversary, bool Unreliable>
class TestEnv2 : public ::testing::Test
{
protected:
	NetworkSystem netSystem;
	Host* host1;
	Host* host2;
	Switch* switchingHub;
	Ethernet* ethernet1;
	Ethernet* ethernet2;
	IPv4* ipv4_1;
	IPv4* ipv4_2;
	HostModule* interface;
	HostModule* interface2;

	virtual void SetUp()
	{
		host1 = new Host("TestHost1", 2, &netSystem);
		host2 = new Host("TestHost2", 2, &netSystem);

		uint8_t mac1[6] = {0xBC,0xBC,0xBC,0xBC,0xBC,0xBC};
		uint8_t mac1_2[6] = {0xCB,0xCB,0xCB,0xCB,0xCB,0xCB};
		uint8_t mac2[6] = {0xCD,0xCD,0xCD,0xCD,0xCD,0xCD};
		uint8_t mac2_2[6] = {0xDC,0xDC,0xDC,0xDC,0xDC,0xDC};
		uint8_t ip1[4] = {192,168,0,7};
		uint8_t ip1_2[4] = {192,168,0,8};
		uint8_t ip2[4] = {10,0,1,4};
		uint8_t ip2_2[4] = {10,0,1,5};
		host1->setMACAddr(mac1, 0);
		host1->setMACAddr(mac1_2, 1);
		host1->setARPTable(mac2, ip2);
		host1->setARPTable(mac2_2, ip2_2);
		host1->setIPAddr(ip1, 0);
		host1->setIPAddr(ip1_2, 1);
		host1->setRoutingTable(ip2, 16, 0);
		host1->setRoutingTable(ip2_2, 16, 1);

		host2->setMACAddr(mac2, 0);
		host2->setMACAddr(mac2_2, 1);
		host2->setARPTable(mac1, ip1);
		host2->setARPTable(mac1_2, ip1_2);
		host2->setIPAddr(ip2, 0);
		host2->setIPAddr(ip2_2, 1);
		host2->setRoutingTable(ip1, 16, 0);
		host2->setRoutingTable(ip1_2, 16, 1);

		host1->getPort(0)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));
		host1->getPort(1)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));
		host2->getPort(0)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));
		host2->getPort(1)->setPropagationDelay(TimeUtil::makeTime(1, TimeUtil::MSEC));

		switchingHub = new Switch("Switch1", &netSystem, Unreliable);
		switchingHub->addPort(host1->getPort(0));
		switchingHub->addPort(host1->getPort(1));
		switchingHub->addPort(host2->getPort(0));
		switchingHub->addPort(host2->getPort(1));
		switchingHub->setQueueSize(16);

		switchingHub->addMACEntry(host1->getPort(0), mac1);
		switchingHub->addMACEntry(host1->getPort(1), mac1_2);
		switchingHub->addMACEntry(host2->getPort(0), mac2);
		switchingHub->addMACEntry(host2->getPort(1), mac2_2);

		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string file_name(test_info->name());
		file_name.append(".pcap");
		switchingHub->enablePCAPLogging(file_name);

		ethernet1 = new Ethernet(host1);
		ethernet2 = new Ethernet(host2);
		ipv4_1 = new IPv4(host1);
		ipv4_2 = new IPv4(host2);

		interface = Target::allocate(host1);
		interface2 = Adversary::allocate(host2);
		interface->initialize();
		interface2->initialize();
	}
	virtual void TearDown()
	{
		delete interface;
		delete interface2;


		delete ipv4_1;
		delete ipv4_2;

		delete ethernet1;
		delete ethernet2;


		delete switchingHub;

		delete host1;
		delete host2;
	}

	void runTest()
	{
		netSystem.run(TimeUtil::makeTime(1000, TimeUtil::SEC));

		host1->cleanUp();
		host2->cleanUp();
		interface2->finalize();
		interface->finalize();
		netSystem.run(TimeUtil::makeTime(2000, TimeUtil::SEC));
	}
};

template <class Target, class Adversary, int CLIENTS, int TIMEOUT>
class TestEnv3 : public ::testing::Test
{
protected:
	NetworkSystem netSystem;
	Host* server_host;
	Host** client_hosts;
	Switch* switchingHub;
	Ethernet* ethernet_server;
	Ethernet** ethernet_clients;
	IPv4* ipv4_server;
	IPv4** ipv4_clients;
	HostModule** interface_clients;
	HostModule* interface_server;

	const int num_client = CLIENTS;
	Size port_speed = 10000000;
	Time propagationDelay = TimeUtil::makeTime(10, TimeUtil::MSEC);
	uint64_t prev_log;

	virtual void SetUp()
	{
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
				0UL
		);

		client_hosts = new Host*[num_client];
		ethernet_clients = new Ethernet*[num_client];
		ipv4_clients = new IPv4*[num_client];
		interface_clients = new HostModule*[num_client];

		server_host = new Host("CongestionServer", 1, &netSystem);

		uint8_t server_mac[6] = {0xBC,0xBC,0xBC,0xBC,0xBC,0xBC};
		uint8_t server_ip[4] = {192,168,1,7};
		uint8_t server_mask[4] = {192,168,1,0};
		uint8_t client_mask[4] = {10,0,1,0};

		ethernet_server = new Ethernet(server_host);
		ipv4_server = new IPv4(server_host);

		server_host->setMACAddr(server_mac, 0);
		server_host->setIPAddr(server_ip, 0);
		server_host->setRoutingTable(client_mask, 24, 0);
		server_host->getPort(0)->setPropagationDelay(propagationDelay);
		server_host->getPort(0)->setPortSpeed(port_speed);

		interface_server = Adversary::allocate(server_host);
		interface_server->initialize();

		switchingHub = new Switch("Switch1", &netSystem);
		switchingHub->addPort(server_host->getPort(0));
		switchingHub->addMACEntry(server_host->getPort(0), server_mac);

		char name_buf[128];
		for(int k=0; k<num_client; k++)
		{
			snprintf(name_buf, sizeof(name_buf), "CongestionClient%d", k);
			client_hosts[k] = new Host(name_buf, 2, &netSystem);
			ethernet_clients[k] = new Ethernet(client_hosts[k]);
			ipv4_clients[k] = new IPv4(client_hosts[k]);
			interface_clients[k] = Target::allocate(client_hosts[k]);
			interface_clients[k]->initialize();

			uint8_t client_ip[4] = {10,0,1,10};
			client_ip[3] += k;
			uint8_t client_mac[6] = {0xEE,0xEE,0xEE,0xEE,0xEE,0x00};
			client_mac[5] += k;

			client_hosts[k]->setMACAddr(client_mac, 0);
			client_hosts[k]->setIPAddr(client_ip, 0);
			client_hosts[k]->setRoutingTable(server_mask, 24, 0);
			client_hosts[k]->setARPTable(server_mac, server_ip);
			client_hosts[k]->getPort(0)->setPropagationDelay(propagationDelay);
			client_hosts[k]->getPort(0)->setPortSpeed(port_speed);

			server_host->setARPTable(client_mac, client_ip);

			switchingHub->addPort(client_hosts[k]->getPort(0));
			switchingHub->addMACEntry(client_hosts[k]->getPort(0), client_mac);
		}

		switchingHub->setLinkSpeed(port_speed);
		switchingHub->setQueueSize(64);

		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string file_name(test_info->name());
		file_name.append(".pcap");
		switchingHub->enablePCAPLogging(file_name, 64);

	}
	virtual void TearDown()
	{
		delete interface_server;
		for(int k=0; k<num_client; k++)
			delete interface_clients[k];

		delete ipv4_server;
		for(int k=0; k<num_client; k++)
			delete ipv4_clients[k];

		delete ethernet_server;
		for(int k=0; k<num_client; k++)
			delete ethernet_clients[k];

		delete switchingHub;

		delete server_host;
		for(int k=0; k<num_client; k++)
			delete client_hosts[k];

		delete[] client_hosts;
		delete[] ethernet_clients;
		delete[] ipv4_clients;
		delete[] interface_clients;

		NetworkLog::defaultLevel = prev_log;
	}

	void runTest()
	{
		netSystem.run(TimeUtil::makeTime(TIMEOUT, TimeUtil::SEC));

		server_host->cleanUp();
		for(int k=0; k<num_client; k++)
			client_hosts[k]->cleanUp();

		interface_server->finalize();
		for(int k=0; k<num_client; k++)
			interface_clients[k]->finalize();
		netSystem.run(TimeUtil::makeTime(1000 + TIMEOUT, TimeUtil::SEC));
	}
};

//#define UNRELIABLE
//#define RUN_SOLUTION
#ifdef RUN_SOLUTION
typedef TestEnv1<TCPSolutionProvider> TestEnv_Reliable;
#ifdef UNRELIABLE
typedef TestEnv2<TCPSolutionProvider,TCPSolutionProvider, true> TestEnv_Any;
#else
typedef TestEnv2<TCPSolutionProvider,TCPSolutionProvider, false> TestEnv_Any;
#endif
typedef TestEnv2<TCPSolutionProvider,TCPSolutionProvider, true> TestEnv_Unreliable;
typedef TestEnv3<TCPSolutionProvider,TCPSolutionProvider, 1, 1000> TestEnv_Congestion0;
typedef TestEnv3<TCPSolutionProvider,TCPSolutionProvider, 2, 1000> TestEnv_Congestion1;
typedef TestEnv3<TCPSolutionProvider,TCPSolutionProvider, 8, 1000> TestEnv_Congestion2;
#else
typedef TestEnv1<TCPAssignmentProvider> TestEnv_Reliable;
#ifdef UNRELIABLE
typedef TestEnv2<TCPAssignmentProvider,TCPSolutionProvider, true> TestEnv_Any;
#else
typedef TestEnv2<TCPAssignmentProvider,TCPSolutionProvider, false> TestEnv_Any;
#endif
typedef TestEnv2<TCPAssignmentProvider,TCPSolutionProvider, true> TestEnv_Unreliable;
typedef TestEnv3<TCPAssignmentProvider,TCPSolutionProvider, 1, 1000> TestEnv_Congestion0;
typedef TestEnv3<TCPAssignmentProvider,TCPSolutionProvider, 2, 1000> TestEnv_Congestion1;
typedef TestEnv3<TCPAssignmentProvider,TCPSolutionProvider, 8, 1000> TestEnv_Congestion2;
#endif

#endif /* APP_TESTTCP_TESTENV_HPP_ */
