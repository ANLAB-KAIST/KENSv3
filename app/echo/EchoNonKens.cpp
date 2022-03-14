
#include "EchoAssignment.hpp"

int main(int argc, char *argv[]) {
  Host host;
  std::map<std::string, std::string> answers;
  EchoAssignment assignment(host, answers, argc, argv);

  int ret = assignment.E_Main();
  printf("Submitted answers:\n");
  for (auto &answer : answers) {
    printf("  %s -> %s\n", answer.first.c_str(), answer.second.c_str());
  }
  return ret;
}