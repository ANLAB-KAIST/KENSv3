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
#include <E/Networking/E_RoutingInfo.hpp>
extern "C"
{
#include <sys/time.h>
}

namespace E
{
class Port;
class Host;
class TCPApplication;

/**
 * @brief HostModule is an interface for classes
 * which is registered to a certain Host.
 * HostModules can communicate with each other by their names.
 *
 * @note HostModule has no relationship with Module.
 */
class HostModule
{
private:
	Host* host;
	std::string name;

public:
	/**
	 * @brief Create a HostModule. It is automatically registered to the Host.
	 * Module is registered to a System when it is created.
	 * @param name Name of this module. This name is used for identifying each module.
	 * @param host Host to be registered.
	 */
	HostModule(std::string name, Host* host);
	virtual ~HostModule();

	/**
	 * @return My name used in the registered Host.
	 */
	virtual std::string getHostModuleName() final;

	/**
	 * @return Host which this module belongs to.
	 * @note You cannot override this function.
	 */
	virtual Host* getHost() final;

	/**
	 * @brief This function is automatically called by Host just before the simulation begins.
	 * You can override this function if needed.
	 */
	virtual void initialize(void) {};

	/**
	 * @brief This function is automatically called by Host just after the simulation ends.
	 * You can override this function if needed.
	 */
	virtual void finalize(void) {};
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
	virtual void packetArrived(std::string fromModule, Packet* packet) = 0;

	/**
	 * @brief This function transfers Packets among HostModules in the Host.
	 * Unlike Module::Message, we use fire-and-forget policy with Packets.
	 * Once you send a Packet using sendPacket, you DO NOT HAVE TO FREE THAT PACKET.
	 *
	 * @param toModule Name of the destination HostModule.
	 * @param packet Packet to be sent.
	 */
	virtual void sendPacket(std::string toModule, Packet* packet) final;

	friend class Host;
};

/**
 * @brief If you want to handle system call events of the Host,
 * you should inherit this class.
 * This class provides file descriptor management,
 * and blocking behavior of system calls.
 *
 */
class SystemCallInterface
{
public:
	static const int AF_INET = 2;
	static const int IPPROTO_TCP = 6;
	static const int IPPROTO_UDP = 17;
	enum SystemCall
	{
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

	class SystemCallParameter
	{
	public:
		enum SystemCall syscallNumber;
		union
		{
			int param1_int;
			void* param1_ptr;
			long param1_long;
		};
		union
		{
			int param2_int;
			void* param2_ptr;
			long param2_long;
		};
		union
		{
			int param3_int;
			void* param3_ptr;
			long param3_long;
		};
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
	SystemCallInterface(int domain, int protocol, Host* host);
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
	virtual void systemCallback(UUID syscallUUID, int pid, const SystemCallParameter& param) = 0;

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
	Host* host;
	int domain;
	int protocol;
};

/**
 * @brief This provides system call interface to an application.
 * This provides basic system call invoking mechanism to the application.
 *
 * @see TCPApplication
 */
class SystemCallApplication : public Module, private NetworkLog, public Runnable
{
public:
	SystemCallApplication(Host* host);
	virtual ~SystemCallApplication();

	enum MessageType
	{
		SYSCALL_FINISHED,
	};
	class Message : public Module::Message
	{
	public:
		enum MessageType type;
		int returnValue;
		UUID syscallID;
		std::condition_variable* condVar;
	};

	/**
	 * @brief Standard C++11 thread is automatically launched,
	 * so we cannot control the starting time.
	 * Calling this function guarantees that E_Main thread is
	 * successfully launched.
	 */
	virtual void initialize() final;

protected:
	/**
	 * @brief This is equivalent to INT 0x80 instruction of Linux.
	 * This function may be blocked if the system call is blocking call.
	 * @param param Parameters for system call.
	 * @note You cannot override this function.
	 */
	virtual int E_Syscall(const SystemCallInterface::SystemCallParameter& param) final;

	/**
	 * @brief This does a role of int main(int argc, char** argv, char** env).
	 * The main functions of multiple applications run in parallel.
	 * @param param Parameters for system call.
	 * @note You cannot override this function.
	 */
	virtual void E_Main() = 0;

private:

	virtual void registerApplication() final;
	virtual void unregisterApplication() final;
	static void __callMain(SystemCallApplication* app);

	virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
	virtual void messageCancelled(Module* to, Module::Message* message) final;

	std::unique_lock<std::mutex> *my_lock_ptr;
	Host* host;
	int pid;
	std::mutex initial_barrier_mutex;
	std::condition_variable initial_barrier_cond;

	std::thread *thread;

	friend class Host;
	friend class TCPApplication;
};

/**
 * @brief This class abstract a single host machine.
 */
class Host : public Module, public NetworkModule, private NetworkLog, public RoutingInfo
{
public:
	typedef int Domain;
	typedef int Protocol;
	typedef std::pair<Domain, Protocol> Namespace;
	typedef SystemCallInterface* Interface;
	typedef SystemCallApplication* Application;
private:
	static const int MAX_FD = 65536;
	static const int MAX_PID = 65536;
	std::vector<Port*> allPort;
	Interface defaultInterface;

	class DefaultSystemCall : public SystemCallInterface, public Module
	{
	public:
		DefaultSystemCall(Host* host);
		virtual ~DefaultSystemCall();

		enum MessageType
		{
			NANOSLEEP,
		};
		class Message : public Module::Message
		{
		public:
			enum MessageType type;
			UUID wakeupSyscallID;
		};

	protected:
		virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
		virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
		virtual void systemCallback(UUID syscallUUID, int pid, const SystemCallParameter& param) final;
	};

	std::unordered_map<Namespace, Interface> namespaceToInterface;
	std::unordered_map<UUID, Application> syscallIDToApplication;
	std::unordered_map<UUID, std::pair<int*, std::condition_variable*>> syscallIDToWakeup;

	class ProcessInfo
	{
	public:
		Application application;
		std::unordered_set<int> fdSet;
		std::unordered_map<int, Namespace> fdToDomain;
		int fdStart;
	};
	std::unordered_map<int, ProcessInfo> pidToProcessInfo;

	int pidStart;
	UUID syscallIDStart;
	bool running;

	std::unordered_map<std::string, HostModule*> hostModuleMap;

	virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
	virtual void messageCancelled(Module* to, Module::Message* message) final;

public:
	Host(std::string name, size_t portNumber, NetworkSystem* system);
	virtual ~Host();
	virtual int cleanUp(void) final;
	virtual bool isRunning(void) final;

	enum MessageType
	{
		SYSCALL_CALLED,
		PACKET_TRANSFER,
	};

	class Message : public Module::Message
	{
	public:
		enum MessageType type;
		union
		{
			Packet* packet;
			struct
			{
				SystemCallInterface::SystemCallParameter param;
				std::condition_variable* condVar;
				int* returnValue;
			}syscall;
			struct
			{
				HostModule* from;
				HostModule* to;
				Packet* packet;
			}packetPass;
		};
	};

	virtual Port* getPort(size_t portIndex) final;
	virtual size_t getPortCount() final;
	virtual void sendPacket(size_t portIndex, Packet* packet) final;

private:
	virtual void registerHostModule(std::string name, HostModule* hostModule) final;
	virtual void unregisterHostModule(std::string name) final;
	virtual HostModule* findHostModule(std::string name) final;
	virtual void sendPacketToModule(HostModule* fromModule, std::string toModule, Packet* packet) final;

	virtual void registerInterface(Interface iface, Domain domain, Protocol protocol) final;
	virtual void returnSystemCall(Interface iface, UUID syscallUUID, int val) final;
	virtual int createFileDescriptor(Interface iface, int processID) final;
	virtual void removeFileDescriptor(Interface iface, int processID, int fd) final;
	virtual int registerProcess(Application app) final;
	virtual void unregisterProcess(int pid) final;

	friend HostModule::HostModule(std::string name, Host* host);
	friend HostModule::~HostModule();
	friend void HostModule::sendPacket(std::string toModule, Packet* packet);

	friend SystemCallInterface::SystemCallInterface(int domain, int protocol, Host* host);
	friend void SystemCallInterface::returnSystemCall(UUID syscallUUID, int val);
	friend int SystemCallInterface::createFileDescriptor(int processID);
	friend void SystemCallInterface::removeFileDescriptor(int processID, int fd);
	friend void SystemCallApplication::registerApplication();
	friend void SystemCallApplication::unregisterApplication();

};

}

#endif /* E_HOST_HPP_ */
