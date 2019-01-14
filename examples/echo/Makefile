CXXFLAGS=-Wall -O0 -g -std=c++11 -isystem ../../src
#LDFLAGS=-L../../src 
LIBS=../../src/libesock.a 

all: echo_client echo_server

echo_client: echo_client.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

echo_server: echo_server.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -rf *.o echo_client echo_server
