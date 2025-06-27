#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "blockchain.h"

// Função hash simples
void generateHash(const char *data, const char *prevHash, char *hash) {
    unsigned long sum = 0;
    for (int i = 0; data[i] != '\0'; i++)
        sum += data[i];
    for (int i = 0; prevHash[i] != '\0'; i++)
        sum += prevHash[i];
    sprintf(hash, "%064lx", sum);
}

BlockchainNode* createBlockchainNode(int id, const char* data, const char* prevBlockHash) {
    BlockchainNode* newNode = (BlockchainNode*) malloc(sizeof(BlockchainNode));
    newNode->id = id;
    strncpy(newNode->data, data, 255);
    newNode->data[255] = '\0';
    newNode->timestamp = time(NULL);
    strncpy(newNode->prevBlockHash, prevBlockHash, HASH_LENGTH);
    generateHash(data, prevBlockHash, newNode->blockHash);
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

void addNode(BlockchainNode** head, BlockchainNode** tail, int id, const char* data) {
    char prevHash[HASH_LENGTH];
    if (*tail == NULL) {
        strncpy(prevHash, "0000000000000000000000000000000000000000000000000000000000000000", HASH_LENGTH);
    } else {
        strncpy(prevHash, (*tail)->blockHash, HASH_LENGTH);
    }
    BlockchainNode* newNode = createBlockchainNode(id, data, prevHash);
    if (*head == NULL) {
        *head = newNode;
        *tail = newNode;
    } else {
        (*tail)->next = newNode;
        newNode->prev = *tail;
        *tail = newNode;
    }
    printf("Bloco %d adicionado à blockchain: %s\n", newNode->id, newNode->data);
}

void printNode(const BlockchainNode* node) {
    printf("----------------------------------------\n");
    printf("ID do Bloco: %d\n", node->id);
    printf("Dados: %s\n", node->data);
    printf("Hash do Bloco: %s\n", node->blockHash);
    printf("Hash do Bloco Anterior: %s\n", node->prevBlockHash);
    printf("Timestamp: %s", ctime(&node->timestamp));
    printf("----------------------------------------\n");
}

void printBlockchainForward(const BlockchainNode* head) {
    const BlockchainNode* current = head;
    while (current != NULL) {
        printNode(current);
        current = current->next;
    }
}

void printBlockchainBackward(const BlockchainNode* tail) {
    const BlockchainNode* current = tail;
    while (current != NULL) {
        printNode(current);
        current = current->prev;
    }
}

void freeBlockchain(BlockchainNode** head) {
    BlockchainNode* current = *head;
    while (current != NULL) {
        BlockchainNode* nextNode = current->next;
        free(current);
        current = nextNode;
    }
    *head = NULL;
    printf("\nBlockchain libertada da memória.\n");
}
