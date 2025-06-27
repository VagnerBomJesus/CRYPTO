#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "crypto.h"
#include "blockchain.h"


#define UDP_PORT 9877
#define SERVER_PORT 9000

void process_client(int client_fd);
void erro(char *msg);

int main() {
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;

    bzero((void *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erro("socket");

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        erro("bind");

    if (listen(fd, 5) < 0)
        erro("listen");

    client_addr_size = sizeof(client_addr);
    while (1) {
        client = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
        if (client > 0) {
            if (fork() == 0) {
                close(fd);
                process_client(client);
                exit(0);
            }
            close(client);
        }
    }
}

void process_client(int client_fd) {
    int udp_fd, nBytes;
    char buffer[1024];
    struct sockaddr_in udp_addr;
    socklen_t addr_size;
    BlockchainNode* head = NULL;
    BlockchainNode* tail = NULL;

    udp_fd = socket(PF_INET, SOCK_DGRAM, 0);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(udp_addr.sin_zero, '\0', sizeof udp_addr.sin_zero);
    addr_size = sizeof(udp_addr);

    // Diffie-Hellman
    int p = 23, g = 5;
    int myPrivate = generatePrivateKey();
    int clientPublic;
    read(client_fd, &clientPublic, sizeof(clientPublic));
    int myPublic = generatePublicKey(myPrivate, g, p);
    write(client_fd, &myPublic, sizeof(myPublic));

    int sharedKey = computeSharedKey(clientPublic, myPrivate, p);
    printf("Chave partilhada no VPN Server: %d\n", sharedKey);

    addNode(&head, &tail, 1, "VPN Server connection established");

    while (1) {
        nBytes = read(client_fd, buffer, sizeof(buffer));
        buffer[nBytes] = '\0';

        caesarDecrypt(buffer, sharedKey);
        printf("VPN Server recebeu (decifrado): %s\n", buffer);

        // Envia para ProgUDP2 via UDP
        sendto(udp_fd, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&udp_addr, addr_size);

        // Para simplificar, ecoa a mesma mensagem de volta
        caesarEncrypt(buffer, sharedKey);
        write(client_fd, buffer, strlen(buffer)+1);
    }

    close(client_fd);
    close(udp_fd);
    freeBlockchain(&head);
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}
