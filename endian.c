#include "endian.h"

//utils for handling endianness

int _decodeBigEndian(unsigned char* data, char size)
{
    int value = 0;
    char i;
    for(i=0; i<size; i++)
    {
        unsigned char theByte = *(data+i);
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

unsigned int readUnsignedBigEndian(unsigned char* data, char size)
{
    unsigned int value = 0;
    char i;
    for(i=0; i<size; i++)
    {
        unsigned char theByte = *(data+i);
        //printf("%u/", theByte);
        value = value * 256; //left shift last byte
        value |= theByte;
    }
    //printf("\n Value is: %u\n", value);
    return value;
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