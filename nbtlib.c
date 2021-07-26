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
	char testResult = *(((char*)(&test)));
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

void _encodeBigEndianNumber(char* bytes, long number, char size)
{
	char i = size-1;
	while(i>0)
	{
		bytes[i] = number & 255;
		number = number >> 8;
		i--;
	}
}

void _encodeBasicTagInfo(char* bytes, nbtTag_t* tag)
{
	bytes[0] = tag->typeId;
	_encodeBigEndianNumber(bytes+1, (long)(tag->nameLen), 2); //copy size
	memcpy(bytes+3, tag->name, tag->nameLen); //copy name
}

char* _encodeTag(nbtTag_t* tag, long long* length, char isNamed)
{
	char* bytes;
	int nameOffset = isNamed ? 3+tag->nameLen : 0;
	switch(tag->type)
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
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(getShortTagValue(tag)), 2);
		case 3:
			bytes = malloc(sizeof(char)*(nameOffset+4));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(getShortTagValue(tag)), 4);
		case 4:
			bytes = malloc(sizeof(char)*(nameOffset+8));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			_encodeBigEndianNumber(bytes+(nameOffset), (long)(getShortTagValue(tag)), 8);
		case 5:
			bytes = malloc(sizeof(char)*(nameOffset+4));
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			unsigned char* floatBytes = (char *)(tag->payload);
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
			unsigned char* doubleBytes = (char *)(tag->payload);
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
			long long byteslen = nameOffset+4;
			int i;
			for(i=0; i<tag->payloadLength; i++)
			{
				long long length;
				element = _encodeTag(tag->payload[i], length, 0);
				bytes = _resizeBuf(bytes, byteslen, length);
				memcpy(bytes+byteslen, element, length);
				byteslen += length;
			}
		case 10:
			bytes = malloc(sizeof(char)*(nameOffset)); //alloc space for name only
			long long byteslen = nameOffset;
			if(isNamed)
			{
				_encodeBasicTagInfo(bytes, tag);
			}
			int i;
			for(i=0; i<tag->payloadLength; i++)
			{
				long long length;
				element = _encodeTag(tag->payload[i], length, 1);
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
			int i;
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
			int i;
			for(i=0; i<tag->payloadLength; i++)
			{
				_encodeBigEndianNumber(bytes+(nameOffset+4+(i*8)), ((long *)(tag->payload))[i], 8);
			}
		default:
			fprintf(stderr, "nbtlib: error: unrecognized NBT tag type %u\n", tag->type);
			return NULL;
	}
	return bytes;
}

char* encodeTag(nbtTag_t* tag, long long* length)
{
	return _encodeTag(tag, length, 1);
}

nbtTag_t _decodeTag(char* bytes, char isNamed, char tagType, long long* length)
{
	*length = 0;
	nbtTag_t tag;
	tag->isNamed = isNamed;
	if(tagType<0)
	{
		tagType = bytes[0];
		(*length)++;
	}
	bytes++; //advance start pos
	if(isNamed && tagType != 0)
	{
		tag->nameLen = _decodeBigEndian(bytes, 2);
		bytes+=2;
		(*length)+=2;
	}
	switch(tagType)
	{
		case 0:
			return tag;
		case 1:
			char data = bytes[0];
			(*length)++;
		case 2:
			short data = (short)_decodeBigEndian(bytes, 2);
			(*length)+=2;
		case 3:
			int data = (int)_decodeBigEndian(bytes, 4);
			(*length)+=4;
		case 4:
			long data = (long)_decodeBigEndian(bytes, 8);
			(*length)+=8;
		case 5:
			void* dataRaw = malloc(4*sizeof(char));
			memcpy(dataRaw, bytes, 4);
			if(!_testSystemEndianness()) //little endian
			{
				//reverse byte order
				char i;
				for(i=0; i<4; i++)
				{
					char tmp;
					tmp = dataRaw[i];
					dataRaw[i] = dataRaw[3-i];
					dataRaw[3-i] = tmp;
				}
			}
			float data = *((float *)(dataRaw));
			free(dataRaw);
			(*length)+=4;
		case 6:
			void* dataRaw = malloc(8*sizeof(char));
			memcpy(dataRaw, bytes, 8);
			if(!_testSystemEndianness()) //little endian
			{
				//reverse byte order
				char i;
				for(i=0; i<8; i++)
				{
					char tmp;
					tmp = dataRaw[i];
					dataRaw[i] = dataRaw[7-i];
					dataRaw[7-i] = tmp;
				}
			}
			float data = *((float *)(dataRaw));
			free(dataRaw);
			(*length)+=8;
		case 7:
			tag->payloadLength = (int)_decodeBigEndian(bytes, 4);
			bytes+=4;
			(*length)+=4;
			char* data = malloc(tag->payloadLength*sizeof(char));
			memcpy(data, bytes, tag->payloadLength);
			(*length)+=tag->payloadLength;
		case 8:
			tag->payloadLength = (int)_decodeBigEndian(bytes, 2);
			bytes+=2;
			char* data = malloc(tag->payloadLength*sizeof(char)+1);
			memcpy(data, bytes, tag->payloadLength);
			data[tag->payloadLength] = 0; //null-terminate
			(*length)+=tag->payloadLength;
		case 9:
			char payloadType = bytes[0];
			bytes++;
			(*length)++;
			tag->payloadLength = (int)_decodeBigEndian(bytes, 4);
			bytes+=4;
			(*length)+=4;
			nbtTag_t* data = malloc(tag->payloadLength*sizeof(nbtTag_t));
			int i;
			for(i=0; i<tag->payloadLength; i++)
			{
				long long elementLen = 0;
				data[i] = _decodeTag(bytes, 0, payloadType, &elementLen);
				bytes+=elementLen;
				(*length)+=elementLen;
			}
		case 10:
			char payloadType = bytes[0];
			bytes++;
			(*length)++;
			tag->payloadLength = (int)_decodeBigEndian(bytes, 4);
			bytes+=4;
			(*length)+=4;
			nbtTag_t* data = malloc(4*sizeof(nbtTag_t));
			long long dataBufLen = 4;
			int i = 0;
			while(1)
			{
				long long elementLen = 0;
				nbtTag_t newTag = _decodeTag(bytes, 1, -1, &elementLen);
				bytes+=elementLen;
				(*length)+=elementLen;
				if(newTag.tagType == 0)
				{
					break;
				}
				else
				{
					if(i>=dataBufLen)
					{
						data = _resizeBuf(data, dataBufLen, 4);
					}
					data[i] = newTag;
				}
				i++;
			}
			tag->payloadLength = i;
		case 11:
			tag->payloadLength = (int)_decodeBigEndian(bytes, 4);
			bytes+=4;
			int* data = malloc(tag->payloadLength*sizeof(char)*4);
			int i;
			for(i=0; i<tag->payloadLength; i++)
			{
				data[i] = _decodeBigEndian(bytes, 4);
				bytes+=4;
			}
		case 12:
			tag->payloadLength = (int)_decodeBigEndian(bytes, 4);
			bytes+=4;
			long* data = malloc(tag->payloadLength*sizeof(char)*8);
			int i;
			for(i=0; i<tag->payloadLength; i++)
			{
				data[i] = _decodeBigEndian(bytes, 8);
				bytes+=8;
			}
		default:
			fprintf(stderr, "nbtlib: error: unrecognized NBT tag type %u\n", tagType);
			return NULL;
	}
	return tag;
}

nbtTag_t decodeTag(char* bytes)
{
	long long length = 0;
	return _decodeTag(bytes, 0, -1, &length); //-1 means tag type undetermined
}