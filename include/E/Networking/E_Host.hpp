/**
 * @file   E_Host.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Host and other interfaces.
 * E::HostModule, E::SystemCallInterface, E::SystemCallApplication
 */

#ifndef E_HOST_HPP_
#define E_HOST_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_RoutingInfo.hpp>
#include <E/Networking/E_TimerModule.hpp>
#include <E/Networking/E_Wire.hpp>
extern "C" {
#include <sys/time.h>
}

namespace E {
class Host;
class TCPApplication;

/**
 * @brief HostModule is an interface for classes
 * which is registered to a certain Host.
 * HostModules can communicate with each other by their names.
 *
 * @note HostModule has no relationship with Module.
 */
class HostModule {
private:
  Host &host;
  std::string name;

public:
  /**
   * @brief Create a HostModule. It is automatically registered to the Host.
   * Module is registered to a System when it is created.
   * @param name Name of this module. This name is used for identifying each
   * module.
   * @param host Host to be registered.
   */
  HostModule(std::string name, Host &host);
  virtual ~HostModule();

  /**
   * @return My name used in the registered Host.
   */
  virtual std::string getHostModuleName() final;

  /**
   * @brief This function is automatically called by Host just before the
   * simulation begins. You can override this function if needed.
   */
  virtual void initialize(void){};

  /**
   * @brief This function is automatically called by Host just after the
   * simulation ends. You can override this function if needed.
   */
  virtual void finalize(void){};

  /**
   * @brief Host module control function
   */
  virtual std::any diagnose(std::any param) { return 0; };

protected:
  /**
   * @brief This function is automatically called by Host
   * when this module receives a Packet.
   * You must override this function to handle Packet events.
   * When a Packet is received, you must FREE IT OR PASS IT TO OTHER HOSTMODULE.
   *
   * @param fromModule Name of the HostModule who sent this packet.
   * @param packet Received packet.
   */
  virtual void packetArrived(std::string fromModule, Packet &&packet) = 0;

  /**
   * @brief This function transfers Packets among HostModules in the Host.
   * Unlike Module::Message, we use fire-and-forget policy with Packets.
   * Once you send a Packet using sendPacket, you DO NOT HAVE TO FREE THAT
   * PACKET.
   *
   * @param toModule Name of the destination HostModule.
   * @param packet Packet to be sent.
   */
  virtual void sendPacket(std::string toModule, Packet &&packet) final;
  void sendPacket(std::string toModule, const Packet &packet);

  /**
   * @return Returns current virtual clock of the System.
   */
  Time getCurrentTime();

  /**
   * @brief Get cost for local port (link)
   *
   * @param port_num querying port's number
   * @return local link cost
   */
  Size getWireSpeed(int port_num);

  /**
   * @brief Get the number of ports
   *
   * @return the number of ports
   */
  size_t getPortCount();

  /**
   * @brief Prints log with specified log level and format.
   * NetworkLog::print_log prints logs specified in log level parameter.
   * For example, if log level is set to TCP_LOG, it only prints TCP_LOG logs.
   * If you want to print multiple log levels in NetworkLog,
   * you can set log level with OR operation (i.e.  SYSCALL_ERROR |
   * MODULE_ERROR).
   *
   * @note Log::print_log
   *
   * @param level log level
   * @param format Format string
   * @param ... Print arguments for format string
   *
   */
  void print_log(uint64_t level, const char *format, ...)
#ifdef HAVE_ATTR_FORMAT
      __attribute__((format(printf, 3, 4)))
#endif
      ;

  friend class Host;
};

/**
 * @brief If you want to handle system call events of the Host,
 * you should inherit this class.
 * This class provides file descriptor management,
 * and blocking behavior of system calls.
 *
 */
class SystemCallInterface {
public:
  static constexpr int AF_INET = 2;
  static constexpr int IPPROTO_TCP = 6;
  static constexpr int IPPROTO_UDP = 17;

  static constexpr int max_param = 3;

  enum SystemCall {
    SOCKET,
    CLOSE,
    READ,
    WRITE,
    CONNECT,
    LISTEN,
    ACCEPT,
    BIND,
    GETSOCKNAME,
    GETPEERNAME,

    NSLEEP,
    GETTIMEOFDAY,
  };

  class SystemCallParameter {
  public:
    enum SystemCall syscallNumber;
    std::array<std::variant<void *, int8_t, int16_t, int32_t, int64_t, uint8_t,
                            uint16_t, uint32_t, uint64_t>,
               max_param>
        params;
  };

protected:
  /**
   * @brief Multiple SystemCallInterface may be installed to a Host.
   * They are distinguished by their Domain and Protocol.
   * The interface is automatically registered to the Host when created.
   *
   * @param domain Address domain (e.g. AF_INET).
   * @param protocol Protocl to use (e.g. IPPROTO_TCP).
   * @param host Host to be registered.
   */
  SystemCallInterface(int domain, int protocol, Host &host);
  virtual ~SystemCallInterface();

  /**
   * @brief When a system call is invoked by an application,
   * this callback function is automatically called.
   * To handle system call events, override this function.
   *
   * @note IMPORTANT: Every system call automatically blocks the
   * caller application. Every system call should be unblocked manually.
   * For simple non-blocking functions, just call the unblocking utility
   * at the end of the handler.
   *
   * @param syscallUUID Unique ID for this system call request.
   * @param pid Process who raised this system call request.
   * @param param Parameters given this system call.
   *
   * @see returnSystemCall
   */
  virtual void systemCallback(UUID syscallUUID, int pid,
                              const SystemCallParameter &param) = 0;

  /**
   * @brief Unblocks a blocked system call with return value.
   *
   * @param syscallUUID Unique ID for the system call to be unblocked.
   * @param val return value of the system call.
   * @note You cannot override this function.
   *
   * @see returnSystemCall
   */
  virtual void returnSystemCall(UUID syscallUUID, int val) final;

  /**
   * @brief Create a file descriptor for a certain process.
   * The created file descriptor is automatically bound to
   * the domain and protocol of this module.
   * Proper system call request using this file descriptor
   * will be passed to the SystemCallInterface
   * which created the file descriptor.
   *
   * @param processID Create a file descriptor for this process.
   * @return Created file descriptor.
   * @note You cannot override this function.
   */
  virtual int createFileDescriptor(int processID) final;

  /**
   * @brief Close a file descriptor.
   * Closed file descriptor becomes invalid
   * and loses connection between this SystemCallInterface.
   * Any system call request on invalid file descriptor will fail.
   *
   * @param processID PID of the file descriptor owner.
   * @param fd File descriptor to be closed.
   * @note You cannot override this function.
   */
  virtual void removeFileDescriptor(int processID, int fd) final;

  friend class Host;

private:
  Host &host;
  int domain;
  int protocol;
};

/**
 * @brief This provides system call interface to an application.
 * This provides basic system call invoking mechanism to the application.
 *
 * @see TCPApplication
 */
class SystemCallApplication : public Runnable {
public:
  SystemCallApplication(Host &host);
  virtual ~SystemCallApplication();

protected:
  /**
   * @brief Standard C++11 thread is automatically launched,
   * so we cannot control the starting time.
   * Calling this function guarantees that E_Main thread is
   * successfully launched.
   */
  virtual void initialize() final;
  /**
   * @brief This is equivalent to INT 0x80 instruction of Linux.
   * This function may be blocked if the system call is blocking call.
   * @param param Parameters for system call.
   * @note You cannot override this function.
   */
  virtual int
  E_Syscall(const SystemCallInterface::SystemCallParameter &param) final;

  virtual void returnSyscall(int retVal) final;

  /**
   * @brief This does a role of int main(int argc, char** argv, char** env).
   * The main functions of multiple applications run in parallel.
   * @param param Parameters for system call.
   * @note You cannot override this function.
   */
  virtual int E_Main() = 0;

  /**
   * @return Returns current virtual clock of the System.
   */
  Time getCurrentTime();

private:
  virtual void main() override final;
  virtual void finalizeApplication(int returnValue) final;

private:
  Host &host;
  int pid;
  int syscallRet = 0;

  friend class Host;
  friend class TCPApplication;
};

/**
 * @brief This class abstract a single host machine.
 */
class Host : public NetworkModule, public NetworkLog, public RoutingInfo {
public:
  using Domain = int;
  using Protocol = int;
  using Namespace = std::pair<Domain, Protocol>;

private:
  static constexpr int MAX_FD = 65536;
  static constexpr int MAX_PID = 65536;

  class DefaultSystemCall : public SystemCallInterface, public TimerModule {
  public:
    DefaultSystemCall(Host &host);
    virtual ~DefaultSystemCall();

  protected:
    virtual void systemCallback(UUID syscallUUID, int pid,
                                const SystemCallParameter &param) final;
    virtual void timerCallback(std::any payload) final;
  };

  class ProcessInfo {
  public:
    std::shared_ptr<SystemCallApplication> application;
    std::map<int, Namespace> fdToDomain;
  };

  int pidStart;
  UUID syscallIDStart;
  bool running;
  NetworkSystem &networkSystem;

  std::unordered_map<Namespace, std::shared_ptr<SystemCallInterface>>
      interfaceMap;
  std::unordered_map<std::string, std::shared_ptr<HostModule>> hostModuleMap;
  std::unordered_map<std::string, std::shared_ptr<TimerModule>> timerModuleMap;
  std::unordered_map<int, ProcessInfo> processInfoMap;
  std::unordered_map<UUID, int> syscallMap; // to PID

  virtual Module::Message messageReceived(const ModuleID from,
                                          Module::MessageBase &message) final;
  virtual void messageFinished(const ModuleID to, Module::Message message,
                               Module::MessageBase &response) final;
  virtual void messageCancelled(const ModuleID to,
                                Module::Message message) final;

public:
  Host(std::string name, NetworkSystem &system);
  virtual ~Host();
  virtual int cleanUp(void) final;
  virtual bool isRunning(void) final;
  template <typename T, typename... Args> void addHostModule(Args &&...args) {
    static_assert(std::is_base_of<HostModule, T>::value ||
                  std::is_base_of<TimerModule, T>::value ||
                  std::is_base_of<SystemCallInterface, T>::value);

    auto hostModule = std::make_shared<T>(std::forward<Args>(args)...);
    if constexpr (std::is_base_of<HostModule, T>::value) {
      auto hostModuleName = hostModule->getHostModuleName();
      bool ret = hostModuleMap.insert({hostModuleName, hostModule}).second;
      (void)ret;
      assert(ret);
    }

    if constexpr (std::is_base_of<TimerModule, T>::value) {
      auto timerModuleName = hostModule->getTimerModuleName();
      bool ret = timerModuleMap.insert({timerModuleName, hostModule}).second;
      (void)ret;
      assert(ret);
    }

    if constexpr (std::is_base_of<SystemCallInterface, T>::value) {
      bool ret =
          interfaceMap
              .insert({Namespace(hostModule->domain, hostModule->protocol),
                       hostModule})
              .second;
      (void)ret;
      assert(ret);
    }
  }
  template <typename T, typename... Args> int addApplication(Args &&...args) {
    static_assert(std::is_base_of<SystemCallApplication, T>::value);
    auto application = std::make_shared<T>(std::forward<Args>(args)...);
    return registerProcess(std::move(application));
  }

  void initializeHostModule(const char *name) {
    hostModuleMap[name]->initialize();
  }
  void finalizeHostModule(const char *name) { hostModuleMap[name]->finalize(); }
  void launchApplication(int pid);
  std::any diagnoseHostModule(const char *name, std::any param);
  Size getWireSpeed(int port_num);

  class Syscall : public Module::MessageBase {
  public:
    int pid;
    SystemCallInterface::SystemCallParameter param;
    Syscall(int pid, SystemCallInterface::SystemCallParameter param)
        : pid(pid), param(param) {}
    ~Syscall() override {}
  };

  // Application Return
  class Return : public Module::MessageBase {
  public:
    int pid;
    int returnValue;
    Return(int pid, int returnValue) : pid(pid), returnValue(returnValue) {}
    ~Return() override {}
  };
  class PacketPass : public Module::MessageBase {
  public:
    std::optional<std::string> from;
    std::optional<std::string> to;
    Packet packet;
    PacketPass(std::optional<std::string> from, std::optional<std::string> to,
               Packet &&packet)
        : from(from), to(to), packet(packet) {}
    PacketPass(Packet &&packet) : from({}), to({}), packet(packet) {}
    ~PacketPass() override {}
  };
  class Timer : public Module::MessageBase {
  public:
    std::string from;
    std::any payload;
    Timer(std::string from, std::any payload) : from(from), payload(payload) {}
    ~Timer() override {}
  };

  virtual void sendPacket(size_t portIndex, Packet &&packet) final;

private:
  virtual void sendPacketToModule(std::optional<std::string> fromModule,
                                  std::string toModule, Packet &&packet) final;

  virtual UUID addTimer(std::string fromModule, std::any payload,
                        Time timeAfter) final;
  virtual void cancelTimer(UUID key) final;
  virtual UUID
  issueSystemCall(int pid,
                  const SystemCallInterface::SystemCallParameter &param) final;

  virtual void returnSystemCall(UUID syscallUUID, int val) final;
  virtual int createFileDescriptor(int domain, int protocol,
                                   int processID) final;
  virtual void removeFileDescriptor(int processID, int fd) final;
  virtual int registerProcess(std::shared_ptr<SystemCallApplication> app) final;
  virtual void exitProcess(int pid, int returnValue) final;

  friend HostModule::HostModule(std::string name, Host &host);
  friend HostModule::~HostModule();
  friend void HostModule::sendPacket(std::string toModule, Packet &&packet);

  friend SystemCallInterface::SystemCallInterface(int domain, int protocol,
                                                  Host &host);
  friend void SystemCallInterface::returnSystemCall(UUID syscallUUID, int val);
  friend int SystemCallInterface::createFileDescriptor(int processID);
  friend void SystemCallInterface::removeFileDescriptor(int processID, int fd);

  friend int SystemCallApplication::E_Syscall(
      const SystemCallInterface::SystemCallParameter &param);
  friend void SystemCallApplication::finalizeApplication(int returnValue);
  friend UUID TimerModule::addTimer(std::any payload, Time timeAfter);
  friend void TimerModule::cancelTimer(UUID key);
};

} // namespace E

#endif /* E_HOST_HPP_ */
