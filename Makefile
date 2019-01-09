all: main

main: main.cpp  detail/esock_sockpool.cpp esock_engine.cpp
	clang++ -g -ggdb -O0 -std=c++11 -isystem . $^ -o $@
