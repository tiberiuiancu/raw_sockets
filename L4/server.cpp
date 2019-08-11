#include <cstdio>
#include <cstdlib>       // exit, malloc
#include <cstring>       // memset
#include <pthread.h>
#include <unistd.h>      // usleep

#include <sys/socket.h>  // socket
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // htons, ntohs


#define PROTOCOL 200
#define MAX_CONN 10


typedef struct {
    char *buffer;
    struct sockaddr_in addr;
    size_t len;
    int socketfd;
} ip_packet;

in_addr_t *connections;
pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;

int add_connection(struct sockaddr_in addr) {
    for (int i = 0; i < MAX_CONN; ++i) {
        if (connections[i] == addr.sin_addr.s_addr) {
            return -1;
        }
    }

    for (int i = 0; i < MAX_CONN; ++i) {
        if (connections[i] == 0) {
            connections[i] = addr.sin_addr.s_addr;
            return 1;
        }
    }

    return 0;
}

int remove_connection(struct sockaddr_in addr) {
    for (int i = 0; i < MAX_CONN; ++i) {
        if (connections[i] == addr.sin_addr.s_addr) {
            connections[i] = 0;
            return 1;
        }
    }

    return -1;
}

void* handle_packet(void *args) {

//    // retrieve source address from headers
//    unsigned long addr_long = (unsigned long)
//                              ((unsigned char)(packet[12]) << 24 |
//                              (unsigned char)(packet[13]) << 16 |
//                              (unsigned char)(packet[14]) << 8 |
//                              (unsigned char)(packet[15]));
//
//    // this is the address we got the packet from
//    struct sockaddr_in src;
//    memset(&src, 0, sizeof(struct sockaddr_in));
//    src.sin_addr.s_addr = ntohl(addr_long);
//    src.sin_family = AF_INET;
//
//    // convert source address to ipv4 format
//    char *addr_ipv4 = inet_ntoa(src.sin_addr);
//    printf("%s\n", addr_ipv4);

    ip_packet *packet = (ip_packet *)args;

    printf("%d bytes received from %s: %s\n", (int)packet->len, inet_ntoa(packet->addr.sin_addr), packet->buffer + 20);

    strcpy(packet->buffer, "456\0");
    sendto(packet->socketfd, packet->buffer, strlen(packet->buffer), 0, (struct sockaddr *)&(packet->addr), sizeof(sockaddr));

    pthread_mutex_lock(&conn_lock);
    remove_connection(packet->addr);
    pthread_mutex_unlock(&conn_lock);

    return NULL;
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


    connections = (in_addr_t *)calloc(MAX_CONN, sizeof(in_addr_t));
    // wait for packets
    while (true) {

        ip_packet *packet = (ip_packet *)malloc(sizeof(ip_packet));
        packet->socketfd = socketfd;
        packet->buffer = (char *)malloc(1024 * sizeof(char));
        socklen_t socklen = sizeof(struct sockaddr);
        packet->len = recvfrom(socketfd, packet->buffer, 1024, 0, (struct sockaddr *)&packet->addr, &socklen);

        bool solved = false;
        while (!solved) {
            pthread_mutex_lock(&conn_lock);
            int flag = add_connection(packet->addr);
            if (flag == 1) {
                pthread_t thread_id;
                pthread_create(&thread_id, NULL, handle_packet, packet);
                solved = true;
            } else if (flag == -1) {
                solved = true;
            }

            pthread_mutex_unlock(&conn_lock);
            usleep(100 * 1000); // 100ms
        }

        free(packet);
    }
}