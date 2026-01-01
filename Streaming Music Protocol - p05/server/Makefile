CXX = g++
CXXFLAGS=-Wall -Wextra -g -O1 -std=c++20

LIB_OBJS = ChunkedDataSender.o ConnectedClient.o SongLibrary.o protocol.o networking.o

TARGETS = music-server


all: $(TARGETS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $<

music-server: music-server.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS) $(LIB_OBJS)
