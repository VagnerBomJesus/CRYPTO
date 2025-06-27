#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

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
extern void printBlockchain(const void*);
extern void loadAdmins();
extern void saveAdmins();
extern int checkAdmin(const char*, const char*);
extern int registerAdmin(const char*, const char*, const char*);
extern int removeAdmin(const char*);
extern void listAdmins();
extern void loadConfig(char*, int*, char*, char*);
extern void saveConfig(const char*, int, char, const char*);
extern void writeLog(const char*);
extern void readLogs();
extern void clearLogs();
extern unsigned long simpleHash(const char*);

extern int adminCount;
extern struct {
    char username[50];
    char password[50];
    char role[10];
} admins[];

typedef struct BlockchainNode BlockchainNode;

#define MANAGER_PORT 9999
#define SERVER_PORT 9000
#define UDP_PORT 9877

enum Cipher { CAESAR, XOR, VIGENERE };

enum Cipher currentCipher = CAESAR;
int caesarKey = 4;
char xorKey = 0xAA;
char vigenereKey[50] = "SECRET";

BlockchainNode* head = NULL;
BlockchainNode* tail = NULL;

void clean_input(char *buffer) {
    buffer[strcspn(buffer, "\r\n")] = '\0';
}

/****************************************************
 * Envio de mensagem
 ****************************************************/
void send_message(int client_fd, const char *user_role, const char *user_name) {
    char buffer[1024];
    int n;

    write(client_fd, "\nDigite mensagem a enviar:\n> ", 28);
    n = read(client_fd, buffer, sizeof(buffer)-1);
    if (n <= 0) return;
    buffer[n] = '\0';
    clean_input(buffer);

    char original[1024];
    strcpy(original, buffer);

    int sharedKey = 7; // default para TESTES se admin quiser enviar algo sem diffie

    if (currentCipher == CAESAR)
        caesarEncrypt(buffer, sharedKey);
    else if (currentCipher == XOR)
        xorEncryptDecrypt(buffer, xorKey);
    else if (currentCipher == VIGENERE)
        vigenereEncrypt(buffer, vigenereKey);

    write(client_fd, "\nMensagem cifrada:\n", 19);
    write(client_fd, buffer, strlen(buffer));
    write(client_fd, "\n", 1);

    // Para o exemplo, descifra já e mostra
    if (currentCipher == CAESAR)
        caesarDecrypt(buffer, sharedKey);
    else if (currentCipher == XOR)
        xorEncryptDecrypt(buffer, xorKey);
    else if (currentCipher == VIGENERE)
        vigenereDecrypt(buffer, vigenereKey);

    write(client_fd, "\nMensagem decifrada:\n", 21);
    write(client_fd, buffer, strlen(buffer));
    write(client_fd, "\n", 1);

    char logline[512];
    sprintf(logline, "[MESSAGE] %s %s sent: %s", user_role, user_name, original);
    writeLog(logline);
}

/****************************************************
 * Blockchain (com role)
 ****************************************************/
void blockchain_menu(int client_fd, const char *user_role) {
    char buffer[1024];
    int n;

    while (1) {
        const char *msg =
            "\n==== BLOCKCHAIN ====\n"
            "1 - Ver blockchain\n"
            "2 - Adicionar bloco\n"
            "0 - Voltar\n"
            "Escolha: ";
        write(client_fd, msg, strlen(msg));

        n = read(client_fd, buffer, sizeof(buffer)-1);
        if (n <= 0) return;
        buffer[n] = '\0';
        clean_input(buffer);

        if (strcmp(buffer, "1") == 0) {
            printBlockchain(head);
        }
        else if (strcmp(buffer, "2") == 0) {
            if (strcmp(user_role, "ADMIN") != 0) {
                write(client_fd, "Acesso negado.\n", 16);
                continue;
            }
            write(client_fd, "Dados do novo bloco: ", 22);
            n = read(client_fd, buffer, sizeof(buffer)-1);
            if (n <= 0) return;
            buffer[n] = '\0';
            clean_input(buffer);

            static int blockCounter = 2;
            addNode(&head, &tail, blockCounter++, buffer);
            writeLog("[BLOCKCHAIN] Bloco adicionado.");
        }
        else if (strcmp(buffer, "0") == 0) {
            break;
        }
        else {
            write(client_fd, "Opção inválida.\n", 17);
        }
    }
}

/****************************************************
 * Configuração e logs (admin only)
 ****************************************************/
void config_management(int client_fd) {
    char buffer[1024];
    int n;

    while (1) {
        const char *msg =
            "\n==== GESTÃO CONFIGURAÇÃO / LOGS ====\n"
            "1 - Ver algoritmo atual\n"
            "2 - Alterar algoritmo\n"
            "3 - Ver logs\n"
            "4 - Limpar logs\n"
            "5 - Calcular hash de texto\n"
            "0 - Voltar\n"
            "Escolha: ";
        write(client_fd, msg, strlen(msg));

        n = read(client_fd, buffer, sizeof(buffer)-1);
        if (n <= 0) return;
        buffer[n] = '\0';
        clean_input(buffer);

        if (strcmp(buffer, "1") == 0) {
            char cipherName[20];
            if (currentCipher == CAESAR) strcpy(cipherName, "Caesar");
            else if (currentCipher == XOR) strcpy(cipherName, "XOR");
            else strcpy(cipherName, "Vigenere");
            sprintf(buffer, "Algoritmo atual: %s\n", cipherName);
            write(client_fd, buffer, strlen(buffer));
        }
        else if (strcmp(buffer, "2") == 0) {
            write(client_fd,
                "1 - Caesar\n2 - XOR\n3 - Vigenere\nEscolha: ",
                42);
            n = read(client_fd, buffer, sizeof(buffer)-1);
            if (n <= 0) return;
            buffer[n] = '\0';
            clean_input(buffer);

            if (strcmp(buffer, "1") == 0) {
                currentCipher = CAESAR;
                write(client_fd, "Caesar ativo.\n", 14);
                writeLog("[CONFIG] Algoritmo alterado para Caesar.");
            }
            else if (strcmp(buffer, "2") == 0) {
                currentCipher = XOR;
                write(client_fd, "XOR ativo.\n", 11);
                writeLog("[CONFIG] Algoritmo alterado para XOR.");
            }
            else if (strcmp(buffer, "3") == 0) {
                currentCipher = VIGENERE;
                write(client_fd, "Vigenere ativo.\n", 16);
                writeLog("[CONFIG] Algoritmo alterado para Vigenere.");
            }
        }
        else if (strcmp(buffer, "3") == 0) {
            readLogs();
        }
        else if (strcmp(buffer, "4") == 0) {
            clearLogs();
            write(client_fd, "Logs limpos.\n", 13);
            writeLog("[LOG] Logs limpos pelo admin.");
        }
        else if (strcmp(buffer, "5") == 0) {
            write(client_fd, "Digite texto para hash:\n", 24);
            n = read(client_fd, buffer, sizeof(buffer)-1);
            if (n <= 0) return;
            buffer[n] = '\0';
            clean_input(buffer);
            unsigned long h = simpleHash(buffer);
            char resp[128];
            sprintf(resp, "Hash simples (soma ASCII): %lu\n", h);
            write(client_fd, resp, strlen(resp));
        }
        else if (strcmp(buffer, "0") == 0) {
            break;
        }
    }
}

/****************************************************
 * Gestão de utilizadores (CRUD)
 ****************************************************/
void user_management(int client_fd) {
    char buffer[1024];
    char user[50], pass[50], role[10];
    int n;

    while (1) {
        const char *msg =
            "\n==== GESTÃO DE UTILIZADORES ====\n"
            "1 - Listar utilizadores\n"
            "2 - Adicionar utilizador\n"
            "3 - Remover utilizador\n"
            "4 - Alterar role\n"
            "0 - Voltar\n"
            "Escolha: ";
        write(client_fd, msg, strlen(msg));

        n = read(client_fd, buffer, sizeof(buffer)-1);
        if (n <= 0) break;
        buffer[n] = '\0';
        clean_input(buffer);

        if (strcmp(buffer, "1") == 0) {
            listAdmins();
        }
        else if (strcmp(buffer, "2") == 0) {
            write(client_fd, "Novo username: ", 15);
            n = read(client_fd, user, sizeof(user)-1);
            if (n <= 0) break;
            user[n] = '\0';
            clean_input(user);

            write(client_fd, "Nova password: ", 15);
            n = read(client_fd, pass, sizeof(pass)-1);
            if (n <= 0) break;
            pass[n] = '\0';
            clean_input(pass);

            write(client_fd, "Role (ADMIN ou USER): ", 23);
            n = read(client_fd, role, sizeof(role)-1);
            if (n <= 0) break;
            role[n] = '\0';
            clean_input(role);

            if (registerAdmin(user, pass, role)) {
                write(client_fd, "Utilizador registado.\n", 22);
                writeLog("[USER] Novo utilizador registado.");
            } else {
                write(client_fd, "Erro ao registar utilizador.\n", 30);
            }
        }
        else if (strcmp(buffer, "3") == 0) {
            write(client_fd, "Username a remover: ", 20);
            n = read(client_fd, user, sizeof(user)-1);
            if (n <= 0) break;
            user[n] = '\0';
            clean_input(user);

            if (removeAdmin(user)) {
                write(client_fd, "Utilizador removido.\n", 21);
                writeLog("[USER] Utilizador removido.");
            } else {
                write(client_fd, "Utilizador não encontrado.\n", 27);
            }
        }
        else if (strcmp(buffer, "4") == 0) {
            write(client_fd, "Username a alterar role: ", 25);
            n = read(client_fd, user, sizeof(user)-1);
            if (n <= 0) break;
            user[n] = '\0';
            clean_input(user);

            write(client_fd, "Nova role (ADMIN ou USER): ", 27);
            n = read(client_fd, role, sizeof(role)-1);
            if (n <= 0) break;
            role[n] = '\0';
            clean_input(role);

            int found = 0;
            for (int i = 0; i < adminCount; i++) {
                if (strcmp(admins[i].username, user) == 0) {
                    strcpy(admins[i].role, role);
                    found = 1;
                    saveAdmins();
                    write(client_fd, "Role alterada.\n", 15);
                    writeLog("[USER] Role alterada.");
                    break;
                }
            }
            if (!found) {
                write(client_fd, "Utilizador não encontrado.\n", 27);
            }
        }
        else if (strcmp(buffer, "0") == 0) {
            break;
        }
        else {
            write(client_fd, "Opção inválida.\n", 17);
        }
    }
}

void menu_admin(int client_fd, const char *user_name) {
    char buffer[1024];
    int n;

    while (1) {
        const char *msg =
            "\n========================================\n"
            "         VPN SERVER - ADMIN MENU\n"
            "========================================\n"
            "[1] Gestão de Utilizadores\n"
            "[2] Gestão Configuração e Logs\n"
            "[3] Blockchain\n"
            "[4] Enviar Mensagem\n"
            "[0] Logout\n"
            "========================================\n"
            "Escolha: ";
        write(client_fd, msg, strlen(msg));

        n = read(client_fd, buffer, sizeof(buffer)-1);
        if (n <= 0) break;
        buffer[n] = '\0';
        clean_input(buffer);

        if (strcmp(buffer, "1") == 0) {
            user_management(client_fd);
        }
        else if (strcmp(buffer, "2") == 0) {
            config_management(client_fd);
        }
        else if (strcmp(buffer, "3") == 0) {
            blockchain_menu(client_fd, "ADMIN");
        }
        else if (strcmp(buffer, "4") == 0) {
            send_message(client_fd, "ADMIN", user_name);
        }
        else if (strcmp(buffer, "0") == 0) {
            break;
        }
        else {
            write(client_fd, "Opção inválida.\n", 17);
        }
    }
}

void menu_user(int client_fd, const char *user_name) {
    char buffer[1024];
    int n;

    while (1) {
        const char *msg =
            "\n========================================\n"
            "         VPN SERVER - USER MENU\n"
            "========================================\n"
            "[1] Enviar Mensagem\n"
            "[2] Blockchain (ver)\n"
            "[0] Logout\n"
            "========================================\n"
            "Escolha: ";
        write(client_fd, msg, strlen(msg));

        n = read(client_fd, buffer, sizeof(buffer)-1);
        if (n <= 0) break;
        buffer[n] = '\0';
        clean_input(buffer);

        if (strcmp(buffer, "1") == 0) {
            send_message(client_fd, "USER", user_name);
        }
        else if (strcmp(buffer, "2") == 0) {
            blockchain_menu(client_fd, "USER");
        }
        else if (strcmp(buffer, "0") == 0) {
            break;
        }
        else {
            write(client_fd, "Opção inválida.\n", 17);
        }
    }
}

void handle_manager(int client_fd) {
    char user[50], pass[50];
    char buffer[1024];
    int n;

    loadAdmins();

    write(client_fd, "Username: ", 10);
    n = read(client_fd, user, sizeof(user)-1);
    if (n <= 0) { close(client_fd); return; }
    user[n] = '\0';
    clean_input(user);

    write(client_fd, "Password: ", 10);
    n = read(client_fd, pass, sizeof(pass)-1);
    if (n <= 0) { close(client_fd); return; }
    pass[n] = '\0';
    clean_input(pass);

    int authLevel = checkAdmin(user, pass);
    if (authLevel == 0) {
        write(client_fd, "Access Denied.\n", 15);
        close(client_fd);
        return;
    }

    writeLog("[LOGIN] Login efetuado no Manager.");

    if (authLevel == 2)
        menu_admin(client_fd, user);
    else
        menu_user(client_fd, user);

    close(client_fd);
}


void process_client(int client_fd) {
    int udp_fd;
    struct sockaddr_in udp_addr;
    socklen_t addr_size;
    char buffer[1024];
    int nBytes;

    udp_fd = socket(PF_INET, SOCK_DGRAM, 0);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(udp_addr.sin_zero, '\0', sizeof udp_addr.sin_zero);
    addr_size = sizeof(udp_addr);

    int p = 23, g = 5;
    int myPrivate = generatePrivateKey();
    int clientPublic;
    read(client_fd, &clientPublic, sizeof(clientPublic));
    int myPublic = generatePublicKey(myPrivate, g, p);
    write(client_fd, &myPublic, sizeof(myPublic));

    int sharedKey = computeSharedKey(clientPublic, myPrivate, p);
    printf("VPN Server shared key: %d\n", sharedKey);

    addNode(&head, &tail, 1, "VPN Server connection established");

    while (1) {
        nBytes = read(client_fd, buffer, sizeof(buffer));
        if (nBytes <= 0) break;
        buffer[nBytes] = '\0';

        if (currentCipher == CAESAR)
            caesarDecrypt(buffer, sharedKey);
        else if (currentCipher == XOR)
            xorEncryptDecrypt(buffer, xorKey);
        else if (currentCipher == VIGENERE)
            vigenereDecrypt(buffer, vigenereKey);

        printf("VPN Server decrypted: %s\n", buffer);

        sendto(udp_fd, buffer, strlen(buffer)+1, 0,
               (struct sockaddr*)&udp_addr, addr_size);

        if (currentCipher == CAESAR)
            caesarEncrypt(buffer, sharedKey);
        else if (currentCipher == XOR)
            xorEncryptDecrypt(buffer, xorKey);
        else if (currentCipher == VIGENERE)
            vigenereEncrypt(buffer, vigenereKey);

        write(client_fd, buffer, strlen(buffer)+1);
    }

    close(client_fd);
    close(udp_fd);
    freeBlockchain(&head);
}


int main() {
    int fd, manager_fd, client, manager;
    struct sockaddr_in addr, manager_addr;

    loadConfig((char*)&currentCipher, &caesarKey, &xorKey, vigenereKey);

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(fd, 5);

    bzero(&manager_addr, sizeof(manager_addr));
    manager_addr.sin_family = AF_INET;
    manager_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    manager_addr.sin_port = htons(MANAGER_PORT);
    manager_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(manager_fd, (struct sockaddr*)&manager_addr, sizeof(manager_addr));
    listen(manager_fd, 5);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        FD_SET(manager_fd, &readfds);
        int maxfd = (fd > manager_fd) ? fd : manager_fd;
        select(maxfd+1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(fd, &readfds)) {
            client = accept(fd, NULL, NULL);
            if (fork() == 0) {
                close(fd);
                process_client(client);
                exit(0);
            }
            close(client);
        }
        if (FD_ISSET(manager_fd, &readfds)) {
            manager = accept(manager_fd, NULL, NULL);
            if (fork() == 0) {
                close(manager_fd);
                handle_manager(manager);
                exit(0);
            }
            close(manager);
        }
    }
}
