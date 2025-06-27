#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * ProgUDP1 (udp_client)
 * Envia mensagens UDP ao VPN Client.
 */
int main() {
    int sock;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    /* Cria socket UDP */
    sock = socket(PF_INET, SOCK_DGRAM, 0);

    /* Preenche endereço do VPN Client */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9876);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        printf("Digite mensagem para VPN Client:\n");
        fgets(buffer, 1024, stdin);

        sendto(sock, buffer, strlen(buffer)+1, 0,
            (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    }
    return 0;
}
