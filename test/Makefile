all: test_buffer.cpp
	g++ -std=c++11 -g -O0 -ggdb $^  -o test  -lgtest -lgtest_main -lpthread ../src/libesock.a
	./test
