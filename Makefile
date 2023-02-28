all: main

main: main.cpp proxy.cpp proxy.h utils.cpp utils.h client.h client.cpp server.h server.cpp cache.h cache.cpp
	g++ -g  -o main main.cpp proxy.cpp proxy.h utils.cpp utils.h client.h client.cpp server.h server.cpp cache.h cache.cpp -pthread

.PHONY:
	clean
clean:
	rm -rf *.o main
