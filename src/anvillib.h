#ifndef ANVILLIB_ANVILLIB_H
#define ANVILLIB_ANVILLIB_H

#define REGION_HEADER_SIZE 2192
#define END_LOC_HEADER_POS 4096

#define CHUNK_LOC_OFFSET(x, y) 4*((x&31)*(z&31)*32)

struct chunkRec {
    unsigned int offset;
    unsigned char size;
    unsigned int timestamp;
    unsigned char* chunkDataCompressed;
    unsigned char* chunkDataUncompressed;
    unsigned int exactSize;
    unsigned long sizeUncompressed;
    unsigned char compressionScheme;
    nbtTag_t chunkData;
    nbtTag_t** blockIds;
    nbtTag_t** blockAdd;
    nbtTag_t** blockMeta;
    unsigned char* hasAdd;
    unsigned char* sectionPresent;

    unsigned char _compressedDataExists;
    unsigned char _uncompressedDataExists;
    unsigned char _nbtDataExists;
    unsigned char _blockDataExists;
    unsigned char _nbtDataOutdated;
    unsigned char _uncompressedDataOutdated;
    unsigned char _compressedDataOutdated;
};

typedef struct chunkRec chunkRec_t;

struct regionFile {
    FILE* regionFp;
    chunkRec_t* data;
};

typedef struct regionFile regionFile_t;

struct blockData {
    unsigned short id;
    unsigned char meta;
};

typedef struct blockData blockData_t;

unsigned int readUnsignedBigEndian(unsigned char* data, char size);

chunkRec_t parseHeaderEntry(unsigned char* locs, unsigned char* timestamps);

chunkRec_t* parseLocHeader(unsigned char* locs, unsigned char* timestamps);

void inflateChunk(chunkRec_t* chunk);

int deflateChunk(chunkRec_t* chunk);

void loadChunkData(chunkRec_t* chunk, FILE* fp);

void decodeChunkData(chunkRec_t* chunk);

void encodeChunkData(chunkRec_t* chunk);

blockData_t getBlockIdInChunk(chunkRec_t chunk, int x, int y, int z, int* exitcode);

void freeChunk(chunkRec_t* chunk);

int writeChunk(chunkRec_t chunk, chunkRec_t* header, int chunkIndex, FILE* fp);

int initBlockData(chunkRec_t* chunk);

int setBlockIdInChunk(chunkRec_t* chunk, int x, int y, int z, blockData_t newBlock);

#endif //ANVILLIB_ANVILLIB_H
