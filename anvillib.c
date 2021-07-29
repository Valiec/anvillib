#include <stdio.h>
#include <stdlib.h>
#include "nbtlib.h"
#include "anvillib.h"
#include "zlib.h"
#include <string.h>

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
    headerEntry._compressedDataExists = 0;
    headerEntry._uncompressedDataExists = 0;
    headerEntry._nbtDataExists = 0;
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
    if(chunk->_uncompressedDataExists)
    {
        free(chunk->chunkDataUncompressed);
    }
    chunk->chunkDataUncompressed = databuf;
    chunk->_uncompressedDataExists = 1;
}

void deflateChunk(chunkRec_t* chunk)
{
    int returnVal;
    //printf("Deflating chunk of size %lu, to an estimated %u\n", chunk->sizeUncompressed, chunk->exactSize);
    //unsigned long outputLen;
    unsigned long buflen = chunk->exactSize;
    unsigned char* databuf = malloc(chunk->exactSize*sizeof(char));
    z_stream stream;
    unsigned int chunkSize = chunk->exactSize;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = chunk->sizeUncompressed;
    stream.next_in = chunk->chunkDataUncompressed; //data to deflate
    returnVal = deflateInit(&stream, -1); //init deflate
    stream.next_out = databuf; //decompressed data
    stream.avail_out = buflen;
    chunk->exactSize = buflen;
    //printf("decompressing... return val: %d, avail_out: %d, avail_in: %d\n", returnVal, stream.avail_out, stream.avail_in);
    returnVal = deflate(&stream, Z_FINISH); //compress
    while(returnVal != Z_STREAM_END)
    {
        //printf("decompressing... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
        databuf = resizeBlock(databuf, buflen, chunkSize);
        stream.next_out = databuf+buflen;
        stream.avail_out += chunkSize;
        buflen += chunkSize;
        returnVal = deflate(&stream, Z_FINISH); //compress
    }
    //printf("decompressed... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
    chunk->exactSize -= stream.avail_out;
    deflateEnd(&stream);
    if(chunk->_compressedDataExists)
    {
        free(chunk->chunkDataCompressed);
    }
    chunk->chunkDataCompressed = databuf;
    chunk->_compressedDataExists = 1;
}

void decodeChunkData(chunkRec_t* chunk)
{
    if(chunk->_nbtDataExists)
    {
        freeTag(&(chunk->chunkData));
    }
    chunk->chunkData = decodeTag(chunk->chunkDataUncompressed);
    chunk->_nbtDataExists = 1;
}

void encodeChunkData(chunkRec_t* chunk)
{
    if(chunk->_uncompressedDataExists)
    {
        free(chunk->chunkDataUncompressed);
    }
    long long length = 0;
    chunk->chunkDataUncompressed = encodeTag(&(chunk->chunkData), &length);
    chunk->sizeUncompressed = length;
    chunk->_uncompressedDataExists = 1;
    int i;
    for(i=0; i<chunk->sizeUncompressed; i++)
    {
        //fprintf(stderr, "%u\n", chunk->chunkDataUncompressed[i]);
    }
}


//gets a block ID and meta given a chunk and block coords within that chunk (0<x<16, 0<z<16, 0<y<256)
//does not validate the block coords make any sense
blockData_t getBlockIdInChunk(chunkRec_t chunk, int x, int y, int z, int* exitcode)
{
    *exitcode = 0;
    blockData_t fail; //for returning on failure

    if((x<0||x>15) || (z<0||z>15) || (y<0||x>256)) //bad coords
    {
        *exitcode = 1;
        return fail;
    }

    if(chunk.size == 0) //chunk not present
    {
        *exitcode = 1;
        return fail;
    }

    nbtTag_t rootTag = chunk.chunkData;
    nbtTag_t levelTag = ((nbtTag_t*)(rootTag.payload))[0]; //there's only one tag
    nbtTag_t sectionsTag;
    int i, j;
    for(i=0;i<levelTag.payloadLength; i++)
    {
        nbtTag_t tag = ((nbtTag_t*)(levelTag.payload))[i];
        if(strcmp(tag.name, "Sections") == 0)
        {
            sectionsTag = tag;
            break;
        }
    }
    nbtTag_t sectionTag;
    char correctSectionFound = 0;
    for(i=0;i<sectionsTag.payloadLength; i++)
    {
        nbtTag_t aSectionTag = ((nbtTag_t*)(sectionsTag.payload))[i];
        nbtTag_t yTag;
        for(j=0;j<aSectionTag.payloadLength; j++)
        {
            nbtTag_t tag = ((nbtTag_t*)(aSectionTag.payload))[j];
            if(strcmp(tag.name, "Y") == 0)
            {
                yTag = tag;
                break;
            }
        }
        int ymin = (*((unsigned char*)(yTag.payload)))*16;
        int ymax = ((*((unsigned char*)(yTag.payload)))+1)*16;
        if(y>=ymin && y<ymax)
        {
            sectionTag = aSectionTag;
            correctSectionFound = 1;
            break;
        }
    }
    if(!correctSectionFound)
    {
        fail.id = 0;
        fail.meta = 0;
        return fail; //not actually failing, returning air
    }
    char hasAdd = 0;
    nbtTag_t blocksTag;
    nbtTag_t addTag;
    nbtTag_t dataTag;
    for(i=0;i<sectionTag.payloadLength; i++)
    {
        nbtTag_t tag = ((nbtTag_t*)(sectionTag.payload))[i];
        if(strcmp(tag.name, "Blocks") == 0)
        {
            printf("found blocks!\n");
            blocksTag = tag;
        }
        if(strcmp(tag.name, "Add") == 0)
        {
            printf("found add!\n");
            addTag = tag;
            hasAdd = 1;
        }
        if(strcmp(tag.name, "Data") == 0)
        {
            printf("found data!\n");
            dataTag = tag;
        }
    }
    unsigned long blockOffset = x + z*16 + y*256;
    unsigned long dataByteOffset = blockOffset/2;
    unsigned char dataByteHalf = (blockOffset % 2 == 0) ? 0 : 1;
    unsigned char blockIdChar = ((unsigned char*)(blocksTag.payload))[blockOffset];
    unsigned short blockId = (unsigned short)blockIdChar;
    if(hasAdd)
    {
        unsigned char addByte = ((unsigned char*)(addTag.payload))[dataByteOffset];
        addByte = (addByte >> (4*dataByteHalf)) & 0x0F;
        unsigned short addVal = addByte << 8;
        blockId += addVal;
    }
    unsigned char dataByte = ((unsigned char*)(dataTag.payload))[dataByteOffset];
    dataByte = (dataByte >> (4*dataByteHalf)) & 0x0F;
    blockData_t block;
    block.id = blockId;
    block.meta = dataByte;
    return block;

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
    chunk->_compressedDataExists = 1;
}

void freeChunk(chunkRec_t* chunk)
{
    if(chunk->_compressedDataExists)
    {
        free(chunk->chunkDataCompressed);
    }
    if(chunk->_uncompressedDataExists)
    {
        free(chunk->chunkDataUncompressed);
    }
    if(chunk->_nbtDataExists)
    {
        freeTag(&(chunk->chunkData));
    }
}