#include <stdio.h>
#include <stdlib.h>
#include "nbtlib.h"
#include "anvillib.h"
#include <string.h>

int main(int argc, char** argv)
{
    FILE* regionFp;
    regionFp = fopen(argv[1], "rb+");
    unsigned char* locsRaw = malloc(sizeof(char)*4096); //alloc space for loc header data
    unsigned char* timestampsRaw = malloc(sizeof(char)*4096); //alloc space for loc header data
    fread(locsRaw, sizeof(char), 4096, regionFp); //read the header
    fread(timestampsRaw, sizeof(char), 4096, regionFp); //read the header
    chunkRec_t* chunks = parseLocHeader(locsRaw, timestampsRaw);
    free(locsRaw);
    free(timestampsRaw); //done with raw data
    int i;

    /*nbtTag_t tag;
    tag.isNamed = 1;
    tag.typeId = 10;
    tag.nameLen = 0;
    tag.name = "";
    tag.payloadLength = 2;
    tag.payload = malloc(2*sizeof(nbtTag_t));

    nbtTag_t tag2;
    tag.isNamed = 1;
    tag2.typeId = 1;
    tag2.nameLen = 6;
    tag2.name = "Answer";
    tag2.payload = malloc(1);
    *((char*)tag2.payload) = 42;
    ((nbtTag_t *)(tag.payload))[1] = tag2;

    nbtTag_t tag3;
    tag.isNamed = 1;
    tag3.typeId = 7;
    tag3.nameLen = 5;
    tag3.name = "Bytes";
    tag3.payloadLength = 5;
    tag3.payload = "abcdefgh";
    ((nbtTag_t *)(tag.payload))[0] = tag3;

    long long length = 0;
    unsigned char* code = encodeTag(&tag, &length);
    for(i=0; i<length; i++)
    {
        fprintf(stderr, "%u ", code[i]);
    }
    nbtTag_t decoded = decodeTag(code);
    printf("\n");
    length = 0;
    unsigned char* code2 = encodeTag(&decoded, &length);
    for(i=0; i<length; i++)
    {
        fprintf(stderr, "%u ", code2[i]);
    }
    printf("\n");*/

    for(i = 0; i<1024; i++)
    {
        if(chunks[i].size > 0)
        {
            loadChunkData(&(chunks[i]), regionFp);
            inflateChunk(&(chunks[i]));
            printf("inflating!\n");
            decodeChunkData(&(chunks[i]));
            printf("decoding!\n");
            printf("Chunk %u: offset=%u, size=%u, exactSize=%u, compressionScheme=%u, sizeUncompressed=%lu\n", i, chunks[i].offset, chunks[i].size, chunks[i].exactSize, chunks[i].compressionScheme, chunks[i].sizeUncompressed);
            printf("encoding!\n");
            encodeChunkData(&(chunks[i]));
            printf("deflating!\n");
            deflateChunk(&(chunks[i]));
            printf("inflating (again)!\n");
            inflateChunk(&(chunks[i]));
            printf("decoding (again)!\n");
            decodeChunkData(&(chunks[i]));
            printf("encoding (again)!\n");
            encodeChunkData(&(chunks[i]));
            printf("deflating! (again)\n");
            deflateChunk(&(chunks[i]));
            //break;
        }
    }
    int exitcode;
    //printf("getting block\n");
    int y = 0;
    for(y=0; y<255; y++)
    {
        blockData_t block = getBlockIdInChunk(chunks[12], 12, y, 4, &exitcode);
        printf("Block: %u, %u\n", block.id, block.meta);
    }
    for(i = 0; i<1024; i++)
    {
        //writeChunk(chunks[i], chunks, i, regionFp);
        freeChunk(&(chunks[i]));
    }
    free(chunks);
    fclose(regionFp);
    return 0;

}