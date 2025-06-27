#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

extern void caesarEncrypt(char*, int);
extern void caesarDecrypt(char*, int);
extern void xorEncryptDecrypt(char*, char);
extern void vigenereEncrypt(char*, const char*);
extern void vigenereDecrypt(char*, const char*);
extern int generatePrivateKey();
extern int generatePublicKey(int, int, int);
extern int computeSharedKey(int, int, int);
extern void addNode(void*, void*, int, const char*);
extern void freeBlockchain(void*);
extern void loadConfig(char*, int*, char*, char*);

typedef struct BlockchainNode BlockchainNode;

#define UDP_PORT 9876

enum Cipher { CAESAR, XOR, VIGENERE };

int main(int argc, char *argv[]) {
    char buffer[1024];
    int tcp_fd, udp_fd, nBytes;
    struct sockaddr_in udp_addr, server_addr;
    struct hostent *hostPtr;
    socklen_t addr_size;
    BlockchainNode* head = NULL;
    BlockchainNode* tail = NULL;

    enum Cipher currentCipher;
    int caesarKey;
    char xorKey;
    char vigenereKey[50];

    if (argc != 3) {
        printf("Usage: vpn_client <host> <port>\n");
        exit(-1);
    }

    /* Carrega configuração atual (algoritmo e keys) */
    char cipherName[20];
    loadConfig(cipherName, &caesarKey, &xorKey, vigenereKey);

    if (strcmp(cipherName, "CAESAR") == 0)
        currentCipher = CAESAR;
    else if (strcmp(cipherName, "XOR") == 0)
        currentCipher = XOR;
    else
        currentCipher = VIGENERE;

    udp_fd = socket(PF_INET, SOCK_DGRAM, 0);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(udp_fd, (struct sockaddr*)&udp_addr, sizeof(udp_addr));
    addr_size = sizeof(udp_addr);

    hostPtr = gethostbyname(argv[1]);
    if (hostPtr == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    bzero((void*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr = *((struct in_addr *)hostPtr->h_addr_list[0]);

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(tcp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    int p = 23, g = 5;
    int privKey = generatePrivateKey();
    int myPublic = generatePublicKey(privKey, g, p);
    write(tcp_fd, &myPublic, sizeof(myPublic));

    int otherPublic;
    read(tcp_fd, &otherPublic, sizeof(otherPublic));

    int sharedKey = computeSharedKey(otherPublic, privKey, p);
    printf("VPN Client shared key: %d\n", sharedKey);

    addNode(&head, &tail, 1, "VPN Client connection established");

    while (1) {
        printf("Waiting UDP message from ProgUDP1...\n");
        nBytes = recvfrom(udp_fd, buffer, sizeof(buffer), 0,
                          (struct sockaddr*)&udp_addr, &addr_size);
        buffer[nBytes] = '\0';

        printf("Original message: %s\n", buffer);

        if (currentCipher == CAESAR)
            caesarEncrypt(buffer, sharedKey);
        else if (currentCipher == XOR)
            xorEncryptDecrypt(buffer, xorKey);
        else if (currentCipher == VIGENERE)
            vigenereEncrypt(buffer, vigenereKey);

        printf("Encrypted message: %s\n", buffer);
        write(tcp_fd, buffer, strlen(buffer)+1);

        nBytes = read(tcp_fd, buffer, sizeof(buffer));
        buffer[nBytes] = '\0';

        if (currentCipher == CAESAR)
            caesarDecrypt(buffer, sharedKey);
        else if (currentCipher == XOR)
            xorEncryptDecrypt(buffer, xorKey);
        else if (currentCipher == VIGENERE)
            vigenereDecrypt(buffer, vigenereKey);

        printf("VPN Client received decrypted: %s\n", buffer);
    }

    close(tcp_fd);
    close(udp_fd);
    freeBlockchain(&head);
    return 0;
}
