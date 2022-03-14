#ifndef E_ECHOASSIGNMENT_HPP_
#define E_ECHOASSIGNMENT_HPP_

#ifndef NON_KENS

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/Networking/E_Host.hpp>

#include <E/Networking/TCP/E_TCPApplication.hpp>

#else

#include <sys/socket.h>
#include <unistd.h>

namespace E {
class Host {};
class TCPApplication {
public:
  TCPApplication(Host &host) {}
};
} // namespace E

#endif

#include <map>
#include <string>
#include <vector>

using namespace E;

class EchoTesting;
class EchoAssignment : public TCPApplication {
public:
  EchoAssignment(Host &host, std::map<std::string, std::string> &answers,
                 int argc, char *argv[])
      : TCPApplication(host), ___answers(answers), argc(argc), argv(argv) {}

  int serverMain(const char *bind_ip, int port, const char *server_hello);
  int clientMain(const char *server_ip, int port, const char *msg);
  int Main(int argc, char *argv[]);

  int E_Main() { return Main(argc, argv); }

private:
  int argc;
  char **argv;
  std::map<std::string, std::string> &___answers;

  // ip: peer(client or server)'s IP address
  // answer: peer's request/response
  void submitAnswer(const char *ip, const char *answer) {
    ___answers[ip] = answer;
  }
  friend class EchoTesting;
};

#endif