/*
 * E_Host.cpp
 *
 *  Created on: 2014. 11. 10.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Port.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/E_Module.hpp>
#include <E/E_System.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Link.hpp>
#include <E/E_TimeUtil.hpp>

namespace E
{
Host::Host(std::string name, size_t portNumber, NetworkSystem* system) : Module(system), NetworkModule(name, system), NetworkLog(system)
{
	size_t k;
	allPort.clear();
	for(k=0; k<portNumber; k++)
	{
		char nameBuffer[1024];
		snprintf(nameBuffer, sizeof(nameBuffer), "%s's port[%lu]", name.c_str(), k);
		Port* newPort = new Port(std::string(nameBuffer), system);
		newPort->connect(this);
		allPort.push_back(newPort);
	}
	this->pidStart = 0;
	this->syscallIDStart = 0;
	this->defaultInterface = new DefaultSystemCall(this);
	this->running = true;
}

Host::~Host()
{
	size_t k;
	for(k=0; k<allPort.size(); k++)
	{
		allPort[k]->disconnect(this);
		delete allPort[k];
	}
	allPort.clear();
	delete this->defaultInterface;
}

bool Host::isRunning(void)
{
	return this->running;
}

int Host::cleanUp(void)
{
	std::unique_lock<std::mutex> lock(this->getSystem()->getSystemLock(), std::defer_lock);
	lock.lock();
	this->running = false;
	int missing = 0;
	std::list<UUID> syscall_to_wakeup;
	for(auto iter : this->syscallIDToWakeup)
	{
		syscall_to_wakeup.push_back(iter.first);
		print_log(SYSCALL_ERROR, "Unfinished system call at %s", this->getModuleName().c_str());
		missing++;
	}
	for(auto iter : syscall_to_wakeup)
	{
		this->returnSystemCall(defaultInterface, iter, -1);
	}
	lock.unlock();
	return missing;
}

Module::Message* Host::messageReceived(Module* from, Module::Message* message)
{
	Port::Message* portMessage = dynamic_cast<Port::Message*>(message);
	NetworkModule* netModule = dynamic_cast<NetworkModule*>(from);

	if(portMessage != nullptr)
	{
		assert(netModule != nullptr);
		assert(portMessage->type == Port::MessageType::PACKET_FROM_PORT);
		if(this->running == false)
		{
			this->freePacket(portMessage->packet);
		}
		else
		{
			print_log(PACKET_FROM_HOST, "Host [%s] get a packet [size:%lu] from module [%s]",
					this->getModuleName().c_str(), portMessage->packet->getSize(), netModule->getModuleName().c_str());
			//this->freePacket(hostMessage->packet);

			this->sendPacketToModule(nullptr, "Ethernet", portMessage->packet);
		}
		return nullptr;
	}
	Message* hostMessage = dynamic_cast<Message*>(message);
	assert(hostMessage);

	switch(hostMessage->type)
	{
	case PACKET_TRANSFER:
	{
		if(this->running == false)
		{
			this->freePacket(hostMessage->packet);
			break;
		}
		std::string fromName;
		if(hostMessage->packetPass.from == nullptr)
			fromName = "Host";
		else
			fromName = hostMessage->packetPass.from->getHostModuleName();
		hostMessage->packetPass.to->packetArrived(fromName, hostMessage->packetPass.packet);
		break;
	}
	case SYSCALL_CALLED:
	{
		SystemCallApplication* app = dynamic_cast<SystemCallApplication*>(from);
		assert(app != nullptr);

		Interface iface = defaultInterface;
		Domain domain = 0;
		Protocol protocol = 0;
		switch(hostMessage->syscall.param.syscallNumber)
		{
		case SystemCallInterface::SystemCall::SOCKET:
		{
			domain = (Domain)hostMessage->syscall.param.param1_int;
			protocol = (Domain)hostMessage->syscall.param.param3_int;
			break;
		}
		case SystemCallInterface::SystemCall::NSLEEP:
		case SystemCallInterface::SystemCall::GETTIMEOFDAY:
		{
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
		case SystemCallInterface::SystemCall::GETPEERNAME:
		{
			assert(app->pid != -1);
			auto iter = this->pidToProcessInfo.find(app->pid);
			assert(iter != this->pidToProcessInfo.end());

			int fd = hostMessage->syscall.param.param1_int;
			auto nsIter = iter->second.fdToDomain.find(fd);
			assert(nsIter != iter->second.fdToDomain.end());

			domain = nsIter->second.first;
			protocol = nsIter->second.second;
			break;
		}
		default:
			assert(0);
		}


		if(domain == 0 and protocol == 0)
		{
			iface = this->defaultInterface;
		}
		else
		{
			Namespace ns = Namespace(domain, protocol);
			auto iter = namespaceToInterface.find(ns);

			if(iter != namespaceToInterface.end())
			{
				iface = iter->second;
			}
			else
			{
				break;
			}
		}

		UUID curSyscallID = this->syscallIDStart;
		do
		{
			if(syscallIDToApplication.find(curSyscallID) == syscallIDToApplication.end())
			{
				syscallIDStart = curSyscallID + 1;
				syscallIDToApplication.insert(std::pair<UUID, Application>(curSyscallID, app));
				break;
			}
			curSyscallID++;
		}while(curSyscallID != this->syscallIDStart);
		assert(curSyscallID != this->syscallIDStart);
		*(hostMessage->syscall.returnValue) = -1;
		this->syscallIDToWakeup.insert(std::pair<UUID, std::pair<int*,std::condition_variable*>>
				(curSyscallID, std::pair<int*, std::condition_variable*>(hostMessage->syscall.returnValue, hostMessage->syscall.condVar)));

		print_log(SYSCALL_RAISED, "System call[syscall_no:%d, unique_id: %lu] has raised from app[pid:%d] at [%s]",
				hostMessage->syscall.param.syscallNumber, curSyscallID, app->pid, this->getModuleName().c_str());
		iface->systemCallback(curSyscallID, app->pid, hostMessage->syscall.param);

		break;
	}
	default:
		assert(0);
	}

	return nullptr;
}
void Host::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	assert(response == nullptr);
	delete message;
}

void Host::messageCancelled(Module* to, Module::Message* message)
{
	Port::Message* portMessage = dynamic_cast<Port::Message*>(message);
	if(portMessage != nullptr)
	{
		this->freePacket(portMessage->packet);
	}

	Host::Message* hostMessage = dynamic_cast<Host::Message*>(message);
	if(hostMessage != nullptr)
	{
		this->freePacket(hostMessage->packet);
	}

	delete message;
}


size_t Host::getPortCount()
{
	return allPort.size();
}

Port* Host::getPort(size_t portIndex)
{
	assert(portIndex < allPort.size());
	return allPort[portIndex];
}

void Host::sendPacket(size_t portIndex, Packet* packet)
{
	assert(portIndex < allPort.size());
	assert(packet);
	if(this->running == false)
	{
		this->freePacket(packet);
		return;
	}

	Port* port = allPort[portIndex];

	Port::Message* portMessage = new Port::Message;
	portMessage->type = Port::PACKET_TO_PORT;
	portMessage->packet = packet;
	sendMessage(port, portMessage, 0);
}

Host::DefaultSystemCall::DefaultSystemCall(Host* host) : SystemCallInterface(0,0,host), Module(host->getSystem())
{

}
Host::DefaultSystemCall::~DefaultSystemCall()
{

}
Module::Message* Host::DefaultSystemCall::messageReceived(Module* from, Module::Message* message)
{
	Message* defaultMessage = dynamic_cast<Message*>(message);
	assert(defaultMessage != nullptr);

	switch(defaultMessage->type)
	{
	case NANOSLEEP:
	{
		returnSystemCall(defaultMessage->wakeupSyscallID, 0);
		break;
	}
	default:
		assert(0);
	}

	return nullptr;
}

void Host::DefaultSystemCall::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	delete message;
}
void Host::DefaultSystemCall::systemCallback(UUID syscallUUID, int pid, const SystemCallParameter& param)
{
	switch(param.syscallNumber)
	{
	case SystemCallInterface::SystemCall::NSLEEP:
	{
		Message* myMessage = new Message;
		myMessage->type = NANOSLEEP;
		myMessage->wakeupSyscallID = syscallUUID;
		sendMessage(this, myMessage, param.param1_long);
		break;
	}
	case SystemCallInterface::SystemCall::GETTIMEOFDAY:
	{
		Time curTime = this->getSystem()->getCurrentTime();
		struct timeval *tv = (struct timeval *)param.param1_ptr;
		struct timezone *tz = (struct timezone *)param.param2_ptr;

		Time usec = TimeUtil::getTime(curTime, TimeUtil::USEC);
		if(tz != nullptr)
		{
			//Timezone is not implemented yet.
			this->returnSystemCall(syscallUUID, -EINVAL);
		}
		else
		{
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

HostModule::HostModule(std::string name, Host* host)
{
	this->host = host;
	this->name = name;
	host->registerHostModule(name, this);
}
HostModule::~HostModule()
{
	host->unregisterHostModule(name);
}

std::string HostModule::getHostModuleName()
{
	return name;
}

Host* HostModule::getHost()
{
	return host;
}
void HostModule::sendPacket(std::string toModule, Packet* packet)
{
	host->sendPacketToModule(this, toModule, packet);
}

SystemCallInterface::SystemCallInterface(int domain, int protocol, Host* host)
{
	this->host = host;
	this->domain = domain;
	this->protocol = protocol;

	host->registerInterface(this, domain, protocol);
}
SystemCallInterface::~SystemCallInterface()
{

}
/*
	virtual bool openCalled(const SystemCallParameter& param) = 0;
	virtual bool closedCalled(const SystemCallParameter& param) = 0;
	virtual void systemCallback(const SystemCallParameter& param) = 0;
*/


void SystemCallInterface::returnSystemCall(UUID syscallUUID, int val)
{
	host->returnSystemCall(this, syscallUUID, val);
}
int SystemCallInterface::createFileDescriptor(int processID)
{
	return host->createFileDescriptor(this, processID);
}

void SystemCallInterface::removeFileDescriptor(int processID, int fd)
{
	return host->removeFileDescriptor(this, processID, fd);
}


HostModule* Host::findHostModule(std::string name)
{
	auto iter = this->hostModuleMap.find(name);
	if(iter != this->hostModuleMap.end())
		return iter->second;
	return nullptr;
}

void Host::registerHostModule(std::string name, HostModule* hostModule)
{
	this->hostModuleMap.insert(std::pair<std::string, HostModule*>(name, hostModule));
}

void Host::unregisterHostModule(std::string name)
{
	this->hostModuleMap.erase(name);
}

void Host::sendPacketToModule(HostModule* fromModule, std::string toModule, Packet* packet)
{
	HostModule* toHostModule = this->findHostModule(toModule);

	if(toModule.compare("Host") == 0)
	{
		uint8_t my_mac[6];
		packet->readData(6, my_mac, 6);

		int selected_port = 0;
		for(size_t k=0; k<this->allPort.size(); k++)
		{
			uint8_t port_mac[6];
			this->getMACAddr(port_mac, k);
			if(memcmp(port_mac, my_mac, 6) == 0)
			{
				selected_port = (int)k;
				break;
			}
		}
		this->sendPacket(selected_port, packet);
	}
	else if(toHostModule == nullptr)
	{
		print_log(MODULE_ERROR, "No module named [%s] has found. Drop packet.", toModule.c_str());
		this->freePacket(packet);
	}
	else
	{
		Message* hostMessage = new Message;
		hostMessage->type = PACKET_TRANSFER;
		hostMessage->packetPass.from = fromModule;
		hostMessage->packetPass.to = toHostModule;
		hostMessage->packetPass.packet = packet;

		this->sendMessage(this, hostMessage, 0); //DELAY module packet transfer delay
	}
}

void Host::registerInterface(Interface iface, Domain domain, Protocol protocol)
{
	Namespace key(domain, protocol);
	if(namespaceToInterface.find(key) != namespaceToInterface.end())
		assert(0);
	namespaceToInterface.insert(std::pair<Namespace, Interface>(key, iface));
}


void Host::returnSystemCall(Interface iface, UUID syscallUUID, int val)
{
	if(syscallIDToApplication.find(syscallUUID) == syscallIDToApplication.end())
	{
		print_log(NetworkLog::SYSCALL_ERROR, "Invalid System call [%lu] at [%s].",
				syscallUUID, this->getModuleName().c_str());
		//assert(0); now we implement auto-closing unclosed system calls
		return;
	}
	print_log(NetworkLog::SYSCALL_FINISHED, "System call [%lu] at [%s] has finished with return value [%d].",
			syscallUUID, this->getModuleName().c_str(), val);

	auto iter = syscallIDToApplication.find(syscallUUID);
	auto retIter = syscallIDToWakeup.find(syscallUUID);
	Application app = iter->second;

	SystemCallApplication::Message* appMessage = new SystemCallApplication::Message;
	*retIter->second.first = val;
	appMessage->condVar = retIter->second.second;
	appMessage->returnValue = val;
	appMessage->syscallID = syscallUUID;
	appMessage->type = SystemCallApplication::MessageType::SYSCALL_FINISHED;

	this->sendMessage(app, appMessage, 0);

	syscallIDToApplication.erase(iter);
	syscallIDToWakeup.erase(retIter);
}

int Host::createFileDescriptor(Interface iface, int processID)
{
	assert(pidToProcessInfo.find(processID) != pidToProcessInfo.end());
	ProcessInfo& procInfo = pidToProcessInfo.find(processID)->second;
	int start = procInfo.fdStart;
	int current = start;
	//assert(procInfo.fdSet.find(processID) != procInfo.fdSet.end());
	do
	{
		if(procInfo.fdSet.find(current) == procInfo.fdSet.end())
		{
			procInfo.fdSet.insert(current);
			procInfo.fdStart = (current+1) % MAX_FD;
			procInfo.fdToDomain.insert(std::pair<int, Namespace>(current, Namespace(iface->domain, iface->protocol)));
			//found proper fd number
			return current;
		}

		current = (current+1) % MAX_FD;
	}while(start != current);

	print_log(NetworkLog::SYSCALL_ERROR, "Out of FD for process %d.", processID);
	return -1;
}

void Host::removeFileDescriptor(Interface iface, int processID, int fd)
{
	//TIME_WAIT occurs after process termination
	if(pidToProcessInfo.find(processID) != pidToProcessInfo.end())
	{
		ProcessInfo& procInfo = pidToProcessInfo.find(processID)->second;

		procInfo.fdSet.erase(fd);
		procInfo.fdToDomain.erase(fd);
	}
}


int Host::registerProcess(Application app)
{
	int start = pidStart;
	int current = start;
	do
	{
		if(pidToProcessInfo.find(current) == pidToProcessInfo.end())
		{
			ProcessInfo procInfo;
			procInfo.fdStart = 0;
			procInfo.application = app;
			pidToProcessInfo.insert(std::pair<int, ProcessInfo>(current, procInfo));
			pidStart = (current+1) % MAX_PID;
			this->getSystem()->addRunnable(app);
			//found proper fd number
			return current;
		}

		current = (current+1) % MAX_FD;
	}while(start != current);

	print_log(NetworkLog::SYSCALL_ERROR, "Out of PID.");
	return -1;
}



void Host::unregisterProcess(int pid)
{
	auto iter = pidToProcessInfo.find(pid);
	assert(iter != pidToProcessInfo.end());

	for(auto allSyscall : syscallIDToApplication)
	{
		if(allSyscall.second == iter->second.application)
		{
			auto condIter = syscallIDToWakeup.find(allSyscall.first);
			if(condIter != syscallIDToWakeup.end())
			{
				*(condIter->second.first) = -1;
				condIter->second.second->notify_all();
			}
			syscallIDToApplication.erase(allSyscall.first);
		}
	}
	this->getSystem()->delRunnable(iter->second.application);

	pidToProcessInfo.erase(iter);
}

void SystemCallApplication::registerApplication()
{
	this->my_lock_ptr = new std::unique_lock<std::mutex>(this->getSystem()->getSystemLock(), std::defer_lock);
	this->my_lock_ptr->lock();
	assert(this->pid == -1);
	this->pid = this->host->registerProcess(this);
}

void SystemCallApplication::unregisterApplication()
{
	this->host->unregisterProcess(this->pid);
	this->pid = -1;
	this->setRunning(false);
	this->my_lock_ptr->unlock();
	delete this->my_lock_ptr;
	this->my_lock_ptr = nullptr;
}

void SystemCallApplication::__callMain(SystemCallApplication* app)
{
	app->registerApplication();
	std::unique_lock<std::mutex> local_lock(app->initial_barrier_mutex);
	app->initial_barrier_cond.notify_all();
	local_lock.unlock();
	local_lock.release();
	app->E_Main();
	app->unregisterApplication();
}

SystemCallApplication::SystemCallApplication(Host* host) :
		Module(host->getSystem()), NetworkLog(host->getNetworkSystem()), Runnable(host->getSystem(), true),
		my_lock_ptr(nullptr), host(host), pid(-1),
		initial_barrier_mutex()
		//
{
	this->thread = nullptr;
}
SystemCallApplication::~SystemCallApplication()
{
	if(this->thread != nullptr)
	{
		this->thread->join();
		delete this->thread;
		this->thread = nullptr;
	}
}

void SystemCallApplication::initialize()
{
	std::unique_lock<std::mutex> initial_barrier_lock(initial_barrier_mutex, std::defer_lock);
	initial_barrier_lock.lock();
	assert(this->thread == nullptr);
	this->thread = new std::thread(SystemCallApplication::__callMain, this);
	initial_barrier_cond.wait(initial_barrier_lock);
	initial_barrier_lock.unlock();
}

int SystemCallApplication::E_Syscall(const SystemCallInterface::SystemCallParameter& param)
{
	if(!this->host->isRunning())
		return -1;
	std::condition_variable *condVar = new std::condition_variable;

	int ret = 0;

	Host::Message* hostMessage = new Host::Message;
	hostMessage->syscall.param = param;
	hostMessage->type = Host::MessageType::SYSCALL_CALLED;
	hostMessage->syscall.condVar = condVar;
	hostMessage->syscall.returnValue = &ret;
	this->sendMessage(host, hostMessage, 0);
	this->setRunning(false);
	condVar->wait(*this->my_lock_ptr);

	delete condVar;
	return ret;
}

Module::Message* SystemCallApplication::messageReceived(Module* from, Module::Message* message)
{
	Message* appMessage = dynamic_cast<Message*>(message);
	assert(appMessage);

	switch(appMessage->type)
	{
	case SYSCALL_FINISHED:
	{
		appMessage->condVar->notify_one();
		this->setRunning(true);

		break;
	}
	default:
		assert(0);
	}

	return nullptr;
}

void SystemCallApplication::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	assert(response == nullptr);
	delete message;
}

void SystemCallApplication::messageCancelled(Module* to, Module::Message* message)
{
	assert(0);
}

}

