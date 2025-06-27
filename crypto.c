#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "crypto.h"

// Implementação da cifra de César
void caesarEncrypt(char *text, int key) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (isalpha(text[i])) {
            char base = isupper(text[i]) ? 'A' : 'a';
            text[i] = (text[i] - base + key) % 26 + base;
        }
    }
}

void caesarDecrypt(char *text, int key) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (isalpha(text[i])) {
            char base = isupper(text[i]) ? 'A' : 'a';
            text[i] = (text[i] - base - key + 26) % 26 + base;
        }
    }
}

// Diffie-Hellman simples
int generatePrivateKey() {
    return 3 + rand() % 10; 
}

int generatePublicKey(int privateKey, int g, int p) {
    int res = 1;
    for (int i = 0; i < privateKey; i++)
        res = (res * g) % p;
    return res;
}

int computeSharedKey(int receivedPublic, int privateKey, int p) {
    int res = 1;
    for (int i = 0; i < privateKey; i++)
        res = (res * receivedPublic) % p;
    return res;
}
