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

memcheck: echo_server 
	valgrind --tool=memcheck  --track-origins=yes --log-file=x.log --leak-check=full --error-limit=no  --show-leak-kinds=all ./echo_server 
