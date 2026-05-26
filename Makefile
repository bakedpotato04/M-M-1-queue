## This is a simple Makefile

# Define what compiler to use and the flags.
CC=cc
CXX=g++
CCFLAGS= -g -std=c++23 -Wall -Werror


all: hw3_solution

# Compile all .cpp files into .o files
# % matches all (like * in a command)
# $< is the source file (.cpp file)
%.o : %.cpp
	$(CXX) -c $(CCFLAGS) $<


hw3_solution: hw3_solution.o
	$(CC) -o hw3_solution hw3_solution.o -lm -lstdc++


clean:
	rm -f *.o hw3_solution
