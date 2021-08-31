#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "nbtlib.h"
#include "anvillib.h"
#include "zlib.h"
#include <string.h>

int compareChunks (const void * c1, const void * c2) {
    chunkRec_t chunk1 = *((chunkRec_t *) c1);
    chunkRec_t chunk2 = *((chunkRec_t *) c2);
    if(chunk1.offset < chunk2.offset)
    {
        return -1;
    }
    else if(chunk1.offset > chunk2.offset)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int writeChunk(chunkRec_t chunk, chunkRec_t* header, int chunkIndex, FILE* fp)
{

    if(chunk._compressedDataOutdated)
    {
        return 1;
    }

    unsigned int chunkSize = (4096*chunk.size);
    unsigned int chunkSizeBlocks;
    unsigned long chunkOffset = 4096*chunk.offset;
    char found = 1;
    //printf("Writing chunk %u, size %u, blockSize %u, offset %lu, .offset %u, .size %u\n", chunkIndex, chunk.exactSize, chunkSize, chunkOffset, chunk.offset, chunk.size);
    if(chunkSize < chunk.exactSize+5)
    {
        //printf("too big!\n");
        found = 0;
        chunkSizeBlocks = chunk.exactSize/4096;
        if(chunk.exactSize%4096)
        {
            chunkSizeBlocks++;
        }
        chunkRec_t* chunksCopy = malloc(1024*sizeof(chunkRec_t));
        chunksCopy[chunkIndex].offset = 0;
        chunksCopy[chunkIndex].size = 0;
        memcpy(chunksCopy, header, 1024*sizeof(chunkRec_t));
        qsort(chunksCopy, 1024, sizeof(chunkRec_t), compareChunks);
        int i;
        for(i=1; i<1024; i++)
        {
            if(chunksCopy[i].size == 0 || chunksCopy[i-1].size == 0)
            {
                continue;
            }
            int blockSize = chunksCopy[i].offset-(chunksCopy[i-1].offset+chunksCopy[i-1].size);
            if(blockSize >= chunkSizeBlocks)
            {
                //found it
                chunk.offset = chunksCopy[i-1].offset+chunksCopy[i-1].size;
                found = 1;
                break;
            }
        }
        if(!found)
        {
            chunk.offset = chunksCopy[1023].offset+chunksCopy[1023].size; //place at EOF
        }
        chunk.size = chunkSizeBlocks;
        fseek(fp, chunkIndex*4, SEEK_SET);
        unsigned char* tmp = malloc(3);
        _encodeBigEndianNumber(tmp, chunk.offset, 3);
        fwrite(tmp, sizeof(char), 3, fp);
        _encodeBigEndianNumber(tmp, chunk.size, 1);
        fwrite(tmp, sizeof(char), 1, fp);
        free(tmp);
        free(chunksCopy);
    }
    fseek(fp, chunkOffset, SEEK_SET);
    unsigned int size = chunk.exactSize;
    unsigned char compressionType = 2;
    unsigned char* sizeEncoded = malloc(4);
    _encodeBigEndianNumber(sizeEncoded, size, 4);
    fwrite(sizeEncoded, sizeof(char), 4, fp);
    free(sizeEncoded);
    fwrite(&compressionType, sizeof(char), 1, fp);
    fwrite(chunk.chunkDataCompressed, sizeof(char), chunk.exactSize, fp);
    if(!found) //if chunk at end
    {
        printf("at end!\n");
        int i;
        unsigned char* zeroes = malloc(((chunk.size*4096)-chunk.exactSize)*sizeof(char));
        for(i=0; i<((chunk.size*4096)-chunk.exactSize); i++)
        {
            zeroes[i] = 0;
        }
        fwrite(zeroes, sizeof(char), (chunk.size*4096)-chunk.exactSize, fp);
        free(zeroes);
    }
    return 0;
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
    headerEntry._blockDataExists = 0;
    headerEntry._uncompressedDataOutdated = 0;
    headerEntry._compressedDataOutdated = 0;
    headerEntry._nbtDataOutdated = 0;
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
    //printf("resizing!\n");
    unsigned char* newBlock = malloc(sizeof(char)*(size+step));
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
        databuf = resizeBlock(databuf, buflen, chunk->exactSize*7);
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
    if(chunk->_nbtDataExists)
    {
        chunk->_nbtDataOutdated = 1;
    }
}

int deflateChunk(chunkRec_t* chunk)
{

    if(chunk->_uncompressedDataOutdated)
    {
        return 1;
    }

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
        chunk->exactSize += chunkSize;
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
    chunk->_compressedDataOutdated = 0;
    return 0;
}

void decodeChunkData(chunkRec_t* chunk)
{
    //printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    if(chunk->_nbtDataExists)
    {
        freeTag(&(chunk->chunkData));
    }
    chunk->chunkData = decodeTag(chunk->chunkDataUncompressed);
    chunk->_nbtDataExists = 1;
    chunk->_nbtDataOutdated = 1;
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
    chunk->_uncompressedDataOutdated = 0;
    chunk->_compressedDataOutdated = 1;
}


int initBlockData(chunkRec_t* chunk)
{

    if(chunk->size == 0) //chunk not present
    {
        return 1;
    }

    nbtTag_t rootTag = chunk->chunkData;
    int i, j;
    printf("root tag payload length is %lld\n", rootTag.payloadLength);
    nbtTag_t levelTag;
    for(i=0;i<rootTag.payloadLength; i++) //needed because mods can stick extra tags here
    {
        nbtTag_t tag = ((nbtTag_t*)(rootTag.payload))[i];
        printf("root tag child tag %d name is %s\n", i, tag.name);
        fflush(stdout);
        if(strcmp(tag.name, "Level") == 0)
        {
            levelTag = tag;
            break;
        }
    }
    nbtTag_t sectionsTag;

    printf("level tag payload length is %lld\n", levelTag.payloadLength);
    fflush(stdout);

    for(i=0;i<levelTag.payloadLength; i++)
    {
        nbtTag_t tag = ((nbtTag_t*)(levelTag.payload))[i];
        printf("level tag child tag %d name is %s\n", i, tag.name);
        fflush(stdout);

        if(strcmp(tag.name, "Sections") == 0)
        {
            sectionsTag = tag;
            break;
        }

        if(strcmp(tag.name, "Entities") == 0)
        {
            char* val = malloc(1);
            *val = 0;
            tag.payloadTagType = 1;
            tag.payloadLength = 0;
            tag.payload = val;
            tag.payload = (void *)val;
            ((nbtTag_t*)(levelTag.payload))[i] = tag;
        }

        if(strcmp(tag.name, "TileEntities") == 0)
        {
            char* val = malloc(1);
            *val = 0;
            tag.payloadTagType = 1;
            tag.payloadLength = 0;
            tag.payload = val;
            tag.payload = (void *)val;
            ((nbtTag_t*)(levelTag.payload))[i] = tag;
        }
    }
    nbtTag_t** blocksTags = malloc(16*sizeof(nbtTag_t *));
    nbtTag_t** addTags = malloc(16*sizeof(nbtTag_t *));
    nbtTag_t** dataTags = malloc(16*sizeof(nbtTag_t *));
    unsigned char* hasAdd = malloc(16*sizeof(char));
    unsigned char* sectionPresent = malloc(16*sizeof(char));
    for(i=0; i<16; i++)
    {
        sectionPresent[i] = 0;
        hasAdd[i] = 0;
    }
    for(i=0;i<sectionsTag.payloadLength; i++)
    {
        nbtTag_t sectionTag = ((nbtTag_t*)(sectionsTag.payload))[i];
        nbtTag_t yTag;
        for(j=0;j<sectionTag.payloadLength; j++)
        {
            nbtTag_t tag = ((nbtTag_t*)(sectionTag.payload))[j];
            if(strcmp(tag.name, "Y") == 0)
            {
                yTag = tag;
                break;
            }
        }
        unsigned char ind = (*((unsigned char*)(yTag.payload)));
        sectionPresent[ind] = 1;
        printf("section %u present\n", ind);
        for(j=0;j<sectionTag.payloadLength; j++)
        {
            nbtTag_t* tag = ((nbtTag_t*)(sectionTag.payload))+j;
            hasAdd[ind] = 0;
            if(strcmp(tag->name, "Blocks") == 0)
            {
                //printf("found blocks, writing at index %u\n", ind);
                blocksTags[ind] = tag;
            }
            if(strcmp(tag->name, "Add") == 0)
            {
                //printf("found add, writing at index %u\n", ind);
                addTags[ind] = tag;
                hasAdd[ind] = 1;
            }
            if(strcmp(tag->name, "Data") == 0)
            {
                //printf("found data, writing at index %u\n", ind);
                dataTags[ind] = tag;
            }
        }
    }
    chunk->_blockDataExists = 1;
    chunk->blockIds = blocksTags;
    chunk->blockAdd = addTags;
    chunk->blockMeta = dataTags;
    chunk->hasAdd = hasAdd;
    chunk->sectionPresent = sectionPresent;
    for(i = 0; i<16; i++)
    {
        printf("section %u listed as %u\n", i, chunk->sectionPresent[i]);
    }
    return 0;

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

    if(chunk._blockDataExists == 0) //block data not present
    {
        *exitcode = 1;
        return fail;
    }

    char sectionId = y/16;
    //printf("section id is %u\n", sectionId);
    if(!(chunk.sectionPresent[sectionId]))
    {
        blockData_t block;
        block.id = 0;
        block.meta = 0;
        return block;
    }

    unsigned long blockOffset = x + z*16 + (y%16)*256;
    unsigned long dataByteOffset = blockOffset/2;
    unsigned char dataByteHalf = (blockOffset % 2 == 0) ? 0 : 1;
    unsigned char blockIdChar = ((unsigned char*)(chunk.blockIds[sectionId]->payload))[blockOffset];
    unsigned short blockId = (unsigned short)blockIdChar;
    if(chunk.hasAdd[sectionId])
    {
        unsigned char addByte = ((unsigned char*)(chunk.blockAdd[sectionId]->payload))[dataByteOffset];
        addByte = (addByte >> (4*dataByteHalf)) & 0x0F;
        unsigned short addVal = addByte << 8;
        blockId += addVal;
    }
    unsigned char dataByte = ((unsigned char*)(chunk.blockMeta[sectionId]->payload))[dataByteOffset];
    dataByte = (dataByte >> (4*dataByteHalf)) & 0x0F;
    blockData_t block;
    block.id = blockId;
    block.meta = dataByte;
    return block;

}

//sets a block ID and meta given a chunk, block coords within that chunk (0<x<16, 0<z<16, 0<y<256), and a blockData_t
//does not validate the block coords make any sense
int setBlockIdInChunk(chunkRec_t* chunk, int x, int y, int z, blockData_t newBlock)
{

    if((x<0||x>15) || (z<0||z>15) || (y<0||x>256)) //bad coords
    {
        return 1;
    }

    if(chunk->size == 0) //chunk not present
    {
        return 1;
    }

    if(chunk->_blockDataExists == 0) //block data not present
    {
        return 1;
    }

    char sectionId = y/16;

    unsigned long blockOffset = x + z*16 + (y%16)*256;
    unsigned long dataByteOffset = blockOffset/2;
    unsigned char dataByteHalf = (blockOffset % 2 == 0) ? 0 : 1;
    unsigned char blockIdChar = newBlock.id & 255;
    unsigned char blockAddNibble = (newBlock.id & 0x0F00)/256;
    //printf("block id: %u, add: %u\n", blockIdChar, blockAddNibble);
    ((unsigned char*)(chunk->blockIds[sectionId]->payload))[blockOffset] = blockIdChar;
    if(blockAddNibble > 0)
    {
        if(!chunk->hasAdd[sectionId])
        {
            //init add
        }

        unsigned char addByte = ((unsigned char*)(chunk->blockAdd[sectionId]->payload))[dataByteOffset];
        addByte &= (dataByteHalf ? 0x0F : 0xF0);
        addByte |= (blockAddNibble >> (4*dataByteHalf));
        ((unsigned char*)(chunk->blockAdd[sectionId]->payload))[dataByteOffset] = addByte;
    }
    unsigned char metaByte = ((unsigned char*)(chunk->blockMeta[sectionId]->payload))[dataByteOffset];
    metaByte &= (dataByteHalf ? 0x0F : 0xF0);
    metaByte |= (newBlock.meta >> (4*dataByteHalf));
    ((unsigned char*)(chunk->blockMeta[sectionId]->payload))[dataByteOffset] = metaByte;
    chunk->_uncompressedDataOutdated = 1;
    chunk->_compressedDataOutdated = 1;
    return 0;

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
    if(chunk->_blockDataExists)
    {
        free(chunk->blockMeta);
        free(chunk->blockIds);
        free(chunk->blockAdd);
        free(chunk->hasAdd);
        free(chunk->sectionPresent);
    }
}