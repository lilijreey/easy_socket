#CXX = g++-8
CXXFLAGS=-Wall -std=c++11 -O0 -g -ggdb

all: libesock.a

libesock.a : esock_engine.o esock_tcp_listener.o detail/esock_sockpool.o detail/esock_error.o buffer/esock_ring_buff.o
	ar crv $@ $^

clean:
	rm -rf *.o *.a detail/*.o

fmt:
	@@git ls-files  | \grep -E "*.hpp|*.cpp"  | xargs clang-format -i 

check:
	cppcheck --platform=unix64 --std=c++11 --std=posix --enable=all --template='gcc' *.cpp *.hpp detail/*.cpp detail/*.hpp
