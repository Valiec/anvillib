#ifndef ANVILLIB_NBTLIB_H
#define ANVILLIB_NBTLIB_H

struct nbtTag {
    char typeId;
    char isNamed;
    unsigned short nameLen;
    char* name;
    void* payload;
    char payloadTagType;
    unsigned long long payloadLength; //used for Tag_ByteArray, Tag_String, Tag_List, Tag_Compound, Tag_IntArray, and Tag_LongArray
};

typedef struct nbtTag nbtTag_t;

char getByteTagValue(nbtTag_t tag);

short getShortTagValue(nbtTag_t tag);

int getIntTagValue(nbtTag_t tag);

long getLongTagValue(nbtTag_t tag);

float getFloatTagValue(nbtTag_t tag);

double getDoubleTagValue(nbtTag_t tag);

char* getByteArrayTagValue(nbtTag_t tag);

char* getStringTagValue(nbtTag_t tag);

nbtTag_t* getListTagValue(nbtTag_t tag);

nbtTag_t* getCompoundTagValue(nbtTag_t tag);

int* getIntArrayTagValue(nbtTag_t tag);

long* getLongArrayTagValue(nbtTag_t tag);

long* setTagName(nbtTag_t tag, unsigned char* name); //used to simplify not messing up the name length

unsigned char* encodeTag(nbtTag_t* tag, long long* length);

//note: stops at the end of the first tag in bytes
nbtTag_t decodeTag(unsigned char* bytes);

void freeTag(nbtTag_t* tag);

#endif //ANVILLIB_NBTLIB_H
