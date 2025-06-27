#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#define HASH_LENGTH 65

typedef struct BlockchainNode {
    int id;
    char data[256];
    char blockHash[HASH_LENGTH];
    char prevBlockHash[HASH_LENGTH];
    time_t timestamp;
    struct BlockchainNode *next;
    struct BlockchainNode *prev;
} BlockchainNode;

BlockchainNode* createBlockchainNode(int id, const char* data, const char* prevBlockHash);
void addNode(BlockchainNode** head, BlockchainNode** tail, int id, const char* data);
void printBlockchainForward(const BlockchainNode* head);
void printBlockchainBackward(const BlockchainNode* tail);
void freeBlockchain(BlockchainNode** head);

#endif
