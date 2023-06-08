#ifndef ANVILLIB_NBTLIB_H
#define ANVILLIB_NBTLIB_H

#ifndef ANVILLIB_NBTCORE_H
#include "nbtcore.h"
#include "zlib.h"
#include "stdio.h"
#endif

struct nbtFile {
    unsigned char isCompressedNBT;
    unsigned char* compressedData;
    unsigned char* uncompressedData;
    nbtTag_t* nbtData;
    unsigned char _nbtDataOutdated;
    unsigned char _nbtDataExists;
    unsigned char compressedDataOutdated;
    unsigned char _uncompressedDataOutdated;
    unsigned char _uncompressedDataExists;
    unsigned char _compressedDataOutdated;
    unsigned char _compressedDataExists;
    unsigned long sizeCompressed;
    unsigned long sizeUncompressed;
};

typedef struct nbtFile nbtFile_t;

void inflateFile(nbtFile_t* data);

int deflateFile(nbtFile_t* data);

void decodeFile(nbtFile_t* data);

void encodeFile(nbtFile_t* data);

#endif //ANVILLIB_NBTLIB_H
