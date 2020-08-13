
TARGET=ut

GCC = g++
CFLAGS=--std=c++11 -m64 -g -Wall -no-pie -fno-stack-protector

SUBDIR = . mocks fixtures
SOURCE=$(foreach dir, $(SUBDIR), $(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(SOURCE))
DEPS = $(patsubst %.o, %.d, $(OBJS))

INCLUDES=-I../include -Imocks -Ifixtures -I../../src -I../../../../src
LIBS =-lgtest -lgmock -lm -lpthread

$(TARGET): $(OBJS)
	$(GCC) $(CFLAGS) $(OBJS) -L../lib $(LIBS) -o $@
%.o: %.cpp
	$(GCC) $(CFLAGS) $(INCLUDES) -c -MMD $< -o $@
-include $(DEPS)
clean:
	rm $(OBJS) $(TARGET)
