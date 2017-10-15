# Compiler and Compile options.
CC = g++
CXXFLAGS = -g -Wall -Wextra -Wpedantic -std=c++11 -ltcmalloc_minimal
# CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++11 -ltcmalloc_minimal -O3

# Macros specifying path for compile.

# SRCS := $(wildcard src/*.cc)
SRCS := $(wildcard src/*[^(_test)].cc) # exclue test sources
OBJS := $(SRCS:.cc=.o)
# BIN = ./bin/
BIN = ./
INC = ./include/
LIB = ./lib/ -lpthread

# Pre-Processor.
CPPFLAGS += -I$(INC)

# Compile command.
TARGET = run
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(BIN)$(TARGET) $(OBJS) -L$(LIB)

# Delete binary & object files.
clean:
	rm $(BIN)$(TARGET) $(OBJS) thread*.txt

tc:
	rm thread*.txt
	rm marking/thread*.txt
