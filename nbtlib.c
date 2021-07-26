#include <stdio.h>
#include <stdlib.h>
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

char* encodeTag(nbtTag_t* tag)
{
	char* bytes;

	switch(tag->type)
	{
		case 1:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+1));
		case 2:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+2));
		case 3:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+4));
		case 4:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+8));
		case 5:
			//???
		case 6:
			//???
		case 7:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+(tag->payloadLength)));
		case 8:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+(tag->payloadLength)));
		case 9:
			//to handle
		case 10:
			//to handle
		case 11:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+(tag->payloadLength*4)));
		case 12:
			bytes = malloc(sizeof(char)*(3+tag->nameLen+(tag->payloadLength*8)));
		default:
			fprintf(stderr, "nbtlib: error: unrecognized NBT tag type %u\n", tag->type);
	}
}

//note: stops at the end of the first tag in bytes
nbtTag_t decodeTag(char* bytes)
{

}