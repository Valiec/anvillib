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

    blockData_t** remapData = malloc(500*sizeof(blockData_t *));

    for(i=0;i<500;i++)
    {
        remapData[i] = malloc(16*sizeof(blockData_t));
        int j;
        for(j=0;j<16;j++)
        {
            remapData[i][j].id = 0;
            remapData[i][j].meta = 0;
        }
    }

    FILE* fp;
    fp = fopen("mismatched.csv", "r");
    char* buf = malloc(100);
    char finalChar = '\n';
    while(finalChar != EOF)
    {
        char* foo = fgets(buf, 5, (FILE *)fp);
        //printf("%s:", buf);
        fgetc(fp);
        char* bar = fgets(buf, 3, (FILE *)fp);
        //printf("%s -> ", buf);
        fgetc(fp);
        char id = atoi(foo);
        char meta = atoi(bar);
        foo = fgets(buf, 5, (FILE *)fp);
        //printf("%s:", buf);
        fgetc(fp);
        bar = fgets(buf, 3, (FILE *)fp);
        //printf("%s\n", buf);
        char id2 = atoi(foo);
        char meta2 = atoi(bar);
        remapData[id][meta].id = id2;
        remapData[id][meta].meta = meta2;
        finalChar = fgetc(fp);
    }
    fclose(fp);

    for(i = 0; i<1024; i++)
    {
        if(chunks[i].size > 0)
        {
            loadChunkData(&(chunks[i]), regionFp);
            inflateChunk(&(chunks[i]));
            decodeChunkData(&(chunks[i]));
            printf("Chunk %u: offset=%u, size=%u, exactSize=%u, compressionScheme=%u, sizeUncompressed=%lu\n", i, chunks[i].offset, chunks[i].size, chunks[i].exactSize, chunks[i].compressionScheme, chunks[i].sizeUncompressed);
            int exitcode;
            int x,y,z;
            initBlockData(chunks+i);
            for(y=0; y<256; y++) {
                //printf("getting block at y=%u, \n", y);
                for (x = 0; x < 16; x++) {
                    for (z = 0; z < 16; z++) {
                        blockData_t block = getBlockIdInChunk(chunks[i], x, y, z, &exitcode);
                        //printf("Block: %u, %u\n", block.id, block.meta);
                        if(block.id != 0)
                        {
                            if(block.id > 962 && remapData[block.id-962][block.meta].id != 0) {
                                //printf("trying to write at y=%u, old block ID is %u, old block meta is %u\n", y, block.id, block.meta);
                                blockData_t new;
                                new.id = remapData[block.id-962][block.meta].id;
                                new.meta = remapData[block.id-962][block.meta].meta;
                                setBlockIdInChunk(chunks + i, x, y, z, new);
                            }
                        }
                        //printf("getting block at y=%u again\n", y);
                        //blockData_t block_again = getBlockIdInChunk(chunks[0], 12, y, 4, &exitcode);
                        //printf("Block: %u, %u\n", block_again.id, block_again.meta);
                   }
                }
            }
            encodeChunkData(&(chunks[i]));
            deflateChunk(&(chunks[i]));
            //break;
        }
    }
    for(i = 0; i<1; i++) {
        if (chunks[i].size > 0)
        {
            writeChunk(chunks[i], chunks, i, regionFp);
        }
        freeChunk(&(chunks[i]));
    }
    free(chunks);
    fclose(regionFp);
    return 0;

}