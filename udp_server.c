/*************************************************************
 * UDP SERVER - ProgUDP2
 * Recebe mensagens via UDP vindas do VPN Server.
 *************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 512
#define PORT 9877   // Porto onde o VPN Server vai enviar os dados UDP

void erro(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in si_minha, si_outra;
    int s, recv_len;
    socklen_t slen = sizeof(si_outra);
    char buf[BUFLEN];

    // Cria socket UDP
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        erro("Erro na criação do socket");
    }

    // Preenche a struct de endereço
    si_minha.sin_family = AF_INET;
    si_minha.sin_port = htons(PORT);
    si_minha.sin_addr.s_addr = htonl(INADDR_ANY);

    // Faz bind do socket ao endereço
    if (bind(s, (struct sockaddr *)&si_minha, sizeof(si_minha)) == -1) {
        erro("Erro no bind");
    }

    printf("ProgUDP2 à escuta no porto %d...\n", PORT);

    while(1) {
        // Recebe mensagem UDP
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_outra, &slen)) == -1) {
            erro("Erro no recvfrom");
        }

        buf[recv_len] = '\0';

        printf("Recebi uma mensagem do endereço %s e porto %d\n",
               inet_ntoa(si_outra.sin_addr), ntohs(si_outra.sin_port));
        printf("Conteúdo da mensagem: %s\n", buf);
    }

    close(s);
    return 0;
}
