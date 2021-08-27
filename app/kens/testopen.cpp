/*
 * testopen.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: leeopop
 */

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>
#include <E/Networking/TCP/E_TCPApplication.hpp>
#include <E/Networking/TCP/E_TCPSolution.hpp>

#include <arpa/inet.h>

#include "testenv.hpp"
#include <gtest/gtest.h>

using namespace E;

class TestOpen : public TCPApplication {
public:
  TestOpen(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    const static int test_size = 1024;
    const static int test_repeat = 128;
    size_t failed = 0;
    size_t duplicated = 0;
    size_t success = 0;
    for (int j = 0; j < test_repeat; j++) {
      int fd_array[test_size];
      for (int k = 0; k < test_size; k++) {
        fd_array[k] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd_array[k] < 0)
          failed++;
        else
          success++;
      }

      for (int k = 0; k < test_size; k++) {
        for (int i = k + 1; i < test_size; i++) {
          if (fd_array[k] == fd_array[i])
            duplicated++;
        }
      }
      for (int k = 0; k < test_size; k++) {
        close(fd_array[k]);
      }
    }
    EXPECT_EQ(success, ((size_t)test_size) * ((size_t)test_repeat));
    EXPECT_EQ(failed, 0);
    EXPECT_EQ(duplicated, 0);
    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestOpen) {

  int pid = host1->addApplication<TestOpen>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}
