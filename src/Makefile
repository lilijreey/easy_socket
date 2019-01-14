CXX = g++
CXXFLAGS=-Wall -std=c++11 -g -ggdb

all: libesock.a

libesock.a : esock_engine.o esock_tcp_listener.o detail/esock_sockpool.o
	ar crv $@ $^

clean:
	rm -rf *.o *.a detail/*.o

format:
	clang-format -style=../clang-format.conf *.cpp

check:
	cppcheck --platform=unix64 --std=c++11 --std=posix --enable=all --template='gcc' *.cpp *.hpp detail/*.cpp detail/*.hpp
