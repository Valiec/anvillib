#include <stdio.h>
#include <stdlib.h>
#include "nbtcore.h"
#include "anvillib.h"
#include <string.h>

int main(int argc, char** argv)
{
    FILE* regionFp;
    if (argc<2)
    {
        printf("Region file not specified. Exiting.\n");
        exit(1);
    }
    regionFp = fopen(argv[1], "rb+");
    if (regionFp == NULL)
    {
        printf("Opening region file %s failed. Exiting.\n", argv[1]);
        exit(1);
    }
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
            fflush(stdout);
            decodeChunkData(&(chunks[i]));
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
                            if(y/16 > 1)
                            {
                                //printf("sectionPresent is %u\n", chunks[i].sectionPresent[y/16]);
                            }
                            //printf("trying to write at y=%u, old block ID is %u, old block meta is %u\n", y, block.id, block.meta);
                            blockData_t new;
                            if(block.id == 2)
                            {
                                new.id = 3;
                                new.meta = 1;
                            }
                            else
                            {
                                new.id = 19;
                                new.meta = 0;
                            }
                            setBlockIdInChunk(chunks+i, x, y, z, new);
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
    for(i = 0; i<1024; i++) {
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