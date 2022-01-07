/*
 * E_Host.cpp
 *
 *  Created on: 2014. 11. 10.
 *      Author: Keunhong Lee
 */

#include <E/E_Module.hpp>
#include <E/E_System.hpp>
#include <E/E_TimeUtil.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Link.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_Wire.hpp>

namespace E {
Host::Host(std::string name, NetworkSystem &system)
    : NetworkModule(system), NetworkLog(static_cast<System &>(system)),
      networkSystem(system) {

  ports.clear();
  this->pidStart = 0;
  this->syscallIDStart = 0;
  addHostModule<DefaultSystemCall>(std::ref(*this));

  this->running = true;
}

Host::~Host() { ports.clear(); }

bool Host::isRunning(void) { return this->running; }

int Host::cleanUp(void) {
  this->running = false;
  int missing = 0;
  std::list<UUID> syscall_to_wakeup;
  for (auto iter : this->syscallMap) {
    syscall_to_wakeup.push_back(iter.first);
    print_log(SYSCALL_ERROR, "Unfinished system call at %s",
              this->getModuleName().c_str());
    missing++;
  }
  for (auto iter : syscall_to_wakeup) {
    this->returnSystemCall(iter, -1);
  }
  return missing;
}

Module::Message Host::messageReceived(const ModuleID from,
                                      Module::MessageBase &message) {
  if (typeid(message) == typeid(Wire::Message &)) {
    Wire::Message &portMessage = dynamic_cast<Wire::Message &>(message);
    assert(portMessage.type == Wire::MessageType::PACKET_FROM_PORT);
    if (this->running == true) {
      print_log(PACKET_FROM_HOST,
                "Host [%s] get a packet [size:%zu] from module [%s]",
                this->getModuleName().c_str(), portMessage.packet.getSize(),
                this->getModuleName(from).c_str());
      // this->freePacket(hostMessage->packet);

      this->sendPacketToModule({}, "Ethernet", std::move(portMessage.packet));
    }
    return nullptr;
  }

  if (typeid(message) == typeid(PacketPass &)) {
    PacketPass &packetPass = dynamic_cast<PacketPass &>(message);
    if (this->running == true) {
      std::string fromName = packetPass.from.value_or("Host");
      hostModuleMap[packetPass.to.value()]->packetArrived(
          fromName, std::move(packetPass.packet));
    }
  } else if (typeid(message) == typeid(Syscall &)) {
    Syscall &syscall = dynamic_cast<Syscall &>(message);

    assert(syscall.pid != -1);
    auto appIter = this->processInfoMap.find(syscall.pid);
    assert(appIter != this->processInfoMap.end());
    assert(syscall.pid == appIter->second.application->pid);

    Domain domain = 0;
    Protocol protocol = 0;
    switch (syscall.param.syscallNumber) {
    case SystemCallInterface::SystemCall::SOCKET: {
      domain = (Domain)std::get<int>(syscall.param.params[0]);
      protocol = (Domain)std::get<int>(syscall.param.params[2]);
      break;
    }
    case SystemCallInterface::SystemCall::NSLEEP:
    case SystemCallInterface::SystemCall::GETTIMEOFDAY: {
      break;
    }

    case SystemCallInterface::SystemCall::CLOSE:
    case SystemCallInterface::SystemCall::READ:
    case SystemCallInterface::SystemCall::WRITE:
    case SystemCallInterface::SystemCall::CONNECT:
    case SystemCallInterface::SystemCall::LISTEN:
    case SystemCallInterface::SystemCall::ACCEPT:
    case SystemCallInterface::SystemCall::BIND:
    case SystemCallInterface::SystemCall::GETSOCKNAME:
    case SystemCallInterface::SystemCall::GETPEERNAME: {

      int fd = std::get<int>(syscall.param.params[0]);
      auto nsIter = appIter->second.fdToDomain.find(fd);
      assert(nsIter != appIter->second.fdToDomain.end());

      domain = nsIter->second.first;
      protocol = nsIter->second.second;
      break;
    }
    default:
      assert(0);
    }

    Namespace ns = Namespace(domain, protocol);
    auto iter = interfaceMap.find(ns);

    if (iter != interfaceMap.end()) {
      auto iface = iter->second;
      UUID curSyscallID = this->syscallIDStart;
      do {
        if (syscallMap.find(curSyscallID) == syscallMap.end()) {
          syscallIDStart = curSyscallID + 1;
          syscallMap.insert({curSyscallID, syscall.pid});
          break;
        }
        curSyscallID++;
      } while (curSyscallID != this->syscallIDStart);
      assert(curSyscallID != this->syscallIDStart);

      print_log(SYSCALL_RAISED,
                "System call[syscall_no:%d, unique_id: %" PRIu64
                "] has raised from "
                "app[pid:%d] at [%s]",
                syscall.param.syscallNumber, curSyscallID, syscall.pid,
                this->getModuleName().c_str());
      iface->systemCallback(curSyscallID, syscall.pid, syscall.param);
    }
  } else if (typeid(message) == typeid(Timer &)) {
    Timer &timer = dynamic_cast<Timer &>(message);
    timerModuleMap[timer.from]->timerCallback(timer.payload);
  } else if (typeid(message) == typeid(Return &)) {
    Return &ret = dynamic_cast<Return &>(message);
    auto iter = processInfoMap.find(ret.pid);
    assert(iter != processInfoMap.end());

    for (auto allSyscall : syscallMap) {
      (void)allSyscall;
      assert(
          allSyscall.second !=
          iter->second.application->pid); // no syscall pending for returned app
    }
    networkSystem.delRunnable(iter->second.application);
    processInfoMap.erase(iter);

    print_log(APPLICATION_RETRUN, "Application [ pid: %d] returend %d", ret.pid,
              ret.returnValue);

  } else {
    assert(0);
  }

  return nullptr;
}
void Host::messageFinished(const ModuleID to, Module::Message message,
                           Module::MessageBase &response) {
  (void)to;
  assert(Module::EmptyMessage::shared() ==
         dynamic_cast<Module::EmptyMessage &>(response));
}

void Host::messageCancelled(const ModuleID to, Module::Message message) {
  (void)to;
}

std::any Host::diagnoseHostModule(const char *moduleName, std::any arg) {
  return hostModuleMap[moduleName]->diagnose(arg);
}

void Host::sendPacket(size_t portIndex, Packet &&packet) {
  assert(portIndex < ports.size());
  if (this->running == false) {
    return;
  }

  auto portID = ports[portIndex];
  auto portMessage =
      std::make_unique<Wire::Message>(Wire::PACKET_TO_PORT, std::move(packet));
  sendMessage(portID, std::move(portMessage), 0);
}

Host::DefaultSystemCall::DefaultSystemCall(Host &host)
    : SystemCallInterface(0, 0, host), TimerModule("DefaultSyscall", host) {}
Host::DefaultSystemCall::~DefaultSystemCall() {}

void Host::DefaultSystemCall::timerCallback(std::any payload) {
  UUID syscallUUID = std::any_cast<UUID>(payload);
  returnSystemCall(syscallUUID, 0);
}

void Host::DefaultSystemCall::systemCallback(UUID syscallUUID, int pid,
                                             const SystemCallParameter &param) {
  (void)pid;
  switch (param.syscallNumber) {
  case SystemCallInterface::SystemCall::NSLEEP: {
    addTimer(syscallUUID, std::get<uint64_t>(param.params[0]));
    break;
  }
  case SystemCallInterface::SystemCall::GETTIMEOFDAY: {
    Time curTime =
        static_cast<SystemCallInterface *>(this)->host.getCurrentTime();

    struct timeval *tv = (struct timeval *)std::get<void *>(param.params[0]);
    struct timezone *tz = (struct timezone *)std::get<void *>(param.params[1]);

    Time usec = TimeUtil::getTime(curTime, TimeUtil::USEC);
    if (tz != nullptr) {
      // Timezone is not implemented yet.
      this->returnSystemCall(syscallUUID, -EINVAL);
    } else {
      tv->tv_usec = usec % 1000000;
      tv->tv_sec = usec / 1000000;
      this->returnSystemCall(syscallUUID, 0);
    }
    break;
  }
  default:
    assert(0);
  }
}

HostModule::HostModule(std::string name, Host &host) : host(host), name(name) {}
HostModule::~HostModule() {}

std::string HostModule::getHostModuleName() { return name; }

void HostModule::sendPacket(std::string toModule, Packet &&packet) {
  host.sendPacketToModule(name, toModule, std::move(packet));
}
void HostModule::sendPacket(std::string toModule, const Packet &packet) {
  sendPacket(toModule, Packet(packet));
}
Time HostModule::getCurrentTime() { return host.getCurrentTime(); }

Size HostModule::getWireSpeed(int port_num) {
  return host.getWireSpeed(port_num);
}

size_t HostModule::getPortCount() { return host.getPortCount(); }

void HostModule::print_log(uint64_t level, const char *format, ...) {
  va_list arglist;
  va_start(arglist, format);
  host.vprint_log(level, format, arglist);
  va_end(arglist);
}

SystemCallInterface::SystemCallInterface(int domain, int protocol, Host &host)
    : host(host), domain(domain), protocol(protocol) {}
SystemCallInterface::~SystemCallInterface() {}

void SystemCallInterface::returnSystemCall(UUID syscallUUID, int val) {
  host.returnSystemCall(syscallUUID, val);
}
int SystemCallInterface::createFileDescriptor(int processID) {
  return host.createFileDescriptor(domain, protocol, processID);
}

void SystemCallInterface::removeFileDescriptor(int processID, int fd) {
  return host.removeFileDescriptor(processID, fd);
}

void Host::sendPacketToModule(std::optional<std::string> fromModule,
                              std::string toModule, Packet &&packet) {
  auto it = hostModuleMap.find(toModule);

  if (toModule.compare("Host") == 0) {
    mac_t my_mac;
    packet.readData(6, my_mac.data(), 6);

    int selected_port = 0;
    for (size_t k = 0; k < this->ports.size(); k++) {
      auto port_mac = this->getMACAddr(k);
      if (my_mac == port_mac.value()) {
        selected_port = (int)k;
        break;
      }
    }
    this->sendPacket(selected_port, std::move(packet));
  } else if (it == hostModuleMap.end()) {
    print_log(MODULE_ERROR, "No module named [%s] has found. Drop packet.",
              toModule.c_str());
  } else {
    auto hostMessage = std::make_unique<PacketPass>(
        std::move(fromModule), std::move(toModule), std::move(packet));

    this->sendMessageSelf(std::move(hostMessage),
                          0); // DELAY module packet transfer delay
  }
}

UUID Host::addTimer(std::string fromModule, std::any payload, Time timeAfter) {
  auto timerMessage = std::make_unique<Timer>(fromModule, payload);
  return this->sendMessageSelf(std::move(timerMessage), timeAfter);
}

void Host::cancelTimer(UUID key) { this->cancelMessage(key); }

UUID Host::issueSystemCall(
    int pid, const SystemCallInterface::SystemCallParameter &param) {

  auto hostMessage = std::make_unique<Syscall>(pid, param);

  return this->sendMessageSelf(std::move(hostMessage), 0);
}

void Host::returnSystemCall(UUID syscallUUID, int val) {
  if (syscallMap.find(syscallUUID) == syscallMap.end()) {
    print_log(NetworkLog::SYSCALL_ERROR,
              "Invalid System call [%" PRIu64 "] at [%s].", syscallUUID,
              this->getModuleName().c_str());
    // assert(0); now we implement auto-closing unclosed system calls
    return;
  }
  print_log(NetworkLog::SYSCALL_FINISHED,
            "System call [%" PRIu64
            "] at [%s] has finished with return value [%d].",
            syscallUUID, this->getModuleName().c_str(), val);

  auto iter = syscallMap.find(syscallUUID);
  auto app = processInfoMap[iter->second].application;

  app->returnSyscall(val);
  networkSystem.addRunnable(app);
  syscallMap.erase(iter);
}

int Host::createFileDescriptor(int domain, int protocol, int processID) {
  assert(processInfoMap.find(processID) != processInfoMap.end());
  ProcessInfo &procInfo = processInfoMap.find(processID)->second;
  int current = 3;
  // assert(procInfo.fdSet.find(processID) != procInfo.fdSet.end());

  auto it = procInfo.fdToDomain.begin();

  while (it != procInfo.fdToDomain.end() && it->first == current) {
    ++current;
    ++it;
  }

  if (current >= MAX_FD) {

    print_log(NetworkLog::SYSCALL_ERROR, "Out of FD for process %d.",
              processID);
    return -1;
  }
  procInfo.fdToDomain.insert(
      std::pair<int, Namespace>(current, Namespace(domain, protocol)));

  return current;
}

void Host::removeFileDescriptor(int processID, int fd) {
  // TIME_WAIT occurs after process termination
  if (processInfoMap.find(processID) != processInfoMap.end()) {
    ProcessInfo &procInfo = processInfoMap.find(processID)->second;

    procInfo.fdToDomain.erase(fd);
  }
}

int Host::registerProcess(std::shared_ptr<SystemCallApplication> app) {
  int start = pidStart;
  int current = start;
  do {
    if (processInfoMap.find(current) == processInfoMap.end()) {
      ProcessInfo procInfo;
      app->pid = current;
      procInfo.application = std::move(app);
      processInfoMap.insert(
          std::pair<int, ProcessInfo>(current, std::move(procInfo)));
      pidStart = (current + 1) % MAX_PID;

      // found proper fd number
      return current;
    }

    current = (current + 1) % MAX_FD;
  } while (start != current);

  print_log(NetworkLog::SYSCALL_ERROR, "Out of PID.");
  return -1;
}

void Host::launchApplication(int pid) {
  auto iter = processInfoMap.find(pid);
  assert(iter != processInfoMap.end());
  assert(iter->second.application->pid == pid);
  iter->second.application->initialize();
  networkSystem.addRunnable(iter->second.application);
}

Size Host::getWireSpeed(int port_num) {
  return networkSystem.getWireSpeed(ports[port_num]);
}

void Host::exitProcess(int pid, int returnValue) {

  auto retMessage = std::make_unique<Return>(pid, returnValue);
  sendMessageSelf(std::move(retMessage), 0);
}

void SystemCallApplication::finalizeApplication(int returnValue) {

  this->host.exitProcess(this->pid, returnValue);
}

void SystemCallApplication::main() {
  int returnValue = E_Main();
  finalizeApplication(returnValue);
}

SystemCallApplication::SystemCallApplication(Host &host)
    : Runnable(), host(host), pid(-1) {}
SystemCallApplication::~SystemCallApplication() {}

void SystemCallApplication::initialize() { start(); }

int SystemCallApplication::E_Syscall(
    const SystemCallInterface::SystemCallParameter &param) {
  if (!this->host.isRunning())
    return -1;

  syscallRet = -1;
  host.issueSystemCall(pid, param);
  wait();
  return syscallRet;
}

void SystemCallApplication::returnSyscall(int retVal) {
  syscallRet = retVal;
  ready();
}
Time SystemCallApplication::getCurrentTime() { return host.getCurrentTime(); }

} // namespace E
