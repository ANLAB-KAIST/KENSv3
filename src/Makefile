ARCHIVE= libe.a

CXX+= -std=c++11
CXXFLAGS= -g -O0 -Wall
SOLUTION_CXXFLAGS= -O3 -Wall -funit-at-a-time -fvisibility=hidden \
 -fvisibility-inlines-hidden -s -finline-limit=1000000000 \
 -fmerge-all-constants -fgcse -fgcse-lm -fgcse-sm -fgcse-las -fgcse-after-reload\
 --param max-inline-recursive-depth=10000 --param max-inline-insns-single=1000000000 \
 --param large-function-insns=1000000000 --param large-function-growth=1000000000 \
 --param max-inline-insns-recursive=1000000000 --param max-inline-insns-auto=1000000000 \
 --param max-grow-copy-bb-insns=1000000000 --param max-goto-duplication-insns=1000000000 \
 --param max-gcse-memory=1000000000 --param max-inline-insns-recursive=1000000000 \
 --param inline-unit-growth=1000000000 --param large-unit-insns=1000000000 \
 --param max-inline-insns-recursive-auto=1000000000 --param max-inline-recursive-depth-auto=10000

#not available with native mac gcc
#-finline-limit=1000000000
#-fgcse -fgcse-lm -fgcse-sm -fgcse-las -fgcse-after-reload

#not available:
#--param salias-max-implicit-fields=1000
#--param max-gcse-passes=10
#-freorder-blocks-and-partition
#--param inline-call-cost=100

#for distribution version of solution binaries

INCLUDE_DIR=../include
INCLUDES+= -I$(INCLUDE_DIR)
LIBS+= -pthread

CXXFLAGS+= $(INCLUDES) $(EXTRA_CXXFLAGS)
SOLUTION_CXXFLAGS+= $(INCLUDES)

LIB_SRC_DIR=.
SOLUTION_SRC= $(wildcard $(LIB_SRC_DIR)/*Solution.cpp) $(wildcard $(LIB_SRC_DIR)/*/*Solution.cpp) $(wildcard $(LIB_SRC_DIR)/*/*/*Solution.cpp)
LIB_SRCS= $(wildcard $(LIB_SRC_DIR)/*.cpp) $(wildcard $(LIB_SRC_DIR)/*/*.cpp) $(wildcard $(LIB_SRC_DIR)/*/*/*.cpp)
LIB_SRCS := $(filter-out $(SOLUTION_SRC), $(LIB_SRCS))
HEADERS= $(wildcard $(INCLUDE_DIR)/*.hpp) $(wildcard $(INCLUDE_DIR)/*/*.hpp) $(wildcard $(INCLUDE_DIR)/*/*/*.hpp)
DEPS= .make.dep
OUTPUT_DIR=../lib

OBJS= $(LIB_SRCS:.cpp=.o)
SOLUTION_OBJS= $(SOLUTION_SRC:.cpp=.o)

all: $(OUTPUT_DIR)/$(ARCHIVE) $(SOLUTION_OBJS)

$(OUTPUT_DIR)/$(ARCHIVE): $(OBJS)
	$(AR) rs $@ $(OBJS)

clean:
	rm -f $(OUTPUT_DIR)/$(ARCHIVE) *.o $(LIB_SRC_DIR)/*.o $(LIB_SRC_DIR)/*/*.o $(LIB_SRC_DIR)/*/*/*.o $(OBJS)
	
%Solution.o: %Solution.cpp
	$(CXX) $(SOLUTION_CXXFLAGS) -c $< -o $@

$(DEPS): $(LIB_SRCS) $(SOLUTION_SRC) $(HEADERS)
	@$(CXX) $(CXXFLAGS) -MM $(LIB_SRCS) > $(DEPS);
	@$(CXX) $(SOLUTION_CXXFLAGS) -MM $(SOLUTION_SRC) >> $(DEPS);

#@$(CXX) $(SOLUTION_CXXFLAGS) -MM $(SOLUTION_SRC) >> $(DEPS);
	
-include $(DEPS)

depend: $(DEPS)

.PHONY: all clean depend
