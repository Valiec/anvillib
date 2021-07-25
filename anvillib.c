#include <stdio.h>
#include <stdlib.h>
#include "zlib.h"

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
};

typedef struct chunkRec chunkRec_t;

unsigned int readUnsignedBigEndian(unsigned char* data, char size)
{
	unsigned int value = 0;
	char i;
	for(i=0; i<size; i++)
	{
		unsigned char theByte = *(data+i);
		//printf("%u/", theByte);
		value = value << 8; //left shift last byte
		value += theByte;
	}
	//printf("\n Value is: %u\n", value);
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

unsigned char* resizeBlock(unsigned char* block, unsigned long size, unsigned long step)
{
	unsigned char* newBlock = malloc(sizeof(char)*size+step);
	int i;
	for(i=0; i<size; i++)
	{
		newBlock[i] = block[i];
	}
	free(block);
	return newBlock;
}

void inflateChunk(chunkRec_t* chunk)
{
	int returnVal;
	//unsigned long outputLen;
	unsigned long buflen = chunk->exactSize*7*sizeof(char);
	unsigned char* databuf = malloc(chunk->exactSize*7*sizeof(char));
	z_stream stream;
	stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = chunk->exactSize;
    stream.next_in = chunk->chunkDataCompressed; //data to inflate
    returnVal = inflateInit(&stream); //init inflate
    stream.next_out = databuf; //decompressed data
    stream.avail_out = buflen;
    chunk->sizeUncompressed = buflen;
   	//printf("decompressing... return val: %d, avail_out: %d, avail_in: %d\n", returnVal, stream.avail_out, stream.avail_in);
    returnVal = inflate(&stream, Z_NO_FLUSH); //decompress
    while(returnVal != Z_STREAM_END)
    {
    	//printf("decompressing... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
    	chunk->sizeUncompressed += chunk->exactSize*7*sizeof(char);
    	databuf = resizeBlock(databuf, buflen, chunk->exactSize*7*sizeof(char));
    	stream.next_out = databuf+buflen;
    	stream.avail_out += chunk->exactSize*7*sizeof(char);
    	buflen += chunk->exactSize*7*sizeof(char);
    	returnVal = inflate(&stream, Z_NO_FLUSH); //decompress
    }
    //printf("decompressed... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
    chunk->sizeUncompressed -= stream.avail_out;
    inflateEnd(&stream);
    chunk->chunkDataUncompressed = databuf;
}

void loadChunkData(chunkRec_t* chunk, FILE* fp)
{
	fseek(fp, (chunk->offset)*4096, SEEK_SET);
	unsigned char* chunkDataRaw = malloc(sizeof(char)*4096*chunk->size);
	unsigned char* chunkHeaderRaw = malloc(sizeof(char)*5);
	fread(chunkHeaderRaw, sizeof(char), 5, fp); //read the data header
	chunk->exactSize = readUnsignedBigEndian(chunkHeaderRaw, 4);
	chunk->compressionScheme = *(chunkHeaderRaw+4);
	fread(chunkDataRaw, sizeof(char), chunk->exactSize, fp); //read the data
	chunk->chunkDataCompressed = chunkDataRaw;
	free(chunkHeaderRaw); //don't need this
}

int main(int argc, char** argv)
{
	FILE* regionFp;
	regionFp = fopen(argv[1], "rb");
	unsigned char* locsRaw = malloc(sizeof(char)*4096); //alloc space for loc header data
	unsigned char* timestampsRaw = malloc(sizeof(char)*4096); //alloc space for loc header data
	fread(locsRaw, sizeof(char), 4096, regionFp); //read the header
	fread(timestampsRaw, sizeof(char), 4096, regionFp); //read the header
	chunkRec_t* chunks = parseLocHeader(locsRaw, timestampsRaw);
	free(locsRaw);
	free(timestampsRaw); //done with raw data
	int i;
	for(i = 0; i<1024; i++)
	{
		if(chunks[i].size > 0)
		{
			loadChunkData(&(chunks[i]), regionFp);
			inflateChunk(&(chunks[i]));
			printf("Chunk %u: offset=%u, size=%u, exactSize=%u, compressionScheme=%u, sizeUncompressed=%lu\n", i, chunks[i].offset, chunks[i].size, chunks[i].exactSize, chunks[i].compressionScheme, chunks[i].sizeUncompressed);
			//break;
		}
	}

}