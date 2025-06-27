#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * ProgUDP2 (udp_server)
 * Recebe mensagens UDP do VPN Server.
 */
int main() {
    int s;
    struct sockaddr_in addr, client;
    char buffer[512];
    socklen_t client_len = sizeof(client);

    /* Cria socket UDP */
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    /* Preenche estrutura do endereço */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9877);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Faz bind ao porto */
    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    printf("UDP Server listening on port 9877...\n");

    while (1) {
        int n = recvfrom(s, buffer, sizeof(buffer)-1, 0,
                         (struct sockaddr*)&client, &client_len);
        buffer[n] = '\0';

        printf("Received from VPN Server: %s\n", buffer);
    }
    close(s);
    return 0;
}
