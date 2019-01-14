CXXFLAGS=-Wall -g -std=c++11 -isystem ../../src
LDFLAGS=-L../../src 
LIBS=libesock.a 

all: echo_client

echo_client: echo_client.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

