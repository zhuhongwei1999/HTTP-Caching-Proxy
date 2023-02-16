all: main

main: main.cpp proxy.cpp proxy.h utils.cpp utils.h client.h client.cpp
	g++ -g -o main main.cpp proxy.cpp proxy.h utils.cpp utils.h client.h client.cpp

.PHONY:
	clean
clean:
	rm -rf *.o main
