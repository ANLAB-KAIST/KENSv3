TCP_SOLUTION=

OS := $(shell uname)
ifneq (,$(findstring CYGWIN,$(OS)))
TCP_SOLUTION=../../solution/E_TCPSolution_cygwin_amd64.o
else
ifneq (,$(findstring Linux,$(OS)))
GCCVERSION := $(shell expr `gcc -dumpversion | cut -f1 -d.`)
TCP_SOLUTION=../../solution/E_TCPSolution_linux_amd64_gcc$(GCCVERSION).o
else
ifneq (,$(findstring Darwin,$(OS)))
TCP_SOLUTION=../../solution/E_TCPSolution_darwin_amd64.o
else
TCP_SOLUTION=not_supported_os.o
endif   
endif
endif