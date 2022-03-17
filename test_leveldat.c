#include <stdio.h>
#include <stdlib.h>
#include "nbtlib.h"
#include <string.h>

#define BUFSTEP 512

int main(int argc, char** argv)
{
    FILE* regionFp;
    if (argc<2)
    {
        printf("Level file not specified. Exiting.\n");
        exit(1);
    }
    regionFp = fopen(argv[1], "rb+");
    if (regionFp == NULL)
    {
        printf("Opening level file %s failed. Exiting.\n", argv[1]);
        exit(1);
    }

    nbtFile_t* levelDat = malloc(sizeof(nbtFile_t));

    levelDat->sizeCompressed = 0;
    levelDat->compressedData = malloc(BUFSTEP); //starting buffer size

    unsigned long size = BUFSTEP;

    unsigned long offset = 0;

    unsigned long amountRead;

    amountRead = fread(levelDat->compressedData+offset, 1, BUFSTEP, regionFp);
    while(amountRead == BUFSTEP)
    {
        offset+=BUFSTEP;
        levelDat->compressedData = resizeBlock(levelDat->compressedData, size, BUFSTEP);
        size+=BUFSTEP;
        levelDat->sizeCompressed+=BUFSTEP;
        amountRead = fread(levelDat->compressedData+offset, 1, BUFSTEP, regionFp);
    }

    levelDat->sizeCompressed+=amountRead;

    printf("%lu\n", levelDat->sizeCompressed);

    inflateFile(levelDat);

    int i;

    for(i=0;i<100;i++)
    {
        printf("%d ", levelDat->uncompressedData[i]);
    }

    printf("\n");

    nbtTag_t data = decodeTag(levelDat->uncompressedData);

    nbtTag_t* tags = ((nbtTag_t*)(((nbtTag_t*)(data.payload))->payload));

    int tagCount = (nbtTag_t*)(((nbtTag_t*)(data.payload)))->payloadLength;

    int j;

    for(j = 0; j<tagCount; j++)
    {
        printf("%s\n", tags[j].name);
    }

    long long length = 0;

    levelDat->uncompressedData = encodeTag(&data, &length);



    free(levelDat);
    fclose(regionFp);
    return 0;

}