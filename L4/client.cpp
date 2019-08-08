#include <cstdio>        // I/O

#include <cstring>       // strlen
#include <cstdlib>       // malloc

#include <sys/socket.h>  // socket
#include <netinet/in.h>  // sockaddr_in
#include <netinet/in.h>  // IPPROTO_TCP
#include <arpa/inet.h>   // inet_aton
#include <errno.h>       // perror


#define PROTOCOL 200

int main(int argc, char **argv) {

    // create L4 socket
    int socketfd = socket(AF_INET, SOCK_RAW, PROTOCOL);
    if (socketfd == -1) {
        printf("failed to create socket. run as root\n");
        exit(-1);
    } else {
        printf("socket created successfully\n");
    }

    // get the server address from the arguments
    if (argc < 2) {
        printf("server address must be provided\n");
        exit(EXIT_FAILURE);
    }
    char *server_addr = (char *)malloc( (strlen(argv[1]) + 1) * sizeof(char) );
    strcpy(server_addr, argv[1]);

    // fill in destination (server) address in the sockaddr_in struct
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if (inet_aton(server_addr, &dest_addr.sin_addr) == 0) {
        printf("provided server address (%s) is invalid", server_addr);
        exit(EXIT_FAILURE);
    }

    // OPTIONAL: bind socket to specific interface
    const char *interface = "enp2s0";
    if (setsockopt(socketfd, SOL_SOCKET, SO_BINDTODEVICE, interface, 6) < 0) {
        printf("setsockopt failed\n");
        exit(EXIT_FAILURE);
    }

    // declare and populate buffer
    char *buffer = (char *)malloc(1024 * sizeof(char));
    if (argc < 3)
        strcpy(buffer, "123");
    else
        strcpy(buffer, argv[2]);

    // send packet
    if (sendto(socketfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&dest_addr,
            sizeof(struct sockaddr_in)) < 0) {
        printf("failed to send packet: sendto err\n");
        exit(EXIT_FAILURE);
    } else {
        printf("packet sent\n");
    }

    socklen_t addr_len = sizeof(dest_addr);

    int len = recvfrom(socketfd, buffer, 1024, 0, (struct sockaddr *)&dest_addr,
             (socklen_t *)&addr_len);
    if (len < 0) {
        perror("recvfrom failed");
    } else {
        printf("%d bytes received: ", len);
        for (int i = 0; i < len; ++i) {
            printf("%c", buffer[i]);
        }
        printf("\n");
    }

    return 0;
}