.PHONY: clean
all: clean client server
CFLAGS = -Wall -g -Og -Wl,-wrap=sendto -Wl,-wrap=recvfrom
DEBUG_OBJ = unrel_sendto.o
LIBS = -lrt

server: timer.cpp server.cpp udpsocket.o Packets.o GBNServer.cpp $(DEBUG_OBJ) $(LIBS)
	g++  $(CFLAGS) $^ -o $@

client: client.cpp udpsocket.o Packets.o StopWaitClient.cpp $(DEBUG_OBJ) $(LIBS)
	g++ $(CFLAGS) $^ -o $@

%.o: %.cpp
	g++ $(CFLAGS) -c $^ -o $@

clean:
	rm -f client server *.o
