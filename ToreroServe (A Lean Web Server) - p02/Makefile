CXX=g++
CXXFLAGS=-Wall -Wextra -g -O1 -std=c++20 -pthread
LDFLAGS	:= 
LDLIBS	:=

TARGETS	:=	torero-serve

all: $(TARGETS)

%.o: %.cpp %.hpp
	$(CXX) $< -o $@ $(CXXFLAGS) -c

torero-serve: main.cpp torero-serve.cpp ServerSocket.o ClientSocket.o BoundedBuffer.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS)

clean:
	rm -f $(TARGETS) *.o
