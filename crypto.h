#ifndef CRYPTO_H
#define CRYPTO_H

// Função para encriptar/desencriptar com Cifra de César
void caesarEncrypt(char *text, int key);
void caesarDecrypt(char *text, int key);

// Funções Diffie-Hellman
int generatePrivateKey();
int generatePublicKey(int privateKey, int g, int p);
int computeSharedKey(int receivedPublic, int privateKey, int p);

#endif
