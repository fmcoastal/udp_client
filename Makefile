
all:  socket_client_udp 

CCFLAGS = -c -g -O -D_GNU_SOURCE -Wall  -pthread
LDFLAGS = -Wl,-v -Wl,-Map=a.map -Wl,--cref -Wl,-t -lpthread -pthread
ARFLAGS = -rcs

CC = gcc
LD = gcc
AR = ar


socket_client_udp: socket_client_udp.o
	$(LD) $(LDFLAGS) -o socket_client_udp  socket_client_udp.o

socket_client_udp.o: socket_client_udp.c
	$(CC)  $(CCFLAGS) -o socket_client_udp.o  socket_client_udp.c


clean:
	rm -rf *.o
	rm -rf *.map
	rm -rf so_type
	rm -rf socket_client_udp

