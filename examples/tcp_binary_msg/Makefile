CXXFLAGS=-Wall -O0 -g -std=c++11 -isystem ../../src
LIBS=../../src/libesock.a 

all: bin_client bin_server

bin_client: bin_client.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

bin_server: bin_server.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -rf *.o bin_client bin_server

memcheck: bin_server 
	valgrind --tool=memcheck  --track-origins=yes --log-file=x.log --leak-check=full --error-limit=no  --show-leak-kinds=all ./bin_server 
