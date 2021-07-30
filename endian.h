#ifndef ANVILLIB_ENDIAN_H

void _encodeBigEndianNumber(unsigned char* bytes, long number, char size);

unsigned int readUnsignedBigEndian(unsigned char* data, char size);

int _decodeBigEndian(unsigned char* data, char size);

// tests system endianness by looking at the lower-addressed byte of a short containing the value 256 (0x0100)
// this byte will be 1 in big-endian systems and 0 in little-endian systems
char _testSystemEndianness();

#define ANVILLIB_ENDIAN_H

#endif //ANVILLIB_ENDIAN_H
