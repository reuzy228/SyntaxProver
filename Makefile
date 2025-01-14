# Project name
PROJECT = pc-solver

# Compiler flags
CXX = g++
CFLAGS = -O3 -Wall -Wextra -pedantic -std=c++20
#CFLAGS = -O0 -g -fsanitize=leak -Wall -Wextra -pedantic -std=c++20

# Source files
SRCS = $(wildcard src/math/ast.cpp src/math/helper.cpp src/solver/solver.cpp src/math/rules.cpp src/parser/parser.cpp src/main.cpp)
OBJS = $(SRCS:.cpp=.o)

# Include directories
INCLUDES = -I.

.PHONY: all clean

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(PROJECT)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	find . -name '*.o' -xtype f -exec rm {} +
	find . -name '$(PROJECT)' -xtype f -exec rm {} +

# Default target
default: all
