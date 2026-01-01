CC=g++
CFLAGS=-O1 -g -Wall -Wextra -std=c++20

TARGETS = sender receiver

RDT_LIB_OBJS = UnreliableSocket.o ReliableSocket.o RDTSegment.o rdt_time.o logging.o

all: $(TARGETS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^

sender: sender.cpp $(RDT_LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

receiver: receiver.cpp $(RDT_LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS) $(RDT_LIB_OBJS)
