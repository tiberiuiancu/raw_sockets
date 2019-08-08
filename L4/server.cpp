#include <cstdio>
#include <cstdlib>       // exit, malloc
#include <cstring>       // memset

#include <sys/socket.h>  // socket
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // htons, ntohs



#define PROTOCOL 200


sockaddr_in handle_packet(const char *packet) {

    // retrieve source address from headers
    unsigned long addr_long = (unsigned long)
                              ((unsigned char)(packet[12]) << 24 |
                              (unsigned char)(packet[13]) << 16 |
                              (unsigned char)(packet[14]) << 8 |
                              (unsigned char)(packet[15]));

    // this is the address we got the packet from
    struct sockaddr_in src;
    memset(&src, 0, sizeof(struct sockaddr_in));
    src.sin_addr.s_addr = ntohl(addr_long);
    src.sin_family = AF_INET;

    // convert source address to ipv4 format
    char *addr_ipv4 = inet_ntoa(src.sin_addr);
    printf("%s\n", addr_ipv4);

    return src;
}


int main() {

    // create socket
    int socketfd = socket(AF_INET, SOCK_RAW, PROTOCOL);
    if (socketfd == -1) {
        printf("socket could not be created. run as root\n");
        exit(EXIT_FAILURE);
    } else {
        printf("socket created successfully\n");
    }

    // create buffer
    char *buffer = (char *)malloc(1024 * sizeof(char));

    // wait for packets
    while (true) {

        int packet_len = recv(socketfd, buffer, 1024, 0);
        printf("%d bytes received from ", packet_len);

        struct sockaddr_in client_addr = handle_packet(buffer);

        strcpy(buffer, "456");
        if (sendto(socketfd, buffer, 3, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in)) < 0) {
            printf("failed to send message back\n");
            exit(EXIT_FAILURE);
        } else {
            printf("packet sent\n");
        }
    }
}