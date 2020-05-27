# udp_client


EVOL: BE careful you do not use any network address currenly assigned inside 
    the corporation. You may end up sending traffic out on the corportate 
    network as opposed to your closed connection 

Setup:
     -----------------------                     -----------------------  
    |                       |                   |                       | 
    | Server                |                   | Client                |   
    |                       |                   |                       | 
    | p5p1                  |                   | p5p2                  | 
    | ip:   192.168.2.10/24 |                   | ip:   192.168.2.20/24 | 
    | Port: 50020           |===================| Port: xxxx            | 
    |                       |                   |                       | 
    ------------------------                    ------------------------                             

HELP:

    enter socket_client_udp -h for instructions

    tcpdump is your friend to figure out what is going on :-)

Example: 
./socket_client_udp -s 192.168.2.10 -p 50020 -b 192.168.2.20 -f packetdata


Notes Route:

   If you have two machines plugged back together, you may need to add
   manual routing information so the packet goes out the correct inteface

      route add -net 192.168.2.10/24 dev p5p1

      route del 192.168.2.10/24 dev p5p1

Notes: ARP:

   If the machine was reset, and your interface will not respond to an ARP
   request, you may need to re-enter mac address for target IP address

        arp -s 192.168.2.20 00:03:fb:ac:2A:20


Notes: capturing PCAP file.

# TCP dump acutally captures data from the Ethernet port. 
# tcpdump -w payload.pcap -i eth0
#
#
#  http://xmodulo.com/2013/05/how-to-capture-and-replay-network-traffic-on-linux.html
#
#
tcpreplay --intf1=eth0 payload.pcap

[root@rover udp_client]# ./playpcap  -i eth0  -r /depot/udp_client/payload.pcap
Failed to open /depot/udp_client/payload.pcap
[root@rover udp_client]# ls
a.map  Makefile  mkaddr.c  mkaddr.h  packetdata  payload.pcap  playpcap  save  socket_client_udp  socket_client_udp.c  socket_client_udp.o  x

In wireshark,  File needs to be saved ad Redhat 6.1 tcpdump - libpcap 


[root@rover udp_client]# ./playpcap  -i eth0  -r /depot/udp_client/payload.pcap
Sent 1 packets in 0 seconds.
[root@rover udp_client]# vim x



