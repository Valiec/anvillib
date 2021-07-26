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

unsigned char* _resizeBuf(unsigned char* block, unsigned long size, unsigned long step)
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

int _decodeBigEndian(unsigned char* data, char size)
{
	int value = 0;
	char i;
	for(i=0; i<size; i++)
	{
		char theByte = *(data+i);
		//printf("%u/", theByte);
		value = value << 8; //left shift last byte
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
	while(i>0)
	{
		bytes[i] = number & 255;
		number = number >> 8;
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
	long long byteslen;
	int i;
	switch(tag->typeId)
	{
		case 1:
			bytes = malloc(sizeof(char)*(nameOffset+1));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			bytes[nameOffset] = getByteTagValue(*tag);
		case 2:
			bytes = malloc(sizeof(char)*(nameOffset+2));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(getShortTagValue(*tag)), 2);
		case 3:
			bytes = malloc(sizeof(char)*(nameOffset+4));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(getShortTagValue(*tag)), 4);
		case 4:
			bytes = malloc(sizeof(char)*(nameOffset+8));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(getShortTagValue(*tag)), 8);
		case 5:
			bytes = malloc(sizeof(char)*(nameOffset+4));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
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
		case 6:
			bytes = malloc(sizeof(char)*(nameOffset+8));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			unsigned char* doubleBytes = (unsigned char *)(tag->payload);
			memcpy(bytes+(nameOffset), doubleBytes, 8);
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
		case 7:
			bytes = malloc(sizeof(char)*(nameOffset+(tag->payloadLength)));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 4);
			memcpy(bytes+(nameOffset+4), tag->payload, tag->payloadLength); //copy data
		case 8:
			bytes = malloc(sizeof(char)*(nameOffset+(tag->payloadLength)));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 2);
			memcpy(bytes+(nameOffset+2), tag->payload, tag->payloadLength); //copy data
		case 9:
			bytes = malloc(sizeof(char)*(nameOffset)+4); //alloc space for name only
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 4);
			byteslen = nameOffset+4;
			for(i=0; i<tag->payloadLength; i++)
			{
				long long length;
				unsigned char* element = _encodeTag(&(tag->payload[i]), &length, 0);
				bytes = _resizeBuf(bytes, byteslen, length);
				memcpy(bytes+byteslen, element, length);
				byteslen += length;
			}
		case 10:
			bytes = malloc(sizeof(char)*(nameOffset)); //alloc space for name only
			byteslen = nameOffset;
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			for(i=0; i<tag->payloadLength; i++)
			{
				long long length;
				unsigned char* element = _encodeTag(&(tag->payload[i]), &length, 1);
				if(i == tag->payloadLength-1) //if last element add space for Tag_End
				{
					length++;
				}
				bytes = _resizeBuf(bytes, byteslen, length);
				memcpy(bytes+byteslen, element, length);
				byteslen += length;
			}
			bytes[byteslen-1] = 0; //add Tag_End
		case 11:
			bytes = malloc(sizeof(char)*(nameOffset+(tag->payloadLength*4)));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 4);
			for(i=0; i<tag->payloadLength; i++)
			{
				_encodeBigEndianNumber(bytes+(nameOffset+4+(i*4)), ((int *)(tag->payload))[i], 4);
			}
		case 12:
			bytes = malloc(sizeof(char)*(nameOffset+(tag->payloadLength*8)));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(tag->payloadLength), 4);
			for(i=0; i<tag->payloadLength; i++)
			{
				_encodeBigEndianNumber(bytes+(nameOffset+4+(i*8)), ((long *)(tag->payload))[i], 8);
			}
		default:
			fprintf(stderr, "nbtlib: error: unrecognized NBT tag type %u\n", tag->typeId);
			return NULL;
	}
	return bytes;
}

unsigned char* encodeTag(nbtTag_t* tag, long long* length)
{
	return _encodeTag(tag, length, 1);
}

void _printContext(unsigned char* bytes)
{
	int i;
	for(i=0;i<30;i++)
	{
		printf("%u ", bytes[i]);
	}
	printf("\n");
}

nbtTag_t _decodeTag(unsigned char* bytes, char isNamed, char tagType, long long* length, long long totalLen)
{
	*length = 0;
	nbtTag_t tag;
	tag.isNamed = isNamed;

	if(1)
	{
		printf("======= (%d)\n", tagType);
		_printContext(bytes);
	}

	if(tagType<0)
	{
		tagType = bytes[0];
		(*length)++;
		bytes++; //advance start pos
		printf("------- (%d)\n", tagType);
	}
	if(isNamed && tagType != 0)
	{
		tag.nameLen = _decodeBigEndian(bytes, 2);
		bytes+=2;
		(*length)+=2;
		tag.name = malloc(sizeof(char)*tag.nameLen+1);
		memcpy(tag.name, bytes, tag.nameLen);
		tag.name[tag.nameLen] = 0;
		bytes+=tag.nameLen;
		(*length)+=tag.nameLen;
		printf("Named: %s\n", tag.name);
	}
	else
	{
		printf("Not Named!\n");	
	}
	int i;
	tag.typeId = tagType;
	switch(tagType)
	{
		case 0:
			printf("Found Tag_End\n");
			break;
		case 1:
			printf("Found Tag_Byte\n");
			char dataChar = bytes[0];
			(*length)++;
			break;
		case 2:
			printf("Found Tag_Short\n");
			short dataShort = (short)_decodeBigEndian(bytes, 2);
			(*length)+=2;
			break;
		case 3:
			printf("Found Tag_Int\n");
			int dataInt = (int)_decodeBigEndian(bytes, 4);
			(*length)+=4;
			break;
		case 4:
			printf("Found Tag_Long\n");
			long dataLong = (long)_decodeBigEndian(bytes, 8);
			(*length)+=8;
			break;
		case 5:
			printf("Found Tag_Float\n");
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
			float dataFloat = *((float *)(dataRawFloat));
			free(dataRawFloat);
			(*length)+=4;
			break;
		case 6:
			printf("Found Tag_Double\n");
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
			float dataDouble = *((float *)(dataRawDouble));
			free(dataRawDouble);
			(*length)+=8;
			break;
		case 7:
			printf("Found Tag_ByteArray\n");
			tag.payloadLength = (int)(_decodeBigEndian(bytes, 4));
			bytes+=4;
			(*length)+=4;
			printf("Length: %lld\n", tag.payloadLength);
			char* data = malloc(tag.payloadLength*sizeof(char));
			memcpy(data, bytes, tag.payloadLength);
			(*length)+=tag.payloadLength;
			break;
		case 8:
			printf("Found Tag_String\n");
			tag.payloadLength = (int)(_decodeBigEndian(bytes, 2));
			bytes+=2;
			(*length)+=2;
			char* dataStr = malloc(tag.payloadLength*sizeof(char)+1);
			memcpy(dataStr, bytes, tag.payloadLength);
			dataStr[tag.payloadLength] = 0; //null-terminate
			(*length)+=tag.payloadLength;
			printf("Content: %s\n", (char *)(dataStr));
			break;
		case 9:
			printf("Found Tag_List\n");
			char payloadType = bytes[0];
			bytes++;
			(*length)++;
			tag.payloadLength = (int)(_decodeBigEndian(bytes, 4));
			bytes+=4;
			(*length)+=4;
			nbtTag_t* dataTags = malloc(tag.payloadLength*sizeof(nbtTag_t));
			printf("Length: %lld\n", tag.payloadLength);
			if(tag.payloadLength > 0)
			{
				for(i=0; i<tag.payloadLength; i++)
				{
					long long elementLen = 0;
					dataTags[i] = _decodeTag(bytes, 0, payloadType, &elementLen, totalLen);
					bytes+=elementLen;
					(*length)+=elementLen;
				}
			}
			else
			{
				if(bytes[0] == 0) //handle empty-lists 
				{
					bytes++;
					(*length)++;
					printf("Blank TagList!\n");
				}
			}
			break;
		case 10:
			printf("Found Tag_Compound\n");
			nbtTag_t* dataCompound = malloc(4*sizeof(nbtTag_t));
			long long dataBufLen = 4;
			i = 0;
			while(1)
			{
				long long elementLen = 0;
				nbtTag_t newTag = _decodeTag(bytes, 1, -1, &elementLen, totalLen);
				bytes+=elementLen;
				(*length)+=elementLen;
				if(newTag.typeId == 0)
				{
					printf("Tag_Compound done!\n");	
					break;
				}
				else
				{
					printf("Tag found! type: %u, length: %llu\n", newTag.typeId, elementLen);		
					if(i>=dataBufLen)
					{
						dataCompound = (nbtTag_t *)(_resizeBuf((unsigned char *)dataCompound, dataBufLen, 4));
					}
					dataCompound[i] = newTag;
				}
				i++;
			}
			tag.payloadLength = i;
			break;
		case 11:
			printf("Found Tag_IntArray\n");
			tag.payloadLength = (int)_decodeBigEndian(bytes, 4);
			bytes+=4;
			(*length)+=4;
			printf("Length: %lld\n", tag.payloadLength);
			int* dataIntArr = malloc(tag.payloadLength*sizeof(char)*4);
			for(i=0; i<tag.payloadLength; i++)
			{
				dataIntArr[i] = _decodeBigEndian(bytes, 4);
				bytes+=4;
				(*length)+=4;
			}
			break;
		case 12:
			printf("Found Tag_LongArray\n");
			tag.payloadLength = (int)_decodeBigEndian(bytes, 4);
			bytes+=4;
			(*length)+=4;
			printf("Length: %lld\n", tag.payloadLength);
			long* dataLongArr = malloc(tag.payloadLength*sizeof(char)*8);
			for(i=0; i<tag.payloadLength; i++)
			{
				dataLongArr[i] = _decodeBigEndian(bytes, 8);
				bytes+=8;
				(*length)+=8;
			}
			break;
		default:
			fprintf(stderr, "nbtlib: error: unrecognized NBT tag type %u\n", tagType);
			tag.typeId = -1;
			return tag;
	}
	printf("Tag done! %llu/%llu\n", *length, totalLen);
	fflush(stdout);
	return tag;
}

nbtTag_t decodeTag(unsigned char* bytes, long long totalLen)
{
	long long length = 0;
	return _decodeTag(bytes, 1, -1, &length, totalLen); //-1 means tag type undetermined
}