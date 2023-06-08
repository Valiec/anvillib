#ifndef ANVILLIB_WORLD_H
#define ANVILLIB_WORLD_H

#ifndef ANVILLIB_ANVILLIB_H
#include "anvillib.h"
#endif

struct dimension {
    unsigned int id; //maybe
    regionFile_t* regions;
};

typedef struct dimension dimension_t;

struct world {
    char* saveName;
    char* worldName;
    unsigned long lockTime;
    unsigned char hasLock;
    //levelData_t levelData;
    //playerData_t* players;
    //playerStats_t* stats;
    dimension_t* dimensions;
};

typedef struct world world_t;

#endif //ANVILLIB_WORLD_H
