#include "nbtlib.h"
#include <stdlib.h>
/*include <stdio.h>

int main(int argc, char** argv)
{
    printf("%s\n", argv[1]);
    return 0; //testing
}*/

void inflateFile(nbtFile_t* data)
{
    int returnVal;
    //unsigned long outputLen;
    unsigned long buflen = data->sizeCompressed*7*sizeof(char);
    unsigned char* databuf = malloc(data->sizeCompressed*7*sizeof(char));
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = data->sizeCompressed;
    stream.next_in = data->compressedData; //data to inflate
    returnVal = inflateInit2(&stream, 16+MAX_WBITS); //init inflate for gzip
    stream.next_out = databuf; //decompressed data
    stream.avail_out = buflen;
    data->sizeUncompressed = buflen;
    //printf("decompressing... return val: %d, avail_out: %d, avail_in: %d\n", returnVal, stream.avail_out, stream.avail_in);
    returnVal = inflate(&stream, Z_NO_FLUSH); //decompress
    while(returnVal != Z_STREAM_END)
    {
        //printf("decompressing... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
        data->sizeUncompressed += data->sizeCompressed*7*sizeof(char);
        databuf = resizeBlock(databuf, buflen, data->sizeCompressed*7);
        stream.next_out = databuf+buflen;
        stream.avail_out += data->sizeCompressed*7*sizeof(char);
        buflen += data->sizeCompressed*7*sizeof(char);
        returnVal = inflate(&stream, Z_NO_FLUSH); //decompress
    }
    //printf("decompressed... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
    data->sizeUncompressed -= stream.avail_out;
    inflateEnd(&stream);
    if(data->_uncompressedDataExists)
    {
        free(data->uncompressedData);
    }
    data->uncompressedData = databuf;
    data->_uncompressedDataExists = 1;
    if(data->_nbtDataExists)
    {
        data->_nbtDataOutdated = 1;
    }
}

int deflateFile(nbtFile_t* data)
{

    if(data->_uncompressedDataOutdated)
    {
        return 1;
    }

    int returnVal;
    //printf("Deflating chunk of size %lu, to an estimated %u\n", data->sizeUncompressed, data->sizeCompressed);
    //unsigned long outputLen;
    unsigned long buflen = data->sizeCompressed;
    unsigned char* databuf = malloc(data->sizeCompressed*sizeof(char));
    z_stream stream;
    unsigned int chunkSize = data->sizeCompressed;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = data->sizeUncompressed;
    stream.next_in = data->uncompressedData; //data to deflate
    returnVal = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY); //init deflate
    stream.next_out = databuf; //decompressed data
    stream.avail_out = buflen;
    data->sizeCompressed = buflen;
    //printf("decompressing... return val: %d, avail_out: %d, avail_in: %d\n", returnVal, stream.avail_out, stream.avail_in);
    returnVal = deflate(&stream, Z_FINISH); //compress
    while(returnVal != Z_STREAM_END)
    {
        //printf("decompressing... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
        databuf = resizeBlock(databuf, buflen, chunkSize);
        stream.next_out = databuf+buflen;
        stream.avail_out += chunkSize;
        buflen += chunkSize;
        data->sizeCompressed += chunkSize;
        returnVal = deflate(&stream, Z_FINISH); //compress
    }
    //printf("decompressed... return val: %d, avail_out: %u, avail_in: %u\n", returnVal, stream.avail_out, stream.avail_in);
    data->sizeCompressed -= stream.avail_out;
    deflateEnd(&stream);
    if(data->_compressedDataExists)
    {
        free(data->compressedData);
    }
    data->compressedData = databuf;
    data->_compressedDataExists = 1;
    data->_compressedDataOutdated = 0;
    return 0;
}