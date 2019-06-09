SOURCE = $(filter-out %win.cpp, $(wildcard *.cpp))
TARGETS = $(patsubst %.cpp, %, $(SOURCE))
INC=-I../src -I../src/elfio/elfio
LIB=-lm
CC = g++
CFLAGS =-std=c++11 -Wall -g
 
all:$(TARGETS)
 
$(TARGETS):%:%.cpp
	$(CC) $< $(CFLAGS) $(INC) ${LIB}  -o $@
 
.PHONY:clean all
clean:
	-rm -rf $(TARGETS)
