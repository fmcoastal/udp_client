
all: test_loopback so_type hostinfo getsockopt setsockopt socket_server_tcp socket_client_tcp socket_client_udp socket_server_udp  socket1 socket2 socket3 socket4 socket5

CCFLAGS=-c -g -D_GNU_SOURCE -Wall

test_loopback:   test_loopback.o
	gcc test_loopback.o -o test_loopback

test_loopback.o: test_loopback.c
	gcc  $(CCFLAGS) -o test_loopback.o test_loopback.c



so_type:   so_type.o
	gcc so_type.o -o so_type

so_type.o: so_type.c
	gcc  $(CCFLAGS) -o so_type.o so_type.c




setsockopt:   setsockopt.o
	gcc setsockopt.o -o setsockopt

setsockopt.o: setsockopt.c
	gcc  $(CCFLAGS) -o setsockopt.o setsockopt.c



getsockopt:   getsockopt.o
	gcc getsockopt.o -o getsockopt

getsockopt.o: getsockopt.c
	gcc  $(CCFLAGS) -o getsockopt.o getsockopt.c


hostinfo:   hostinfo.o
	gcc hostinfo.o -o hostinfo

hostinfo.o: hostinfo.c
	gcc  $(CCFLAGS) -o hostinfo.o hostinfo.c


socket_server_tcp:   socket_server_tcp.o
	gcc socket_server_tcp.o -o socket_server_tcp

socket_server_tcp.o: socket_server_tcp.c
	gcc  $(CCFLAGS) -o socket_server_tcp.o  socket_server_tcp.c



socket_client_tcp:   socket_client_tcp.o
	gcc socket_client_tcp.o -o socket_client_tcp

socket_client_tcp.o: socket_client_tcp.c
	gcc  $(CCFLAGS) -o socket_client_tcp.o  socket_client_tcp.c


socket_server_udp:   socket_server_udp.o
	gcc socket_server_udp.o -o socket_server_udp

socket_server_udp.o: socket_server_udp.c
	gcc  $(CCFLAGS) -o socket_server_udp.o  socket_server_udp.c


socket:   socket.o
	gcc socket.o -o socket

socket.o: socket.c
	gcc  $(CCFLAGS) -o socket.o  socket.c




socket1:   socket1.o
	gcc socket1.o -o socket1

socket1.o: socket1.c
	gcc $(CCFLAGS) -o socket1.o  socket1.c


socket2:   socket2.o
	gcc socket2.o -o socket2

socket2.o: socket2.c
	gcc $(CCFLAGS) -o socket2.o  socket2.c


socket3:   socket3.o
	gcc socket3.o -o socket3

socket3.o: socket3.c
	gcc $(CCFLAGS) -o socket3.o  socket3.c

socket4:   socket4.o
	gcc socket4.o -o socket4

socket4.o: socket4.c
	gcc $(CCFLAGS) -o socket4.o  socket4.c

socket5:   socket5.o
	gcc socket5.o -o socket5

socket5.o: socket5.c
	gcc $(CCFLAGS) -o socket5.o  socket5.c





clean:
	rm -rf *.o
	rm -rf so_type
	rm -rf setsockopt
	rm -rf getsockopt
	rm -rf hostinfo
	rm -rf socket_client_udp
	rm -rf socket_server_udp
	rm -rf socket_client_tcp
	rm -rf socket_server_tcp
	rm -rf socket1
	rm -rf socket2
	rm -rf socket3
	rm -rf socket4
	rm -rf socket5

