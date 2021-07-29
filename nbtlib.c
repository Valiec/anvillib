#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nbtlib.h"

/*
Tag types:

0: Tag_End
1: Tag_Byte
2: Tag_Short
3: Tag_Int
4: Tag_Long
5: Tag_Float
6: Tag_Double
7: Tag_ByteArray
8: Tag_String
9: Tag_List
10: Tag_Compound
11: Tag_IntArray
12: Tag_LongArray

*/

unsigned char* _resizeBuf(unsigned char* block, unsigned long size, unsigned long long step)
{
    unsigned char* newBlock = malloc(sizeof(char)*(size+step));
    int i;
    for(i=0; i<size; i++)
    {
        newBlock[i] = block[i];
    }
    free(block);
    return newBlock;
}

nbtTag_t* _resizeBufNbt(nbtTag_t* block, unsigned long size, unsigned long long step)
{
    nbtTag_t* newBlock = malloc(sizeof(nbtTag_t)*(size+step));
    int i;
    for(i=0; i<size; i++)
    {
        newBlock[i] = block[i];
    }
    free(block);
    return newBlock;
}

int _decodeBigEndian(unsigned char* data, char size)
{
    int value = 0;
    char i;
    for(i=0; i<size; i++)
    {
        char theByte = *(data+i);
        //printf("%u/", theByte);
        value = value * 256; //left shift last byte
        value |= theByte;
    }
    //printf("\n Value is: %u\n", value);
    return value;
}

// tests system endianness by looking at the lower-addressed byte of a short containing the value 256 (0x0100)
// this byte will be 1 in big-endian systems and 0 in little-endian systems
char _testSystemEndianness()
{
    short testValue = 256;
    char testResult = *(((char*)(&testValue)));
    return testResult;
}

char getByteTagValue(nbtTag_t tag)
{
    return *((char *)(tag.payload));
}

short getShortTagValue(nbtTag_t tag)
{
    return *((short *)(tag.payload));
}

int getIntTagValue(nbtTag_t tag)
{
    return *((int *)(tag.payload));
}

long getLongTagValue(nbtTag_t tag)
{
    return *((long *)(tag.payload));
}

float getFloatTagValue(nbtTag_t tag)
{
    return *((float *)(tag.payload));
}

double getDoubleTagValue(nbtTag_t tag)
{
    return *((double *)(tag.payload));
}

char* getByteArrayTagValue(nbtTag_t tag)
{
    return *((char **)(tag.payload));
}

char* getStringTagValue(nbtTag_t tag)
{
    return *((char **)(tag.payload));
}

nbtTag_t* getListTagValue(nbtTag_t tag)
{
    return *((nbtTag_t**)(tag.payload));
}

nbtTag_t* getCompoundTagValue(nbtTag_t tag)
{
    return *((nbtTag_t**)(tag.payload));
}

int* getIntArrayTagValue(nbtTag_t tag)
{
    return *((int**)(tag.payload));
}

long* getLongArrayTagValue(nbtTag_t tag)
{
    return *((long**)(tag.payload));
}

void _encodeBigEndianNumber(unsigned char* bytes, long number, char size)
{
    char i = size-1;
    while(i>=0)
    {
        bytes[i] = number & 255;
        number = number/256;
        i--;
    }
}

void _encodeBasicTagInfo(unsigned char* bytes, nbtTag_t* tag)
{
    bytes[0] = tag->typeId;
    _encodeBigEndianNumber(bytes+1, (long)(tag->nameLen), 2); //copy size
    memcpy(bytes+3, tag->name, tag->nameLen); //copy name
}

unsigned char* _encodeTag(nbtTag_t* tag, long long* length, char isNamed)
{
    unsigned char* bytes;
    int nameOffset = isNamed ? 3+tag->nameLen : 0;
    long long byteslen = 0;
    int i;
    int z;
    switch(tag->typeId)
    {
        case 1:
            /*printf("Found Tag_Byte");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", value: %u\n", getByteTagValue(*tag));*/
            bytes = malloc(sizeof(char)*(nameOffset+1));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            bytes[nameOffset] = getByteTagValue(*tag);
            byteslen = nameOffset+1;
            break;
        case 2:
            /*printf("Found Tag_Short");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", value: %u\n", getShortTagValue(*tag));*/
            bytes = malloc(sizeof(char)*(nameOffset+2));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            byteslen = nameOffset+2;
            _encodeBigEndianNumber(bytes+(nameOffset), (long)(getShortTagValue(*tag)), 2);
            break;
        case 3:
            /*printf("Found Tag_Int");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", value: %u\n", getIntTagValue(*tag));*/
            bytes = malloc(sizeof(char)*(nameOffset+4));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            byteslen = nameOffset+4;
            _encodeBigEndianNumber(bytes+(nameOffset), (long)(getIntTagValue(*tag)), 4);
            break;
        case 4:
            /*printf("Found Tag_Long");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", value: %lu\n", getLongTagValue(*tag));*/
            bytes = malloc(sizeof(char)*(nameOffset+8));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            byteslen = nameOffset+8;
            _encodeBigEndianNumber(bytes+(nameOffset), (long)(getLongTagValue(*tag)), 8);
            break;
        case 5:
            /*printf("Found Tag_Float");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", value: %f\n", getFloatTagValue(*tag));*/
            bytes = malloc(sizeof(char)*(nameOffset+4));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            byteslen = nameOffset+4;
            unsigned char* floatBytes = (unsigned char *)(tag->payload);
            memcpy(bytes+(nameOffset), floatBytes, 4);
            if(!_testSystemEndianness()) //little endian
            {
                //reverse byte order
                char i;
                for(i=0; i<4; i++)
                {
                    char tmp;
                    tmp = bytes[(nameOffset)+i];
                    bytes[(nameOffset)+i] = bytes[(nameOffset)+(3-i)];
                    bytes[(nameOffset)+(3-i)] = tmp;
                }
            }
            break;
        case 6:
            /*printf("Found Tag_Double");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", value: %lf\n", getDoubleTagValue(*tag));*/
            bytes = malloc(sizeof(char)*(nameOffset+8));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            unsigned char* doubleBytes = (unsigned char *)(tag->payload);
            memcpy(bytes+(nameOffset), doubleBytes, 8);
            byteslen = nameOffset+8;
            if(!_testSystemEndianness()) //little endian
            {
                //reverse byte order
                char i;
                for(i=0; i<8; i++)
                {
                    char tmp;
                    tmp = bytes[(nameOffset)+i];
                    bytes[(nameOffset)+i] = bytes[(nameOffset)+(7-i)];
                    bytes[(nameOffset)+(7-i)] = tmp;
                }
            }
            break;
        case 7:
            /*printf("Found Tag_ByteArray");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", payloadLength: %llu, ", tag->payloadLength);
            for(z=0; z<tag->payloadLength; z++)
            {
                //printf("%u ", ((char*)tag->payload)[z]);
            }
            printf("\n");*/
            bytes = malloc(sizeof(char)*(nameOffset+4+(tag->payloadLength)));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            _encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 4);
            //printf("length: %llu\n", tag->payloadLength);
            int iii;
            for(iii=0;iii<tag->payloadLength; iii++)
            {
                //printf("%u ", (((char*)tag->payload)[iii]) & 256);
            }
            //printf("\n");
            memcpy(bytes+(nameOffset+4), tag->payload, tag->payloadLength); //copy data
            for(iii=0;iii<tag->payloadLength; iii++)
            {
                //printf("%u ", *(bytes+(nameOffset+4+iii)));
            }
            //("\n");
            byteslen = nameOffset+4+(tag->payloadLength);
            break;
        case 8:
            /*printf("Found Tag_String");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", payloadLength: %llu, ", tag->payloadLength);
            printf("%s", (char*)tag->payload);
            printf("\n");*/
            bytes = malloc(sizeof(char)*(nameOffset+2+tag->payloadLength));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            _encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 2);
            memcpy(bytes+(nameOffset+2), tag->payload, tag->payloadLength); //copy data
            byteslen = nameOffset+2+(tag->payloadLength);
            break;
        case 9:
            /*printf("Found Tag_List");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", payloadLength: %llu\n", tag->payloadLength);*/
            bytes = malloc(sizeof(char)*(nameOffset)+5); //alloc space for name, type and len only
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            bytes[nameOffset] = tag->payloadTagType;
            _encodeBigEndianNumber(bytes+(nameOffset+1), tag->payloadLength, 4);
            byteslen = nameOffset+5;
            for(i=0; i<tag->payloadLength; i++)
            {
                long long listElementLength = 0;
                unsigned char* element = _encodeTag(&(((nbtTag_t *)(tag->payload))[i]), &listElementLength, 0);
                bytes = _resizeBuf(bytes, byteslen, listElementLength);
                memcpy(bytes+byteslen, element, listElementLength);
                byteslen += listElementLength;
                free(element);
            }
            //printf("End Tag_List\n");
            break;
        case 10:
            /*printf("Found Tag_Compound");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", payloadLength: %llu\n", tag->payloadLength);*/
            bytes = malloc(sizeof(char)*(nameOffset)); //alloc space for name only
            byteslen = nameOffset;
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            for(i=0; i<tag->payloadLength; i++)
            {
                long long compoundElementLength = 0;
                unsigned char* element = _encodeTag(&(((nbtTag_t *)(tag->payload))[i]), &compoundElementLength, 1);
                bytes = _resizeBuf(bytes, byteslen, ((i == tag->payloadLength-1) ? compoundElementLength+1 : compoundElementLength));
                memcpy(bytes+byteslen, element, compoundElementLength);
                if(i == tag->payloadLength-1) //if last element add space for Tag_End
                {
                    compoundElementLength++;
                }
                byteslen += compoundElementLength;
                if(i == tag->payloadLength-1) //if last element add Tag_End
                {
                    bytes[byteslen-1] = 0; //add Tag_End
                }
                free(element);
            }
            //printf("End Tag_Compound\n");
            break;
        case 11:
            /*printf("Found Tag_IntArray");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", payloadLength: %llu, ", tag->payloadLength);
            for(z=0; z<tag->payloadLength; z++)
            {
                //printf("%u ", ((int*)tag->payload)[z]);
            }
            printf("\n");*/
            bytes = malloc(sizeof(char)*(nameOffset+4+(tag->payloadLength*4)));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            _encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 4);
            for(i=0; i<tag->payloadLength; i++)
            {
                _encodeBigEndianNumber(bytes+(nameOffset+4+(i*4)), ((int *)(tag->payload))[i], 4);
            }
            byteslen = nameOffset+4+(tag->payloadLength*4);
            break;
        case 12:
            /*printf("Found Tag_IntArray");
            if(tag->isNamed && tag->nameLen > 0) {
                printf(", name: %s", tag->name);
            }
            printf(", payloadLength: %llu, ", tag->payloadLength);
            for(z=0; z<tag->payloadLength; z++)
            {
                //printf("%lu ", ((long*)tag->payload)[z]);
            }
            printf("\n");*/
            bytes = malloc(sizeof(char)*(nameOffset+4+(tag->payloadLength*8)));
            if(isNamed)
            {
                _encodeBasicTagInfo(bytes, tag);
            }
            _encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 4);
            for(i=0; i<tag->payloadLength; i++)
            {
                _encodeBigEndianNumber(bytes+(nameOffset+4+(i*8)), ((long *)(tag->payload))[i], 8);
            }
            byteslen = nameOffset+4+(tag->payloadLength*8);
            break;
        default:
            fprintf(stderr, "nbtlib: error: unrecognized NBT tag type %u\n", tag->typeId);
            return NULL;
    }
    //printf("====================================\n");
    (*length)+=byteslen;
    return bytes;
}

unsigned char* encodeTag(nbtTag_t* tag, long long* length)
{
    (*length) = 0;
    return _encodeTag(tag, length, 1);
}

nbtTag_t _decodeTag(unsigned char* bytes, char isNamed, char tagType, long long* length)
{
    *length = 0;
    nbtTag_t tag;
    tag.typeId = -1; //placeholder for errors
    tag.isNamed = isNamed;

    if(tagType<0)
    {
        tagType = bytes[0];
        (*length)++;
        bytes++; //advance start pos
    }
    if(isNamed && tagType != 0)
    {
        tag.nameLen = _decodeBigEndian(bytes, 2);
        bytes+=2;
        (*length)+=2;
        if(tag.nameLen > 0) {
            tag.name = malloc(sizeof(char)*tag.nameLen+1);
            memcpy(tag.name, bytes, tag.nameLen);
            tag.name[tag.nameLen] = 0;
            bytes += tag.nameLen;
            (*length) += tag.nameLen;
        } else {
            tag.name = malloc(1*sizeof(char)); //set name to something that can be freed
        }
    }
    int i;
    tag.typeId = tagType;
    switch(tagType)
    {
        case 0:
            //printf("Found Tag_End\n");
            ; //empty statement because you can't start a case with a declaration
            break;
        case 1:
            //printf("Found Tag_Byte\n");
            ; //empty statement because you can't start a case with a declaration
            char* dataChar = malloc(1*sizeof(char));
            *dataChar = bytes[0];
            (*length)++;
            tag.payload = (void*)(dataChar);
            break;
        case 2:
            //printf("Found Tag_Short\n");
            ; //empty statement because you can't start a case with a declaration
            short* dataShort = malloc(2*sizeof(char));
            *dataShort = (short)_decodeBigEndian(bytes, 2);
            (*length)+=2;
            tag.payload = (void*)(dataShort);
            break;
        case 3:
            //printf("Found Tag_Int\n");
            ; //empty statement because you can't start a case with a declaration
            int* dataInt = malloc(4*sizeof(char));
            *dataInt = (int)_decodeBigEndian(bytes, 4);
            (*length)+=4;
            tag.payload = (void*)(dataInt);
            break;
        case 4:
            //printf("Found Tag_Long\n");
            ; //empty statement because you can't start a case with a declaration
            long* dataLong = malloc(8*sizeof(char));
            *dataLong = (long)_decodeBigEndian(bytes, 8);
            (*length)+=8;
            tag.payload = (void*)(dataLong);
            break;
        case 5:
            //printf("Found Tag_Float\n");
            ; //empty statement because you can't start a case with a declaration
            unsigned char* dataRawFloat = malloc(4*sizeof(char));
            memcpy(dataRawFloat, bytes, 4);
            if(!_testSystemEndianness()) //little endian
            {
                //reverse byte order
                char i;
                for(i=0; i<4; i++)
                {
                    unsigned char tmp;
                    tmp = dataRawFloat[i];
                    dataRawFloat[i] = dataRawFloat[3-i];
                    dataRawFloat[3-i] = tmp;
                }
            }
            float* dataFloat = malloc(sizeof(float));
            *dataFloat = *((double *)(dataRawFloat));
            free(dataRawFloat);
            (*length)+=4;
            tag.payload = (void*)(dataFloat);
            break;
        case 6:
            //printf("Found Tag_Double\n");
            ; //empty statement because you can't start a case with a declaration
            unsigned char* dataRawDouble = malloc(8*sizeof(char));
            memcpy(dataRawDouble, bytes, 8);
            if(!_testSystemEndianness()) //little endian
            {
                //reverse byte order
                char i;
                for(i=0; i<8; i++)
                {
                    unsigned char tmp;
                    tmp = dataRawDouble[i];
                    dataRawDouble[i] = dataRawDouble[7-i];
                    dataRawDouble[7-i] = tmp;
                }
            }
            double* dataDouble = malloc(sizeof(double));
            *dataDouble = *((double *)(dataRawDouble));
            free(dataRawDouble);
            (*length)+=8;
            tag.payload = (void*)(dataDouble);
            break;
        case 7:
            //printf("Found Tag_ByteArray\n");
            tag.payloadLength = (int)(_decodeBigEndian(bytes, 4));
            bytes+=4;
            (*length)+=4;
            //printf("Length: %lld\n", tag.payloadLength);
            char* data = malloc(tag.payloadLength*sizeof(char));
            memcpy(data, bytes, tag.payloadLength);
            (*length)+=tag.payloadLength;
            tag.payload = (void*)(data);
            break;
        case 8:
            //printf("Found Tag_String\n");
            tag.payloadLength = (int)(_decodeBigEndian(bytes, 2));
            bytes+=2;
            (*length)+=2;
            char* dataStr = malloc(tag.payloadLength*sizeof(char)+1);
            memcpy(dataStr, bytes, tag.payloadLength);
            dataStr[tag.payloadLength] = 0; //null-terminate
            (*length)+=tag.payloadLength;
            //printf("Content: %s\n", (char *)(dataStr));
            tag.payload = (void*)(dataStr);
            break;
        case 9:
            //printf("Found Tag_List\n")
            ; //empty statement because you can't start a case with a declaration
            char payloadType = bytes[0];
            tag.payloadTagType = payloadType;
            bytes++;
            (*length)++;
            tag.payloadLength = (int)(_decodeBigEndian(bytes, 4));
            bytes+=4;
            (*length)+=4;
            nbtTag_t* dataTags = malloc(tag.payloadLength*sizeof(nbtTag_t));
            //printf("Length: %lld\n", tag.payloadLength);
            if(tag.payloadLength > 0)
            {
                for(i=0; i<tag.payloadLength; i++)
                {
                    long long elementLen = 0;
                    dataTags[i] = _decodeTag(bytes, 0, payloadType, &elementLen);
                    bytes+=elementLen;
                    (*length)+=elementLen;
                }
            }
            else
            {
                if(bytes[0] == 0) //handle empty-lists
                {
                    //bytes++;
                    //(*length)++;
                    //printf("Blank TagList!\n");
                }
            }
            tag.payload = (void*)(dataTags);
            break;
        case 10:
            //printf("@@@@@@@@@@@@@@@@ Found Tag_Compound, interior nesting level will be %u\n", nestingLevel+1);
            ; //empty statement because you can't start a case with a declaration
            nbtTag_t* dataCompound = malloc(4*sizeof(nbtTag_t));
            long long dataBufLen = 4;
            i = 0;
            while(1)
            {
                long long elementLen = 0;
                nbtTag_t newTag = _decodeTag(bytes, 1, -1, &elementLen);
                bytes+=elementLen;
                (*length)+=elementLen;
                if(newTag.typeId == 0)
                {
                    //printf("@@@@@@@@@@@@@@@@ Tag_Compound done! Nesting level back to %u\n", nestingLevel);
                    break;
                }
                else
                {
                    //printf("Tag found! type: %u, length: %llu\n", newTag.typeId, *length);
                    if(i>=dataBufLen)
                    {
                        dataCompound = _resizeBufNbt(dataCompound, dataBufLen, 4);
                        dataBufLen+=4;
                    }
                    dataCompound[i] = newTag;
                }
                i++;
            }
            tag.payloadLength = i;
            tag.payload = (void*)(dataCompound);
            break;
        case 11:
            //printf("Found Tag_IntArray\n");
            tag.payloadLength = (int)_decodeBigEndian(bytes, 4);
            bytes+=4;
            (*length)+=4;
            //printf("Length: %lld\n", tag.payloadLength);
            int* dataIntArr = malloc(tag.payloadLength*sizeof(char)*4);
            for(i=0; i<tag.payloadLength; i++)
            {
                dataIntArr[i] = _decodeBigEndian(bytes, 4);
                bytes+=4;
                (*length)+=4;
            }
            tag.payload = (void*)(dataIntArr);
            break;
        case 12:
            //printf("Found Tag_LongArray\n");
            tag.payloadLength = (int)_decodeBigEndian(bytes, 4);
            bytes+=4;
            (*length)+=4;
            //printf("Length: %lld\n", tag.payloadLength);
            long* dataLongArr = malloc(tag.payloadLength*sizeof(char)*8);
            for(i=0; i<tag.payloadLength; i++)
            {
                dataLongArr[i] = _decodeBigEndian(bytes, 8);
                bytes+=8;
                (*length)+=8;
            }
            tag.payload = (void*)(dataLongArr);
            break;
        default:
            //fprintf(stderr, "nbtlib: error: unrecognized NBT tag type %u\n", tagType);
            tag.typeId = -1;
            return tag;
    }
    if(tag.typeId > 12 || tag.typeId < 0)
    {
        printf("BAD TAG!!! Type: %d\n", tag.typeId);
    }
    //printf("Tag done! %llu/%llu, nesting level: %u, in list: %u\n", *length, totalLen, nestingLevel, inList);
    return tag;
}

nbtTag_t decodeTag(unsigned char* bytes)
{
    long long length = 0;
    return _decodeTag(bytes, 1, -1, &length); //-1 means tag type undetermined
}

void freeTag(nbtTag_t* tag)
{
    //printf("attempting to free tag of type %d\n", tag->typeId);
    if(tag->typeId > 0)
    {
        if (tag->isNamed)
        {
            free(tag->name);
        }
        if (tag->typeId == 9 || tag->typeId == 10) //list or compound tag
        {
            int i;
            for (i = 0; i < tag->payloadLength; i++)
            {
                freeTag(((nbtTag_t *)(tag->payload))+i);
            }
        }
        free(tag->payload);
    }
}