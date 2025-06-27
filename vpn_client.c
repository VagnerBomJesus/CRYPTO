#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "crypto.h"
#include "blockchain.h"


#define UDP_PORT 9876

void erro(char *msg);

int main(int argc, char *argv[]) {
    char buffer[1024];
    int tcp_fd, udp_fd, nBytes;
    struct sockaddr_in udp_addr, server_addr;
    struct hostent *hostPtr;
    socklen_t addr_size;
    BlockchainNode* head = NULL;
    BlockchainNode* tail = NULL;

    if (argc != 3) {
        printf("Uso: vpn_client <host VPN server> <port TCP>\n");
        exit(-1);
    }

    // Criação do socket UDP para receber dados do ProgUDP1
    udp_fd = socket(PF_INET, SOCK_DGRAM, 0);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(udp_fd, (struct sockaddr *) &udp_addr, sizeof(udp_addr));
    addr_size = sizeof(udp_addr);

    // Criação da ligação TCP ao VPN Server
    if ((hostPtr = gethostbyname(argv[1])) == NULL)
        erro("host");

    bzero((void *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    server_addr.sin_port = htons(atoi(argv[2]));

    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        erro("socket TCP");

    if (connect(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        erro("connect TCP");

    // Diffie-Hellman
    int p = 23, g = 5;
    int privKey = generatePrivateKey();
    int myPublic = generatePublicKey(privKey, g, p);
    write(tcp_fd, &myPublic, sizeof(myPublic));

    int otherPublic;
    read(tcp_fd, &otherPublic, sizeof(otherPublic));

    int sharedKey = computeSharedKey(otherPublic, privKey, p);
    printf("Chave simétrica partilhada: %d\n", sharedKey);

    addNode(&head, &tail, 1, "VPN Client established connection");

    while (1) {
        printf("A aguardar mensagem UDP do ProgUDP1...\n");
        nBytes = recvfrom(udp_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&udp_addr, &addr_size);
        buffer[nBytes] = '\0';

        caesarEncrypt(buffer, sharedKey);

        write(tcp_fd, buffer, strlen(buffer)+1);

        nBytes = read(tcp_fd, buffer, sizeof(buffer));
        buffer[nBytes] = '\0';

        caesarDecrypt(buffer, sharedKey);

        printf("Recebido do VPN Server (depois de decifrado): %s\n", buffer);
    }

    close(tcp_fd);
    close(udp_fd);
    freeBlockchain(&head);

    return 0;
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}
