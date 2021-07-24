#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include "zlib.h"

#define REGION_HEADER_SIZE 2192
#define END_LOC_HEADER_POS 4096

#define CHUNK_LOC_OFFSET(x, y) 4*((x&31)*(z&31)*32)

struct chunkRec {
	unsigned int offset;
	unsigned char size;
	unsigned int timestamp;
};

typedef struct chunkRec chunkRec_t;

int readUnsignedBigEndian(unsigned char* data, char size)
{
	int value = 0;
	char i;
	for(i=0; i<size; i++)
	{
		unsigned char theByte = *(data+i);
		//printf("%u/", theByte);
		value = value << 8; //left shift last byte
		value += theByte;
	}
	//printf("\n");
	return value;
}

chunkRec_t parseHeaderEntry(unsigned char* locs, unsigned char* timestamps)
{
	chunkRec_t headerEntry;
	headerEntry.offset = readUnsignedBigEndian(locs, 3);
	headerEntry.size = *(locs+3);
	headerEntry.timestamp = readUnsignedBigEndian(locs, 4);
	return headerEntry;
}

chunkRec_t* parseLocHeader(unsigned char* locs, unsigned char* timestamps)
{
	chunkRec_t* header = malloc(sizeof(chunkRec_t)*1024); //alloc space for loc header data
	unsigned short index;
	for(index=0; index<1024; index++)
	{
		chunkRec_t headerEntry = parseHeaderEntry(locs+(index*4), timestamps+(index*4));
		header[index] = headerEntry;
	}
	return header; //return pointer to arr

}

/*unsigned char* inflateChunk(unsigned char* chunk)
{
	int returnVal;
	unsigned long outputLen;
	z_stream stream;


}*/

int main(int argc, char** argv)
{
	FILE* regionFp;
	regionFp = fopen(argv[1], "rb");
	unsigned char* locsRaw = malloc(sizeof(char)*4096); //alloc space for loc header data
	unsigned char* timestampsRaw = malloc(sizeof(char)*4096); //alloc space for loc header data
	fread(locsRaw, sizeof(char), 4096, regionFp); //read the header
	fread(timestampsRaw, sizeof(char), 4096, regionFp); //read the header
	chunkRec_t* header = parseLocHeader(locsRaw, timestampsRaw);
	free(locsRaw);
	free(timestampsRaw); //done with raw data
	int i;
	for(i = 0; i<1024; i++)
	{
		printf("Chunk %u: offset=%u, size=%u\n", i, header[i].offset, header[i].size);
	}

}