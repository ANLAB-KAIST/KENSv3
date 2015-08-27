CXX+= -std=c++11
CXXFLAGS= -g -O0 -Wall
CXXFLAGS+= $(EXTRA_CXXFLAGS)

LIB_DIR=../../lib
LIBS+= -L$(LIB_DIR) -le
LIBS+= -pthread
LIBS+= $(EXTRA_LIBS)

INCLUDE_DIR=../../include
INCLUDES+= -I$(INCLUDE_DIR)

CXXFLAGS+= $(INCLUDES)

SRCS_C= $(wildcard *.c)
SRCS_CPP= $(wildcard *.cpp) 
SRCS_CC= $(wildcard *.cc)
SRCS_CXX= $(wildcard *.cxx)
SRCS= $(SRCS_C) $(SRCS_CC) $(SRCS_CPP) $(SRCS_CXX)
OBJS= $(SRCS_C:.c=.o) $(SRCS_CPP:.cpp=.o) $(SRCS_CC:.cc=.o) $(SRCS_CXX:.cxx=.o)
HEADERS= $(wildcard *.h) $(wildcard *.hh) $(wildcard *.hpp) $(wildcard $(INCLUDE_DIR)/*.hpp) $(wildcard $(INCLUDE_DIR)/*/*.hpp) $(wildcard $(INCLUDE_DIR)/*/*/*.hpp)
DEPS= .make.dep
OUTPUT_DIR=../../build

all: $(OUTPUT_DIR)/$(PROGRAM)

$(OUTPUT_DIR)/$(PROGRAM): $(LIB_DIR) $(DEPS) $(OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(EXTRA_OBJS) -o $@ $(LIBS)

clean:
	rm -f $(OUTPUT_DIR)/$(PROGRAM) *.o $(OUTPUT_DIR)/*.pcap $(OBJS)

$(DEPS): $(SRCS) $(HEADERS)
	@$(CXX) $(CXXFLAGS) -MM $(SRCS) > $(DEPS);
	
-include $(DEPS)

depend: $(DEPS)

.PHONY: all clean depend
