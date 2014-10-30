#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void pad(uint8_t * block, int blockSize, int fileSize)
{
    //Padding for a 1-block case
    int d = (447 - (fileSize*8)) % 512;
    //l is block size in bits
    uint64_t l = (uint64_t)fileSize * 8;
    printf("%d\n", l);
    block[blockSize] = 0x80;
    int i;
    for( i = 0; i < 8; i++ )
    {
        block[56+i] = (l >> (56-(8*i)));
    }
}

void doSha1(uint8_t * block)
{
    printf("Doing SHA1 on %s\n", block);
}

int main( int argc, char **argv )
{
    if( argc == 2 )
    {
        char * fileName = argv[1];
        //Read the input file 512 bits at a time = 64 bytes = 16 32-bit words
        FILE * file = fopen( fileName, "rb" );
        int fileSize = 0;
        if( file != NULL )
        {
            uint8_t *block = malloc(64);
            size_t readSize = fread( block, 1, 64, file );
            fileSize += readSize;
            while( readSize == 64 )
            {
                doSha1(block);
                readSize = fread( block, 1, 64, file );
                fileSize += readSize;
            }
            //We need padding
            int i;
            for(i = readSize; i < 64; i++)
            {
                block[i] = 0x00;
            }
            pad(block, readSize, fileSize);
        }
        else
        {
            printf("Input file must exist.\n");
        }
    }
    else
    {
        printf("Usage is sha1 [filename]\n");
    }
}


