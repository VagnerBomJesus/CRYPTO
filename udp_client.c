/*************************************************************
 * UDP CLIENT - ProgUDP1
 * Envia mensagens em UDP para a VPN Client.
 *************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>  


int main(){
    int clientSocket, nBytes;
    char buffer[1024];
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    // Cria socket UDP
    clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

    // Define parâmetros do servidor VPN Client
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9876);   // Porto onde VPN Client está a escutar UDP
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // VPN Client está na mesma máquina

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    addr_size = sizeof serverAddr;

    while(1){
        printf("Type a sentence to send to VPN Client:\n");
        fgets(buffer, 1024, stdin);

        nBytes = strlen(buffer) + 1;

        sendto(clientSocket, buffer, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);

        printf("Message sent to VPN Client: %s", buffer);
    }

    return 0;
}
