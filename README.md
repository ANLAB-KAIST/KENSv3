# KENSv3 (KAIST Educational Network System)
====

# What is KENSv3?
## KENSv3 is an educational framework.
KENSv3 provides useful template for building prototypes of
Ethernet/ARP/IP/TCP and many other network layers.
Also, it contains automated grading system to verify the completeness of each implementation.
There are solution binaries in KENSv3 which enables incremental building of network functions.

## KENSv3 is a network simulator
Virtual network devices of KENSv3 simulates queuing effect of switched network.
So it can be used for precise network simulations like ns2.
All packets are recorded as pcap log files which can be analyzed by Wireshark or other analysis tools.

## KENSv3 is a virtual execution environment
KENSv3 can execute actual network application source written in C.
The system call requests from applications are linked with proper network layer.

#How to run KENSv3?

##Building KENSv3
1. Check that your compiler supports C++11 and GNU extension (for gtest). 
(At least 4.8.2 for gcc, LLVM 6.0 for MAC c++)
```bash
g++ -std=c++11
g++ -std=gnu++11
```

2. Check that gtest (Google Test) library is installed in your system.
```csharp
#include <gtest/gtest.h>
```
We also need pthread library (or other implementation for C++11 Threading library).
```bash
g++ -std=gnu++11 -lgtest -lpthread
```
To build documentation, you need Doxygen.

3. Build
```bash
#cd KENSv3
make depend # dependancy check
#header changes will trigger rebuilding of affected source

make all
make doxygen # for making documentation (need doxygen)
```

Solution binary for your architecture is needed for building KENS.
Currently, we support Cygwin 64bit, Linux 64bit, and Mac OS 64bit.

##Running KENSv3
In build directory, testTCP binary will be created after building KENS.
```bash
./testTCP # run all tests
./testTCP --getst_filter="TestEnv_Reliable.TestOpen" # run specific test
```
See Google Test documentation for further usage.

##Running KENSv3 using solution binary bundle
Defining symbol RUN_SOLUTION will build testTCP in solution only mode.
You have to clean and rebuild the whole project.
```bash
#cd KENSv3
export EXTRA_CXXFLAGS="-DRUN_SOLUTION"
make all
```

#How to do KENSv3 project?

##Template source
In TestTCP folder, there are one source file and one header file.
You are allowed to modify TCPAssignment.cpp and TCPAssignment.hpp.
Also, you can add member fields to "class TCPAssignment".
Any source files in TestTCP folder are automatically included in the build path.

##What you have to do?
1. Handling system calls
See the documentation for how to handle with blocking system calls.

2. Context management
You have to manage a global context (usually member variables of TCPAssignment).
Also, you have to split incomming packets and system call requests.
Use IP address and port number for classifying packets,
and pid(process id) and fd(file descriptor number) for classifying application sockets.

Until here is test_part1.

3. Handshaking
Implement 3-way handshaking for connect/accept and
4-way handshaking for close.

Until here is test_part2.

3. Flow control
You may block read/write calls if you have no data from TCP window or you TCP window is full.
Handle with read/write and generate/process proper TCP packets for data transmission.

Until here is test_part3.

4. Recovery
You have to handle with losses, reorders, and packet corruption.

5. Congestion control
Implement AIMD congestion control mechanism for test_part4.
You can observe convergence graph from pcap logs.
If you did not implement congestion control algorithm, you may not pass test_part4.
There would be retransmission storms and applications may not finish in time.

Until here is test_part4.