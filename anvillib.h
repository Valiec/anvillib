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
};

typedef struct chunkRec chunkRec_t;

unsigned int readUnsignedBigEndian(unsigned char* data, char size);

chunkRec_t parseHeaderEntry(unsigned char* locs, unsigned char* timestamps);

chunkRec_t* parseLocHeader(unsigned char* locs, unsigned char* timestamps);

unsigned char* resizeBlock(unsigned char* block, unsigned long size, unsigned long step);

void inflateChunk(chunkRec_t* chunk);

void loadChunkData(chunkRec_t* chunk, FILE* fp);